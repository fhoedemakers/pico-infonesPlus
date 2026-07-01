/*===================================================================*/
/*                                                                   */
/*  InfoNES_pAPU_Vrc7.cpp : Konami VRC7 (Yamaha OPLL / YM2413)       */
/*                         FM expansion audio for Mapper 85          */
/*                         (Lagrange Point, Tiny Toon Adv. 2 JP)     */
/*                                                                   */
/*  Six 2-operator FM channels, 15 fixed instruments + 1 user patch. */
/*  No rhythm mode (hard-wired off in VRC7 silicon).                 */
/*                                                                   */
/*  Memory layout:                                                   */
/*    - LUTs are constexpr (flash)                                   */
/*    - per-op/per-channel state allocated from PSRAM in Map85_Init  */
/*    - render loop and immediate helpers in SRAM via               */
/*      __not_in_flash_func() for the 44.1 kHz hot path             */
/*===================================================================*/

#if PICO_RP2350

#include "InfoNES_Types.h"
#include "InfoNES_pAPU.h"
#include "FrensHelpers.h"
#include <array>
#include <cstdint>
#include <cstring>

#ifdef PICO_BUILD
  #include "pico/platform.h"
#else
  #define __not_in_flash_func(name) name
  #define __not_in_flash(name)
#endif

/*-------------------------------------------------------------------*/
/*  Public state                                                     */
/*-------------------------------------------------------------------*/
BYTE  ApuVrc7Enable = 0;
BYTE *vrc7_wave_buffer = nullptr;

/*-------------------------------------------------------------------*/
/*  constexpr math (C++17): polynomial / Taylor-series approximations */
/*  used to bake LUTs at build time, without pulling in libm.        */
/*-------------------------------------------------------------------*/
namespace cx {

constexpr double PI = 3.141592653589793238462643;
constexpr double LN2 = 0.6931471805599453094172;

constexpr double absd(double x) { return x < 0 ? -x : x; }

/* sin(x), x in radians. Range-reduce to [-π, π] then 18-term Taylor. */
constexpr double sin(double x) {
    while (x >  PI) x -= 2 * PI;
    while (x < -PI) x += 2 * PI;
    double s = x, t = x, x2 = x * x;
    for (int i = 1; i < 20; ++i) {
        t *= -x2 / ((2.0 * i) * (2.0 * i + 1.0));
        s += t;
    }
    return s;
}

/* ln(x) for x > 0, via ln(m) = 2 * atanh((m-1)/(m+1)) after range-reducing
 * x to [1, 2) and tracking the power-of-two factor.                     */
constexpr double ln(double x) {
    double k = 0.0;
    while (x >= 2.0) { x *= 0.5; k += 1.0; }
    while (x <  1.0) { x *= 2.0; k -= 1.0; }
    double y = (x - 1.0) / (x + 1.0);
    double y2 = y * y;
    double t = y, s = y;
    for (int i = 1; i < 40; ++i) {
        t *= y2;
        s += t / (2.0 * i + 1.0);
    }
    return k * LN2 + 2.0 * s;
}

constexpr double log2(double x) { return ln(x) / LN2; }

/* 2^x = e^(x ln 2). Split into integer + fractional. */
constexpr double exp2(double x) {
    int k = (int)x;
    if (x < 0 && (double)k != x) k -= 1;
    double f = x - (double)k;
    double a = f * LN2, t = 1.0, s = 1.0;
    for (int i = 1; i < 30; ++i) {
        t *= a / (double)i;
        s += t;
    }
    if (k >= 0) for (int i = 0; i <  k; ++i) s *= 2.0;
    else        for (int i = 0; i < -k; ++i) s *= 0.5;
    return s;
}

} // namespace cx

/*-------------------------------------------------------------------*/
/*  Build-time precalculated LUTs (in flash via static constexpr)   */
/*                                                                   */
/*  LOGSIN: -log2(sin(x)) for x in (0, π/2], at 1024 points.        */
/*          Q8.10 fixed-point: value range 0..2137 (i=0 ≈ peak).    */
/*          A full sine is reconstructed by mirroring the quadrant: */
/*          phase[17..16] selects which of the 4 quarters.          */
/*                                                                   */
/*  EXP:    pow(2, -i/256) * 1024, i in 0..255. Used to convert     */
/*          a log-domain amplitude back to a linear sample.          */
/*-------------------------------------------------------------------*/
namespace {

constexpr int LOGSIN_BITS = 10;            // 1024-entry quarter sine
constexpr int LOGSIN_SIZE = 1 << LOGSIN_BITS;

constexpr std::array<uint16_t, LOGSIN_SIZE> make_logsin() {
    std::array<uint16_t, LOGSIN_SIZE> a{};
    for (int i = 0; i < LOGSIN_SIZE; ++i) {
        double x = (double)(i + 0.5) / (double)LOGSIN_SIZE * (cx::PI * 0.5);
        double v = -cx::log2(cx::sin(x)) * 256.0;
        if (v < 0) v = 0;
        if (v > 65535.0) v = 65535.0;
        a[i] = (uint16_t)(v + 0.5);
    }
    return a;
}

constexpr std::array<uint16_t, 256> make_exp() {
    std::array<uint16_t, 256> a{};
    /* exp[i] = round((2^(i/256) - 1) * 1024), reversed at lookup time so
     * the highest entry corresponds to the smallest log magnitude.      */
    for (int i = 0; i < 256; ++i) {
        double v = (cx::exp2((double)i / 256.0) - 1.0) * 1024.0;
        a[i] = (uint16_t)(v + 0.5);
    }
    return a;
}

constexpr auto LOGSIN = make_logsin();
constexpr auto EXP    = make_exp();

/* Multiplier table (ML × 2). ML field is 4 bits.                    */
constexpr uint8_t MT[16] = { 1, 2, 4, 6, 8, 10, 12, 14,
                            16, 18, 20, 20, 24, 24, 30, 30 };

/* Key-scale level ROM (16-entry table indexed by F-num high 4 bits).
 * Values are in 6 dB units of attenuation applied per octave.       */
constexpr uint8_t KSL_TBL[16] = {
    0, 32, 40, 45, 48, 51, 53, 55,
    56, 58, 59, 60, 61, 62, 63, 64
};

/* VRC7 instrument ROM (15 patches × 8 bytes). Bytes 0-7 follow the
 * OPLL user-patch register layout exactly (regs $00..$07).
 *   byte 0:  AM, VIB, EG-type, KSR, MULT  (modulator)
 *   byte 1:  AM, VIB, EG-type, KSR, MULT  (carrier)
 *   byte 2:  KSL[7:6], TL[5:0]            (modulator total level)
 *   byte 3:  KSL[7:6], DC, DM, FB         (carrier KSL + waveform/fb)
 *   byte 4:  AR[7:4], DR[3:0]             (modulator)
 *   byte 5:  AR[7:4], DR[3:0]             (carrier)
 *   byte 6:  SL[7:4], RR[3:0]             (modulator)
 *   byte 7:  SL[7:4], RR[3:0]             (carrier)
 *
 * Values are the Konami VRC7 silicon dump (Norix), widely cross-
 * validated by FCEUX / Mednafen / emu2413.                          */
constexpr uint8_t VRC7_INST[15][8] = {
    { 0x03, 0x21, 0x05, 0x06, 0xE8, 0x81, 0x42, 0x27 }, /*  1 Buzzy Bell  */
    { 0x13, 0x41, 0x14, 0x0D, 0xD8, 0xF6, 0x23, 0x12 }, /*  2 Guitar      */
    { 0x11, 0x11, 0x08, 0x08, 0xFA, 0xB2, 0x20, 0x12 }, /*  3 Wurly       */
    { 0x31, 0x61, 0x0C, 0x07, 0xA8, 0x64, 0x61, 0x27 }, /*  4 Flute       */
    { 0x32, 0x21, 0x1E, 0x06, 0xE1, 0x76, 0x01, 0x28 }, /*  5 Clarinet    */
    { 0x02, 0x01, 0x06, 0x00, 0xA3, 0xE2, 0xF4, 0xF4 }, /*  6 Synth       */
    { 0x21, 0x61, 0x1D, 0x07, 0x82, 0x81, 0x11, 0x07 }, /*  7 Trumpet     */
    { 0x23, 0x21, 0x22, 0x17, 0xA2, 0x72, 0x01, 0x17 }, /*  8 Organ       */
    { 0x35, 0x11, 0x25, 0x00, 0x40, 0x73, 0x72, 0x01 }, /*  9 Bells       */
    { 0xB5, 0x01, 0x0F, 0x0F, 0xA8, 0xA5, 0x51, 0x02 }, /* 10 Vibes       */
    { 0x17, 0xC1, 0x24, 0x07, 0xF8, 0xF8, 0x22, 0x12 }, /* 11 Vibraphone  */
    { 0x71, 0x23, 0x11, 0x06, 0x65, 0x74, 0x18, 0x16 }, /* 12 Tutti       */
    { 0x01, 0x02, 0xD3, 0x05, 0xC9, 0x95, 0x03, 0x02 }, /* 13 Fretless    */
    { 0x61, 0x63, 0x0C, 0x00, 0x94, 0xC0, 0x33, 0xF6 }, /* 14 Synth Bass  */
    { 0x21, 0x72, 0x0D, 0x00, 0xC1, 0xD5, 0x56, 0x06 }, /* 15 Sweep       */
};

/* Envelope rate tables. The OPLL effective rate is (RATE * 4 + KSR);
 * each effective rate produces a per-sample envelope increment in
 * Q24 fixed-point — the advance loop adds this to op.eg_phase, and
 * any overflow above bit 24 becomes an integer step in eg_level
 * (0..127 range, 0 = loud, 127 = silent).
 *
 * Rate doubles every 4 effective-rate steps. Calibration: at rate
 * index 48 (R=12) the carrier should decay 0→127 in roughly 1.6 s at
 * 49.7 kHz native; we scale by VRC7_FREQ_SCALE for 44.1 kHz output.  */
constexpr double VRC7_FREQ_SCALE = 49716.0 / 44100.0;

constexpr std::array<uint32_t, 64> make_eg_rate(bool attack) {
    std::array<uint32_t, 64> a{};
    a[0] = 0;
    for (int i = 1; i < 64; ++i) {
        /* Per-sample increment in Q24. Doubled every 4 rate steps.
         * Base coefficient (4..7) per (i&3); each exp doubles via
         * left-shift on the integer exp counter.
         * Calibration: at i=48 (R=12) full 0→127 envelope should take
         * ~0.33s, matching OPLL silicon — that's ~385 eg_level/sec,
         * so Q24-per-sample ≈ 385 / 44100 × 2^24 ≈ 146 000.          */
        int frac = 4 + (i & 3);
        int exp_steps = i >> 2;
        uint64_t step = ((uint64_t)frac << exp_steps) << 6;
        if (attack) step <<= 2;                  /* attack ~4× faster */
        step = (uint64_t)((double)step * VRC7_FREQ_SCALE);
        if (step > 0xFFFFFFFFull) step = 0xFFFFFFFFull;
        a[i] = (uint32_t)step;
    }
    return a;
}

constexpr auto AR_RATE = make_eg_rate(true);
constexpr auto DR_RATE = make_eg_rate(false);

/* Phase-increment base table.
 *
 * OPLL output frequency: F = Fnum * (Fchip/2^19) * 2^(block-1)
 *   where Fchip/72 = 49716 Hz (NTSC native sample rate).
 *
 * Per-sample 18-bit phase increment at our 44.1 kHz output:
 *   inc = F * 2^18 / 44100
 *       = Fnum * VRC7_FREQ_SCALE * 2^(block-2) * actual_multiplier
 *
 * We store PHASE_BASE[fnum] = round(fnum * VRC7_FREQ_SCALE * 256)
 * (Q8 fixed-point), then the per-channel formula is
 *   inc = (PHASE_BASE[fnum] << block) * MT[ml] >> 11
 * where MT is 2× the actual multiplier — the trailing >> 11 absorbs
 * the Q8 scaling (8), the block-vs-2^(block-2) offset (2), and the
 * MT-vs-actual-ml ×2 (1).                                            */
constexpr uint32_t PHASE_BITS = 18;
constexpr uint32_t PHASE_MASK = (1u << PHASE_BITS) - 1u;

constexpr std::array<uint32_t, 512> make_phase_inc_base() {
    std::array<uint32_t, 512> a{};
    for (int fnum = 0; fnum < 512; ++fnum) {
        double inc = (double)fnum * VRC7_FREQ_SCALE * 256.0;
        a[fnum] = (uint32_t)(inc + 0.5);
    }
    return a;
}

constexpr auto PHASE_BASE = make_phase_inc_base();

} // anonymous namespace

/*-------------------------------------------------------------------*/
/*  Runtime state (PSRAM-allocated via Frens::f_malloc).            */
/*-------------------------------------------------------------------*/

namespace {

struct Operator {
    uint32_t phase;        // 18-bit phase accumulator
    uint32_t phase_inc;    // per-sample increment (scaled by mult/octave)
    int32_t  eg_level;     // current envelope, 0..127 (0=loud, 127=silent)
    uint32_t eg_phase;     // Q24 fractional accumulator for EG advancement
    uint8_t  eg_state;     // 0=damp, 1=attack, 2=decay, 3=sustain
    uint8_t  fb_lvl;       // feedback shift (only used on modulator op)
    int16_t  fb_buf[2];    // last 2 outputs for feedback averaging

    /* Patch fields (cached from current instrument). */
    uint8_t  mult;         // 4 bits → MT[]
    uint8_t  ksr;          // key-scale-rate enable
    uint8_t  egt;          // EG-type: 1=sustained, 0=percussive
    uint8_t  am;           // tremolo on/off
    uint8_t  vib;          // vibrato on/off
    uint8_t  ksl;          // key-scale-level (2 bits)
    uint8_t  tl;           // total level (6 bits, modulator) or vol (carrier)
    uint8_t  ar, dr, sl, rr;
    uint8_t  is_carrier;
};

struct Channel {
    uint16_t fnum;         // 9 bits
    uint8_t  block;        // 3 bits (octave)
    uint8_t  inst;         // 4 bits
    uint8_t  volume;       // 4 bits (only meaningful on carrier)
    uint8_t  key_on;
    uint8_t  sustain;
};

struct Vrc7State {
    Operator op[12];       // 2 ops per channel × 6 channels
    Channel  ch[6];
    uint8_t  user_patch[8];
    uint8_t  reg_latch;
    /* Shared LFOs (AM = tremolo, PM = vibrato). */
    uint32_t am_phase;
    uint32_t pm_phase;
};

Vrc7State *vrc7 = nullptr;

constexpr uint32_t AM_PM_PER_SAMPLE_AM = (uint32_t)((double)(1u << 20) * 3.7 / 44100.0); // ~3.7 Hz
constexpr uint32_t AM_PM_PER_SAMPLE_PM = (uint32_t)((double)(1u << 20) * 6.4 / 44100.0); // ~6.4 Hz

} // anonymous namespace

/*-------------------------------------------------------------------*/
/*  Patch application (cold path — runs only on register writes)    */
/*-------------------------------------------------------------------*/

static const uint8_t *vrc7_patch_bytes(uint8_t inst)
{
    if (inst == 0) return vrc7->user_patch;
    return VRC7_INST[inst - 1];
}

static void vrc7_apply_patch(int ch_idx)
{
    Channel &c = vrc7->ch[ch_idx];
    Operator &m = vrc7->op[ch_idx * 2];
    Operator &k = vrc7->op[ch_idx * 2 + 1];
    const uint8_t *p = vrc7_patch_bytes(c.inst);

    /* Modulator (op0) */
    m.am   = (p[0] >> 7) & 1;
    m.vib  = (p[0] >> 6) & 1;
    m.egt  = (p[0] >> 5) & 1;
    m.ksr  = (p[0] >> 4) & 1;
    m.mult =  p[0]       & 0x0F;
    m.ksl  = (p[2] >> 6) & 3;
    m.tl   =  p[2]       & 0x3F;
    m.fb_lvl = p[3] & 0x07;
    m.ar = (p[4] >> 4) & 0x0F;
    m.dr =  p[4]       & 0x0F;
    m.sl = (p[6] >> 4) & 0x0F;
    m.rr =  p[6]       & 0x0F;
    m.is_carrier = 0;

    /* Carrier (op1) */
    k.am   = (p[1] >> 7) & 1;
    k.vib  = (p[1] >> 6) & 1;
    k.egt  = (p[1] >> 5) & 1;
    k.ksr  = (p[1] >> 4) & 1;
    k.mult =  p[1]       & 0x0F;
    k.ksl  = (p[3] >> 6) & 3;
    k.tl   = c.volume;              /* carrier TL = per-channel volume */
    k.fb_lvl = 0;
    k.ar = (p[5] >> 4) & 0x0F;
    k.dr =  p[5]       & 0x0F;
    k.sl = (p[7] >> 4) & 0x0F;
    k.rr =  p[7]       & 0x0F;
    k.is_carrier = 1;
}

static void vrc7_update_freq(int ch_idx)
{
    Channel &c = vrc7->ch[ch_idx];
    Operator &m = vrc7->op[ch_idx * 2];
    Operator &k = vrc7->op[ch_idx * 2 + 1];
    /* inc = (PHASE_BASE[fnum] << block) * MT[ml] >> 11  (see table comment).
     * 64-bit intermediate because block=7 + MT=30 overflows 32-bit.   */
    uint64_t inc = (uint64_t)PHASE_BASE[c.fnum] << c.block;
    m.phase_inc = (uint32_t)((inc * MT[m.mult]) >> 11);
    k.phase_inc = (uint32_t)((inc * MT[k.mult]) >> 11);
}

static uint8_t vrc7_effective_rate(const Operator &op, const Channel &c, uint8_t rate)
{
    if (rate == 0) return 0;
    uint8_t ksr_val = (c.block << 1) | (c.fnum >> 8);
    uint8_t shift   = op.ksr ? 0 : 2;
    uint8_t er      = (rate << 2) + (ksr_val >> shift);
    if (er > 63) er = 63;
    return er;
}

static void vrc7_trigger_key_on(int ch_idx)
{
    Operator &m = vrc7->op[ch_idx * 2];
    Operator &k = vrc7->op[ch_idx * 2 + 1];
    /* OPLL key-on starts a DAMP phase that silences the carrier before
     * re-attacking. DO NOT reset the oscillator phase — that creates
     * an audible click on every note-on. Real OPLL preserves phase
     * across key transitions; only the envelope is reset.            */
    m.eg_state = 0;
    k.eg_state = 0;
    m.eg_phase = 0;
    k.eg_phase = 0;
    m.fb_buf[0] = m.fb_buf[1] = 0;
}

static void vrc7_trigger_key_off(int ch_idx)
{
    Operator &m = vrc7->op[ch_idx * 2];
    Operator &k = vrc7->op[ch_idx * 2 + 1];
    /* Both ops drop straight to the release state from wherever they
     * are. eg_level keeps its current value.                          */
    if (m.eg_state < 3) m.eg_state = 3;     /* fall through to release path */
    if (k.eg_state < 3) k.eg_state = 3;
    /* We model RR via the rate table indexed by op.rr when in state 3. */
}

/*-------------------------------------------------------------------*/
/*  Public write handlers — cold path                                */
/*-------------------------------------------------------------------*/

void ApuWriteVrc7Reg(BYTE value)
{
    if (!vrc7) return;
    vrc7->reg_latch = value;
}

void ApuWriteVrc7Data(BYTE value)
{
    if (!vrc7) return;
    BYTE reg = vrc7->reg_latch;

    if (reg < 8) {
        vrc7->user_patch[reg] = value;
        for (int i = 0; i < 6; ++i)
            if (vrc7->ch[i].inst == 0) {
                vrc7_apply_patch(i);
                vrc7_update_freq(i);
            }
        return;
    }
    if (reg >= 0x10 && reg <= 0x15) {
        int c = reg - 0x10;
        vrc7->ch[c].fnum = (vrc7->ch[c].fnum & 0x100) | value;
        vrc7_update_freq(c);
        return;
    }
    if (reg >= 0x20 && reg <= 0x25) {
        int c = reg - 0x20;
        vrc7->ch[c].fnum  = (vrc7->ch[c].fnum & 0xFF) | ((value & 0x01) << 8);
        vrc7->ch[c].block = (value >> 1) & 0x07;
        uint8_t new_key = (value >> 4) & 1;
        uint8_t new_sus = (value >> 5) & 1;
        if (new_key && !vrc7->ch[c].key_on) vrc7_trigger_key_on(c);
        else if (!new_key && vrc7->ch[c].key_on) vrc7_trigger_key_off(c);
        vrc7->ch[c].key_on  = new_key;
        vrc7->ch[c].sustain = new_sus;
        vrc7_update_freq(c);
        return;
    }
    if (reg >= 0x30 && reg <= 0x35) {
        int c = reg - 0x30;
        vrc7->ch[c].inst   = (value >> 4) & 0x0F;
        vrc7->ch[c].volume =  value       & 0x0F;
        vrc7_apply_patch(c);
        vrc7_update_freq(c);
        return;
    }
}

/*-------------------------------------------------------------------*/
/*  Per-sample operator helpers (HOT PATH — SRAM)                   */
/*-------------------------------------------------------------------*/

static inline int32_t __not_in_flash_func(vrc7_op_compute)(
        const Operator &op, uint32_t phase_in, uint8_t am_lvl)
{
    /* phase_in: 18-bit phase already pre-shifted with any modulation. */
    uint32_t phase = phase_in & PHASE_MASK;
    uint32_t quad  = (phase >> (PHASE_BITS - 2)) & 0x3;
    uint32_t idx   = (phase >> (PHASE_BITS - 2 - LOGSIN_BITS)) & (LOGSIN_SIZE - 1);
    if (quad & 1) idx ^= (LOGSIN_SIZE - 1);

    /* Base log-amplitude. */
    uint32_t log_val = LOGSIN[idx];

    /* Envelope contribution. eg_level is 0..127; each step = 0.75 dB so
     * the full sweep covers ~95 dB — that's what makes a released note
     * actually reach silence. In Q8 log domain (1 unit = 1/256 of log2)
     * 0.75 dB ≈ 32 → eg_level << 5.                                   */
    log_val += (uint32_t)op.eg_level << 5;

    /* TL: 6-bit total level / volume, same 0.75 dB per step. */
    log_val += (uint32_t)op.tl << 5;

    /* AM (tremolo): ~1 dB peak modulation when enabled. */
    if (op.am) log_val += (uint32_t)am_lvl << 3;

    /* Convert log → linear via EXP LUT. log_val format: top bits =
     * shift amount, low 8 bits = fractional within EXP[]. */
    uint32_t frac  = log_val & 0xFF;
    uint32_t shift = log_val >> 8;
    if (shift > 30) return 0;
    int32_t lin = (int32_t)(EXP[frac ^ 0xFF] + 1024) >> shift;

    /* Negative half of the sine wave. */
    return (quad & 2) ? -lin : lin;
}

static inline void __not_in_flash_func(vrc7_advance_eg)(Operator &op, const Channel &c)
{
    /* All states feed their per-sample increment through the Q24
     * accumulator. Any overflow becomes integer steps in eg_level.   */
    switch (op.eg_state) {
        case 0: /* DAMP: fast linear ramp toward silence before attacking. */
            op.eg_level += 2;
            if (op.eg_level >= 0x7F) {
                op.eg_level = 0x7F;
                op.eg_state = 1;
                op.eg_phase = 0;
            }
            break;

        case 1: /* ATTACK: exponential approach toward 0 (loud).         */
        {
            uint32_t step = AR_RATE[vrc7_effective_rate(op, c, op.ar)];
            op.eg_phase += step;
            uint32_t adv = op.eg_phase >> 24;
            op.eg_phase &= 0x00FFFFFF;
            if (adv) {
                /* Logarithmic-feel ramp: each tick removes ~1/8 of the
                 * remaining distance to 0, plus a constant floor so we
                 * always reach zero in finite time even at slow rates. */
                int32_t delta = (int32_t)((((uint64_t)(op.eg_level + 1) * adv) >> 3) + 1);
                op.eg_level -= delta;
                if (op.eg_level <= 0) {
                    op.eg_level = 0;
                    op.eg_state = 2;
                    op.eg_phase = 0;
                }
            }
            break;
        }

        case 2: /* DECAY: linear toward sustain level (op.sl). */
        {
            uint32_t step = DR_RATE[vrc7_effective_rate(op, c, op.dr)];
            op.eg_phase += step;
            uint32_t adv = op.eg_phase >> 24;
            op.eg_phase &= 0x00FFFFFF;
            op.eg_level += (int32_t)adv;
            /* OPLL SL spec: each step = 3 dB → 4 eg_level units (one
             * eg_level unit = 0.75 dB). SL=15 is special-cased to "decay
             * all the way to silence" → 124 ≈ -93 dB.                  */
            int32_t sl_target = (op.sl < 15) ? ((int32_t)op.sl << 2) : 124;
            if (op.eg_level >= sl_target) {
                op.eg_level = sl_target;
                op.eg_state = 3;
                op.eg_phase = 0;
            }
            if (op.eg_level > 0x7F) op.eg_level = 0x7F;
            break;
        }

        case 3: /* SUSTAIN (key held) or RELEASE (key off).
                 *
                 * EG-type bit:
                 *   1 = sustained tone — hold at SL while key held,
                 *                        release at op.rr when key off
                 *   0 = percussive   — keep decaying at op.rr while held,
                 *                      release at op.rr when key off
                 * Channel sustain bit (R20 bit5): overrides the release
                 *   rate to a fixed slow rate (5) when key is off, for
                 *   pads / long-tail patches.                          */
        {
            uint8_t rate;
            if (c.key_on) {
                rate = op.egt ? 0 : op.rr;
            } else {
                rate = c.sustain ? 5 : op.rr;
            }
            if (rate) {
                uint32_t step = DR_RATE[vrc7_effective_rate(op, c, rate)];
                op.eg_phase += step;
                uint32_t adv = op.eg_phase >> 24;
                op.eg_phase &= 0x00FFFFFF;
                op.eg_level += (int32_t)adv;
                if (op.eg_level > 0x7F) op.eg_level = 0x7F;
            }
            break;
        }
    }
}

/*-------------------------------------------------------------------*/
/*  HOT PATH: render n samples into vrc7_wave_buffer (0..255 BYTE)  */
/*-------------------------------------------------------------------*/

void __not_in_flash_func(ApuRenderingVrc7)(int n)
{
    if (!ApuVrc7Enable || !vrc7 || !vrc7_wave_buffer) {
        if (vrc7_wave_buffer) memset(vrc7_wave_buffer, 0x80, n);
        return;
    }

    for (int i = 0; i < n; ++i) {
        /* Advance shared LFOs. AM produces a 0..15 triangle; PM is sub-
         * sample frequency wobble — implemented via small phase
         * pre-shifts when vib=1.                                      */
        vrc7->am_phase += AM_PM_PER_SAMPLE_AM;
        vrc7->pm_phase += AM_PM_PER_SAMPLE_PM;
        uint8_t am_lvl = (vrc7->am_phase >> 16) & 0xFF;
        if (am_lvl & 0x80) am_lvl ^= 0xFF;       /* triangle 0..127 */
        am_lvl >>= 3;                            /* 0..15 dB-ish    */
        int32_t pm_offset = (int32_t)(int16_t)(vrc7->pm_phase >> 4);
        pm_offset >>= 10;                        /* small phase wobble */

        int32_t mix = 0;
        for (int c = 0; c < 6; ++c) {
            Operator &m = vrc7->op[c * 2];
            Operator &k = vrc7->op[c * 2 + 1];
            Channel  &ch = vrc7->ch[c];

            vrc7_advance_eg(m, ch);
            vrc7_advance_eg(k, ch);

            uint32_t pm_m = m.vib ? (uint32_t)pm_offset : 0;
            uint32_t pm_k = k.vib ? (uint32_t)pm_offset : 0;

            m.phase += m.phase_inc;
            k.phase += k.phase_inc;

            /* Modulator with feedback: feedback amount is the average
             * of the last two outputs, shifted by (8 - FB). FB=0 means
             * no feedback (special case). FB=7 is max — about 1 full
             * carrier cycle of self-PM at peak amplitude.              */
            int32_t fb = 0;
            if (m.fb_lvl) fb = ((int32_t)m.fb_buf[0] + (int32_t)m.fb_buf[1]) >> (8 - m.fb_lvl);
            int32_t m_out = vrc7_op_compute(m, m.phase + pm_m + (uint32_t)(fb << 8), am_lvl);
            m.fb_buf[1] = m.fb_buf[0];
            m.fb_buf[0] = (int16_t)m_out;

            /* Carrier modulated by modulator output. Scaling controls
             * the FM modulation index — << 8 puts the peak modulation
             * around one full carrier cycle for a max-amplitude
             * modulator, which is the typical OPLL FM index.          */
            int32_t k_out = vrc7_op_compute(k, k.phase + pm_k + (uint32_t)(m_out << 8), am_lvl);

            /* Only emit carrier output if the carrier envelope hasn't
             * fully released — saves cycles vs. emitting silence.     */
            if (k.eg_level < 0x7F) mix += k_out;
        }

        /* mix peak per channel ≈ ±2048 at full volume; up to 6 channels
         * can stack. Shift right by 7 (÷128) gives ±96 worst-case around
         * the 128 BYTE midpoint, leaving headroom and avoiding the hard
         * clipping that produced subharmonics earlier.                 */
        int sample = (mix >> 7) + 128;
        if (sample < 0) sample = 0;
        if (sample > 255) sample = 255;
        vrc7_wave_buffer[i] = (BYTE)sample;
    }
}

/*-------------------------------------------------------------------*/
/*  Init / Reset                                                      */
/*-------------------------------------------------------------------*/

void ApuVrc7Reset(void)
{
    if (!vrc7) {
        vrc7 = (Vrc7State *)Frens::f_malloc(sizeof(Vrc7State));
    }
    if (!vrc7) return;
    memset(vrc7, 0, sizeof(*vrc7));
    /* Start every channel on instrument 0 (user patch) — value 0 is the
     * default register state and zeroed user_patch is "silent piano".  */
    for (int c = 0; c < 6; ++c) {
        vrc7->ch[c].inst = 0;
        vrc7->ch[c].volume = 0x0F;     /* full attenuation = silent */
        vrc7_apply_patch(c);
        /* Park envelopes at silent until a key-on. */
        vrc7->op[c*2].eg_level     = 0x7F;
        vrc7->op[c*2].eg_state     = 3;
        vrc7->op[c*2+1].eg_level   = 0x7F;
        vrc7->op[c*2+1].eg_state   = 3;
    }
    if (vrc7_wave_buffer) memset(vrc7_wave_buffer, 0x80, APU_MAX_SAMPLES_PER_SYNC);
}

void ApuVrc7Free(void)
{
    if (vrc7) {
        Frens::f_free(vrc7);
        vrc7 = nullptr;
    }
}

#endif /* PICO_RP2350 */
