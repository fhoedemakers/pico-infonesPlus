/*===================================================================*/
/*                                                                   */
/*  Mapper 5 (MMC5) - New Implementation                            */
/*                                                                   */
/*  Supports: PRG banking (4 modes), CHR banking (4 modes) with     */
/*  sprite/BG set distinction, nametable mapping, fill mode,        */
/*  extended RAM, scanline IRQ, hardware multiplier, WRAM banking,  */
/*  and two MMC5 pulse audio channels.                              */
/*                                                                   */
/*===================================================================*/

/* External APU variables needed for audio rendering */
extern DWORD ApuPulseMagic;
extern BYTE ApuAtl[];
extern BYTE wave_buffers_extra[];

/*-------------------------------------------------------------------*/
/*  MMC5 Expansion WRAM: 64KB (8 x 8KB banks) - dynamically alloc'd */
/*-------------------------------------------------------------------*/
static BYTE *Map5_Wram = NULL;
#define MAP5_WRAM_SIZE (0x2000 * 8)

/*-------------------------------------------------------------------*/
/*  MMC5 Extended RAM: 1KB - dynamically allocated                   */
/*-------------------------------------------------------------------*/
static BYTE *Map5_ExRam = NULL;
#define MAP5_EXRAM_SIZE 0x400

/*-------------------------------------------------------------------*/
/*  MMC5 Fill-mode nametable: 1KB - dynamically allocated            */
/*-------------------------------------------------------------------*/
static BYTE *Map5_FillNt = NULL;
#define MAP5_FILLNT_SIZE 0x400

/*-------------------------------------------------------------------*/
/*  WRAM page accessor                                               */
/*-------------------------------------------------------------------*/
#define Map5_WRAMPAGE(a) &Map5_Wram[((a) & 0x07) * 0x2000]

/*-------------------------------------------------------------------*/
/*  PRG Banking                                                      */
/*-------------------------------------------------------------------*/
static BYTE Map5_PrgMode;        /* $5100: PRG mode (0-3) */
static BYTE Map5_PrgBank[5];     /* $5113-$5117 bank registers */

/*-------------------------------------------------------------------*/
/*  CHR Banking                                                      */
/*-------------------------------------------------------------------*/
static BYTE Map5_ChrMode;        /* $5101: CHR mode (0-3) */
static WORD Map5_ChrBankA[8];    /* $5120-$5127: Set A (sprites) */
static WORD Map5_ChrBankB[4];    /* $5128-$512B: Set B (backgrounds) */
static BYTE Map5_ChrHigh;        /* $5130: upper CHR bank bits */
static BYTE Map5_LastChrA;       /* 1 = last CHR write was to Set A */

/*-------------------------------------------------------------------*/
/*  RAM Protection                                                   */
/*-------------------------------------------------------------------*/
static BYTE Map5_RamProt0;       /* $5102 */
static BYTE Map5_RamProt1;       /* $5103 */

/*-------------------------------------------------------------------*/
/*  Extended RAM Mode                                                */
/*-------------------------------------------------------------------*/
static BYTE Map5_ExRamMode;      /* $5104: 0-3 */

/*-------------------------------------------------------------------*/
/*  Nametable Mapping                                                */
/*-------------------------------------------------------------------*/
static BYTE Map5_NtMapping;      /* $5105 */

/*-------------------------------------------------------------------*/
/*  Fill Mode                                                        */
/*-------------------------------------------------------------------*/
static BYTE Map5_FillTile;       /* $5106 */
static BYTE Map5_FillColor;      /* $5107 */

/*-------------------------------------------------------------------*/
/*  Scanline IRQ                                                     */
/*-------------------------------------------------------------------*/
static BYTE Map5_IrqTarget;      /* $5203: scanline compare value */
static BYTE Map5_IrqEnable;      /* $5204 write: bit 7 */
static BYTE Map5_IrqPending;     /* Pending IRQ flag */
static BYTE Map5_InFrame;        /* In-frame detection flag */
static BYTE Map5_IrqCounter;     /* Internal scanline counter */

/*-------------------------------------------------------------------*/
/*  Hardware Multiplier                                              */
/*-------------------------------------------------------------------*/
static BYTE Map5_MultA;          /* $5205: multiplicand */
static BYTE Map5_MultB;          /* $5206: multiplier */

/*-------------------------------------------------------------------*/
/*  MMC5 Audio: Two Pulse Channels                                   */
/*-------------------------------------------------------------------*/

/* Pulse channel registers */
static BYTE Map5_PulseCtrl[2];   /* $5000/$5004: DDLCVVVV */
static BYTE Map5_PulseTimerLo[2]; /* $5002/$5006: timer low */
static BYTE Map5_PulseTimerHi[2]; /* $5003/$5007: LLLLLHHH */
static BYTE Map5_PulseEnable[2]; /* From $5015: channel enable */

/* Derived pulse state */
static BYTE Map5_PulseDuty[2];   /* Duty cycle (0-3) */
static BYTE Map5_PulseHalt[2];   /* Length counter halt / envelope loop */
static BYTE Map5_PulseConstVol[2]; /* Constant volume flag */
static BYTE Map5_PulseVolume[2]; /* Volume / envelope period */
static WORD Map5_PulseFreq[2];   /* 11-bit timer period */
static BYTE Map5_PulseLen[2];    /* Length counter */

/* Envelope state */
static BYTE Map5_PulseEnvVol[2]; /* Current envelope output volume */
static BYTE Map5_PulseEnvDiv[2]; /* Envelope divider counter */
static BYTE Map5_PulseEnvStart[2]; /* Envelope start flag */

/* Rendering state */
static DWORD Map5_PulseSkip[2];  /* Phase increment (fixed-point) */
static DWORD Map5_PulseIndex[2]; /* Phase accumulator */

/* Audio status register */
static BYTE Map5_AudioStatus;    /* $5015 */

/* Frame counter for envelope/length clocking (240Hz) */
static BYTE Map5_AudioFrameCounter;
static BYTE Map5_AudioFrameDiv;

/*-------------------------------------------------------------------*/
/*  Pulse duty cycle waveforms (32 steps, matching APU format)       */
/*  Values: 0x11 (17) when ON, 0x00 when OFF                        */
/*  max output = 0x11 * 15 = 255 (fits in BYTE)                     */
/*-------------------------------------------------------------------*/
static const BYTE Map5_Duty87[0x20] = {
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x11,0x11,0x11,0x11,0x00,0x00,0x00,0x00
};
static const BYTE Map5_Duty75[0x20] = {
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
static const BYTE Map5_Duty50[0x20] = {
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
static const BYTE Map5_Duty25[0x20] = {
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const BYTE *Map5_DutyWaves[4] = {
    Map5_Duty87, Map5_Duty75, Map5_Duty50, Map5_Duty25
};

/*-------------------------------------------------------------------*/
/*  Helper: Update fill-mode nametable from tile and color           */
/*-------------------------------------------------------------------*/
static void Map5_UpdateFillNt(void)
{
  /* Tile area: bytes 0-959 */
  InfoNES_MemorySet(Map5_FillNt, Map5_FillTile, 0x3C0);
  /* Attribute area: bytes 960-1023 */
  BYTE attr = Map5_FillColor & 0x03;
  attr = attr | (attr << 2) | (attr << 4) | (attr << 6);
  InfoNES_MemorySet(&Map5_FillNt[0x3C0], attr, 0x400 - 0x3C0);
}

/*-------------------------------------------------------------------*/
/*  Helper: Apply nametable mapping from $5105                       */
/*-------------------------------------------------------------------*/
static void Map5_ApplyNtMapping(void)
{
  for (int i = 0; i < 4; i++)
  {
    BYTE src = (Map5_NtMapping >> (i * 2)) & 0x03;
    switch (src)
    {
    case 0:
      PPUBANK[i + 8] = VRAMPAGE(0);
      break;
    case 1:
      PPUBANK[i + 8] = VRAMPAGE(1);
      break;
    case 2:
      PPUBANK[i + 8] = Map5_ExRam;
      break;
    case 3:
      PPUBANK[i + 8] = Map5_FillNt;
      break;
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Sync PRG Banks                                          */
/*-------------------------------------------------------------------*/
void Map5_Sync_Prg_Banks(void)
{
  int nRomPages = NesHeader.byRomSize << 1; /* Total 8KB ROM pages */

  switch (Map5_PrgMode)
  {
  case 0: /* One 32KB bank */
  {
    int base = (Map5_PrgBank[4] & 0x7C);
    ROMBANK0 = ROMPAGE((base + 0) % nRomPages);
    ROMBANK1 = ROMPAGE((base + 1) % nRomPages);
    ROMBANK2 = ROMPAGE((base + 2) % nRomPages);
    ROMBANK3 = ROMPAGE((base + 3) % nRomPages);
    break;
  }
  case 1: /* Two 16KB banks */
  {
    /* $8000-$BFFF from $5115 */
    if (Map5_PrgBank[2] & 0x80)
    {
      int base = (Map5_PrgBank[2] & 0x7E);
      ROMBANK0 = ROMPAGE((base + 0) % nRomPages);
      ROMBANK1 = ROMPAGE((base + 1) % nRomPages);
    }
    else
    {
      int ramPage = (Map5_PrgBank[2] & 0x06);
      ROMBANK0 = Map5_WRAMPAGE(ramPage);
      ROMBANK1 = Map5_WRAMPAGE(ramPage + 1);
    }
    /* $C000-$FFFF from $5117 (always ROM) */
    {
      int base = (Map5_PrgBank[4] & 0x7E);
      ROMBANK2 = ROMPAGE((base + 0) % nRomPages);
      ROMBANK3 = ROMPAGE((base + 1) % nRomPages);
    }
    break;
  }
  case 2: /* 16KB + 8KB + 8KB */
  {
    /* $8000-$BFFF from $5115 */
    if (Map5_PrgBank[2] & 0x80)
    {
      int base = (Map5_PrgBank[2] & 0x7E);
      ROMBANK0 = ROMPAGE((base + 0) % nRomPages);
      ROMBANK1 = ROMPAGE((base + 1) % nRomPages);
    }
    else
    {
      int ramPage = (Map5_PrgBank[2] & 0x06);
      ROMBANK0 = Map5_WRAMPAGE(ramPage);
      ROMBANK1 = Map5_WRAMPAGE(ramPage + 1);
    }
    /* $C000-$DFFF from $5116 */
    if (Map5_PrgBank[3] & 0x80)
    {
      ROMBANK2 = ROMPAGE((Map5_PrgBank[3] & 0x7F) % nRomPages);
    }
    else
    {
      ROMBANK2 = Map5_WRAMPAGE(Map5_PrgBank[3] & 0x07);
    }
    /* $E000-$FFFF from $5117 (always ROM) */
    ROMBANK3 = ROMPAGE((Map5_PrgBank[4] & 0x7F) % nRomPages);
    break;
  }
  default: /* Mode 3: Four 8KB banks */
  {
    /* $8000-$9FFF from $5114 */
    if (Map5_PrgBank[1] & 0x80)
    {
      ROMBANK0 = ROMPAGE((Map5_PrgBank[1] & 0x7F) % nRomPages);
    }
    else
    {
      ROMBANK0 = Map5_WRAMPAGE(Map5_PrgBank[1] & 0x07);
    }
    /* $A000-$BFFF from $5115 */
    if (Map5_PrgBank[2] & 0x80)
    {
      ROMBANK1 = ROMPAGE((Map5_PrgBank[2] & 0x7F) % nRomPages);
    }
    else
    {
      ROMBANK1 = Map5_WRAMPAGE(Map5_PrgBank[2] & 0x07);
    }
    /* $C000-$DFFF from $5116 */
    if (Map5_PrgBank[3] & 0x80)
    {
      ROMBANK2 = ROMPAGE((Map5_PrgBank[3] & 0x7F) % nRomPages);
    }
    else
    {
      ROMBANK2 = Map5_WRAMPAGE(Map5_PrgBank[3] & 0x07);
    }
    /* $E000-$FFFF from $5117 (always ROM) */
    ROMBANK3 = ROMPAGE((Map5_PrgBank[4] & 0x7F) % nRomPages);
    break;
  }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Sync CHR Banks                                          */
/*  byMode: 1 = background, 0 = sprites                             */
/*-------------------------------------------------------------------*/
void Map5_Sync_Chr_Banks(BYTE byMode)
{
  DWORD nVRomPages = (DWORD)NesHeader.byVRomSize << 3; /* Total 1KB CHR pages */
  if (nVRomPages == 0) return;

  /*
   * When 8x16 sprites are in use (PPU_R0 bit 5):
   *   Sprite fetches ($5120-$5127 / Set A)
   *   BG fetches ($5128-$512B / Set B)
   * When 8x8 sprites:
   *   Both use Set A if last write was to A, else Set B
   */
  BYTE use_set_b;
  if (PPU_R0 & 0x20) /* 8x16 sprite mode */
    use_set_b = byMode; /* 1=BG uses Set B, 0=sprites use Set A */
  else
    use_set_b = !Map5_LastChrA; /* Use whichever set was last written */

  if (use_set_b)
  {
    /* Background: use Set B ($5128-$512B) mapped to all 8 pattern banks */
    DWORD page[4];
    switch (Map5_ChrMode)
    {
    case 0: /* 8K mode */
      page[3] = ((DWORD)Map5_ChrBankB[3] << 3) % nVRomPages;
      for (int i = 0; i < 8; i++)
        PPUBANK[i] = VROMPAGE((page[3] + i) % nVRomPages);
      break;
    case 1: /* 4K mode */
      page[3] = ((DWORD)Map5_ChrBankB[3] << 2) % nVRomPages;
      for (int i = 0; i < 4; i++)
        PPUBANK[i] = VROMPAGE((page[3] + i) % nVRomPages);
      for (int i = 0; i < 4; i++)
        PPUBANK[i + 4] = VROMPAGE((page[3] + i) % nVRomPages);
      break;
    case 2: /* 2K mode */
      page[1] = ((DWORD)Map5_ChrBankB[1] << 1) % nVRomPages;
      page[3] = ((DWORD)Map5_ChrBankB[3] << 1) % nVRomPages;
      PPUBANK[0] = VROMPAGE((page[1] + 0) % nVRomPages);
      PPUBANK[1] = VROMPAGE((page[1] + 1) % nVRomPages);
      PPUBANK[2] = VROMPAGE((page[3] + 0) % nVRomPages);
      PPUBANK[3] = VROMPAGE((page[3] + 1) % nVRomPages);
      PPUBANK[4] = VROMPAGE((page[1] + 0) % nVRomPages);
      PPUBANK[5] = VROMPAGE((page[1] + 1) % nVRomPages);
      PPUBANK[6] = VROMPAGE((page[3] + 0) % nVRomPages);
      PPUBANK[7] = VROMPAGE((page[3] + 1) % nVRomPages);
      break;
    default: /* 1K mode */
      for (int i = 0; i < 4; i++)
      {
        DWORD p = (DWORD)Map5_ChrBankB[i] % nVRomPages;
        PPUBANK[i] = VROMPAGE(p);
        PPUBANK[i + 4] = VROMPAGE(p);
      }
      break;
    }
  }
  else
  {
    /* Sprites: use Set A ($5120-$5127) */
    DWORD page[8];
    switch (Map5_ChrMode)
    {
    case 0: /* 8K mode: register 7 selects one 8K page */
      page[7] = ((DWORD)Map5_ChrBankA[7] << 3) % nVRomPages;
      for (int i = 0; i < 8; i++)
        PPUBANK[i] = VROMPAGE((page[7] + i) % nVRomPages);
      break;
    case 1: /* 4K mode: registers 3 and 7 select two 4K pages */
      page[3] = ((DWORD)Map5_ChrBankA[3] << 2) % nVRomPages;
      page[7] = ((DWORD)Map5_ChrBankA[7] << 2) % nVRomPages;
      for (int i = 0; i < 4; i++)
        PPUBANK[i] = VROMPAGE((page[3] + i) % nVRomPages);
      for (int i = 0; i < 4; i++)
        PPUBANK[i + 4] = VROMPAGE((page[7] + i) % nVRomPages);
      break;
    case 2: /* 2K mode: registers 1,3,5,7 select four 2K pages */
      page[1] = ((DWORD)Map5_ChrBankA[1] << 1) % nVRomPages;
      page[3] = ((DWORD)Map5_ChrBankA[3] << 1) % nVRomPages;
      page[5] = ((DWORD)Map5_ChrBankA[5] << 1) % nVRomPages;
      page[7] = ((DWORD)Map5_ChrBankA[7] << 1) % nVRomPages;
      PPUBANK[0] = VROMPAGE((page[1] + 0) % nVRomPages);
      PPUBANK[1] = VROMPAGE((page[1] + 1) % nVRomPages);
      PPUBANK[2] = VROMPAGE((page[3] + 0) % nVRomPages);
      PPUBANK[3] = VROMPAGE((page[3] + 1) % nVRomPages);
      PPUBANK[4] = VROMPAGE((page[5] + 0) % nVRomPages);
      PPUBANK[5] = VROMPAGE((page[5] + 1) % nVRomPages);
      PPUBANK[6] = VROMPAGE((page[7] + 0) % nVRomPages);
      PPUBANK[7] = VROMPAGE((page[7] + 1) % nVRomPages);
      break;
    default: /* 1K mode: all 8 registers */
      for (int i = 0; i < 8; i++)
      {
        page[i] = (DWORD)Map5_ChrBankA[i] % nVRomPages;
        PPUBANK[i] = VROMPAGE(page[i]);
      }
      break;
    }
  }
  InfoNES_SetupChr();
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Audio: Update pulse channel frequency skip              */
/*-------------------------------------------------------------------*/
static void Map5_UpdatePulseFreq(int ch)
{
  Map5_PulseFreq[ch] = Map5_PulseTimerLo[ch] |
                        ((Map5_PulseTimerHi[ch] & 0x07) << 8);
  /* Note: Unlike the APU, MMC5 does NOT silence channels with freq < 8 */
  if (Map5_PulseFreq[ch] >= 2)
    Map5_PulseSkip[ch] = ApuPulseMagic / (Map5_PulseFreq[ch] / 2);
  else if (Map5_PulseFreq[ch] == 1)
    Map5_PulseSkip[ch] = ApuPulseMagic;
  else
    Map5_PulseSkip[ch] = 0;
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Audio: Clock envelope for one pulse channel             */
/*-------------------------------------------------------------------*/
static void Map5_ClockEnvelope(int ch)
{
  if (Map5_PulseEnvStart[ch])
  {
    Map5_PulseEnvStart[ch] = 0;
    Map5_PulseEnvVol[ch] = 15;
    Map5_PulseEnvDiv[ch] = Map5_PulseVolume[ch];
  }
  else
  {
    if (Map5_PulseEnvDiv[ch] > 0)
    {
      Map5_PulseEnvDiv[ch]--;
    }
    else
    {
      Map5_PulseEnvDiv[ch] = Map5_PulseVolume[ch];
      if (Map5_PulseEnvVol[ch] > 0)
        Map5_PulseEnvVol[ch]--;
      else if (Map5_PulseHalt[ch]) /* Loop flag */
        Map5_PulseEnvVol[ch] = 15;
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Audio: Clock length counter for one pulse channel       */
/*-------------------------------------------------------------------*/
static void Map5_ClockLength(int ch)
{
  if (!Map5_PulseHalt[ch] && Map5_PulseLen[ch] > 0)
    Map5_PulseLen[ch]--;
}

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 5                                              */
/*-------------------------------------------------------------------*/
void Map5_Init()
{
  int nPage;

  /* Initialize Mapper Callbacks */
  MapperInit = Map5_Init;
  MapperWrite = Map5_Write;
  MapperSram = Map5_Sram;
  MapperApu = Map5_Apu;
  MapperReadApu = Map5_ReadApu;
  MapperVSync = Map5_VSync;
  MapperHSync = Map5_HSync;
  MapperPPU = Map0_PPU;
  MapperRenderScreen = Map5_RenderScreen;
  MapperRenderSound = Map5_RenderSound;
  MapperFin = Map5_Fin;

  /* Dynamically allocate large buffers */
  if (!Map5_Wram)
    Map5_Wram = (BYTE *)Frens::f_malloc(MAP5_WRAM_SIZE);
  if (!Map5_ExRam)
    Map5_ExRam = (BYTE *)Frens::f_malloc(MAP5_EXRAM_SIZE);
  if (!Map5_FillNt)
    Map5_FillNt = (BYTE *)Frens::f_malloc(MAP5_FILLNT_SIZE);

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Set ROM Banks to last page */
  ROMBANK0 = ROMLASTPAGE(0);
  ROMBANK1 = ROMLASTPAGE(0);
  ROMBANK2 = ROMLASTPAGE(0);
  ROMBANK3 = ROMLASTPAGE(0);

  /* Set PPU Banks */
  for (nPage = 0; nPage < 8; ++nPage)
    PPUBANK[nPage] = VROMPAGE(nPage);
  InfoNES_SetupChr();

  /* Clear WRAM and ExRAM */
  InfoNES_MemorySet(Map5_Wram, 0x00, MAP5_WRAM_SIZE);
  InfoNES_MemorySet(Map5_ExRam, 0x00, MAP5_EXRAM_SIZE);
  InfoNES_MemorySet(Map5_FillNt, 0xFF, MAP5_FILLNT_SIZE);

  /* Initialize PRG Banking */
  Map5_PrgMode = 3; /* Default to 8K mode */
  for (int i = 0; i < 5; i++)
    Map5_PrgBank[i] = 0xFF;
  /* Last bank is always mapped to ROM */
  Map5_PrgBank[4] = (NesHeader.byRomSize << 1) - 1;

  /* Initialize CHR Banking */
  Map5_ChrMode = 3; /* Default to 1K mode */
  for (int i = 0; i < 8; i++)
    Map5_ChrBankA[i] = i;
  for (int i = 0; i < 4; i++)
    Map5_ChrBankB[i] = i;
  Map5_ChrHigh = 0;
  Map5_LastChrA = 1;

  /* Initialize Protection */
  Map5_RamProt0 = 0;
  Map5_RamProt1 = 0;

  /* Initialize Extended RAM Mode */
  Map5_ExRamMode = 0;

  /* Initialize Nametable Mapping */
  Map5_NtMapping = 0;

  /* Initialize Fill Mode */
  Map5_FillTile = 0xFF;
  Map5_FillColor = 0;
  Map5_UpdateFillNt();

  /* Initialize IRQ */
  Map5_IrqTarget = 0;
  Map5_IrqEnable = 0;
  Map5_IrqPending = 0;
  Map5_InFrame = 0;
  Map5_IrqCounter = 0;

  /* Initialize Multiplier */
  Map5_MultA = 0;
  Map5_MultB = 0;

  /* Initialize Audio */
  for (int ch = 0; ch < 2; ch++)
  {
    Map5_PulseCtrl[ch] = 0;
    Map5_PulseTimerLo[ch] = 0;
    Map5_PulseTimerHi[ch] = 0;
    Map5_PulseEnable[ch] = 0;
    Map5_PulseDuty[ch] = 0;
    Map5_PulseHalt[ch] = 0;
    Map5_PulseConstVol[ch] = 0;
    Map5_PulseVolume[ch] = 0;
    Map5_PulseFreq[ch] = 0;
    Map5_PulseLen[ch] = 0;
    Map5_PulseEnvVol[ch] = 0;
    Map5_PulseEnvDiv[ch] = 0;
    Map5_PulseEnvStart[ch] = 0;
    Map5_PulseSkip[ch] = 0;
    Map5_PulseIndex[ch] = 0;
  }
  Map5_AudioStatus = 0;
  Map5_AudioFrameCounter = 0;
  Map5_AudioFrameDiv = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring(1, 1);
}

/*-------------------------------------------------------------------*/
/*  Finalize Mapper 5 (free dynamically allocated buffers)           */
/*-------------------------------------------------------------------*/
void Map5_Fin()
{
  if (Map5_Wram)
  {
    Frens::f_free(Map5_Wram);
    Map5_Wram = NULL;
  }
  if (Map5_ExRam)
  {
    Frens::f_free(Map5_ExRam);
    Map5_ExRam = NULL;
  }
  if (Map5_FillNt)
  {
    Frens::f_free(Map5_FillNt);
    Map5_FillNt = NULL;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Read from APU ($5000-$5FFF)                             */
/*-------------------------------------------------------------------*/
BYTE Map5_ReadApu(WORD wAddr)
{
  BYTE byRet = (BYTE)(wAddr >> 8);

  switch (wAddr)
  {
  case 0x5015:
  {
    /* Audio status register: return length counter status */
    byRet = 0;
    if (Map5_PulseLen[0] > 0)
      byRet |= 0x01;
    if (Map5_PulseLen[1] > 0)
      byRet |= 0x02;
    break;
  }
  case 0x5204:
  {
    /* IRQ status:
     * Bit 7: IRQ pending
     * Bit 6: In-frame flag
     */
    byRet = 0;
    if (Map5_IrqPending)
      byRet |= 0x80;
    if (Map5_InFrame)
      byRet |= 0x40;
    Map5_IrqPending = 0;
    break;
  }
  case 0x5205:
    byRet = (BYTE)((Map5_MultA * Map5_MultB) & 0xFF);
    break;

  case 0x5206:
    byRet = (BYTE)(((Map5_MultA * Map5_MultB) >> 8) & 0xFF);
    break;

  default:
    if (wAddr >= 0x5C00 && wAddr <= 0x5FFF)
    {
      /* Extended RAM read (modes 2 and 3 allow reading) */
      if (Map5_ExRamMode >= 2)
        byRet = Map5_ExRam[wAddr - 0x5C00];
      else
        byRet = 0;
    }
    break;
  }
  return byRet;
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Write to APU ($5000-$5FFF)                              */
/*-------------------------------------------------------------------*/
void Map5_Apu(WORD wAddr, BYTE byData)
{
  switch (wAddr)
  {
  /*---------------------------------------------------------------*/
  /*  Audio Pulse Channel 1: $5000-$5003                           */
  /*---------------------------------------------------------------*/
  case 0x5000:
    Map5_PulseCtrl[0] = byData;
    Map5_PulseDuty[0] = (byData >> 6) & 0x03;
    Map5_PulseHalt[0] = (byData >> 5) & 0x01;
    Map5_PulseConstVol[0] = (byData >> 4) & 0x01;
    Map5_PulseVolume[0] = byData & 0x0F;
    break;

  case 0x5001:
    /* MMC5 has no sweep unit - register ignored */
    break;

  case 0x5002:
    Map5_PulseTimerLo[0] = byData;
    Map5_UpdatePulseFreq(0);
    break;

  case 0x5003:
    Map5_PulseTimerHi[0] = byData;
    Map5_UpdatePulseFreq(0);
    if (Map5_PulseEnable[0])
      Map5_PulseLen[0] = ApuAtl[(byData >> 3) & 0x1F];
    Map5_PulseEnvStart[0] = 1;
    break;

  /*---------------------------------------------------------------*/
  /*  Audio Pulse Channel 2: $5004-$5007                           */
  /*---------------------------------------------------------------*/
  case 0x5004:
    Map5_PulseCtrl[1] = byData;
    Map5_PulseDuty[1] = (byData >> 6) & 0x03;
    Map5_PulseHalt[1] = (byData >> 5) & 0x01;
    Map5_PulseConstVol[1] = (byData >> 4) & 0x01;
    Map5_PulseVolume[1] = byData & 0x0F;
    break;

  case 0x5005:
    /* MMC5 has no sweep unit - register ignored */
    break;

  case 0x5006:
    Map5_PulseTimerLo[1] = byData;
    Map5_UpdatePulseFreq(1);
    break;

  case 0x5007:
    Map5_PulseTimerHi[1] = byData;
    Map5_UpdatePulseFreq(1);
    if (Map5_PulseEnable[1])
      Map5_PulseLen[1] = ApuAtl[(byData >> 3) & 0x1F];
    Map5_PulseEnvStart[1] = 1;
    break;

  /*---------------------------------------------------------------*/
  /*  Audio PCM / Status: $5010-$5015                              */
  /*---------------------------------------------------------------*/
  case 0x5010:
  case 0x5011:
    /* PCM channel - not commonly used, ignore for now */
    break;

  case 0x5015:
    Map5_AudioStatus = byData;
    Map5_PulseEnable[0] = byData & 0x01;
    Map5_PulseEnable[1] = (byData >> 1) & 0x01;
    if (!Map5_PulseEnable[0])
      Map5_PulseLen[0] = 0;
    if (!Map5_PulseEnable[1])
      Map5_PulseLen[1] = 0;
    break;

  /*---------------------------------------------------------------*/
  /*  PRG/CHR Mode Select: $5100-$5101                             */
  /*---------------------------------------------------------------*/
  case 0x5100:
    Map5_PrgMode = byData & 0x03;
    Map5_Sync_Prg_Banks();
    break;

  case 0x5101:
    Map5_ChrMode = byData & 0x03;
    break;

  /*---------------------------------------------------------------*/
  /*  RAM Protection: $5102-$5103                                  */
  /*---------------------------------------------------------------*/
  case 0x5102:
    Map5_RamProt0 = byData & 0x03;
    break;

  case 0x5103:
    Map5_RamProt1 = byData & 0x03;
    break;

  /*---------------------------------------------------------------*/
  /*  Extended RAM Mode: $5104                                     */
  /*---------------------------------------------------------------*/
  case 0x5104:
    Map5_ExRamMode = byData & 0x03;
    break;

  /*---------------------------------------------------------------*/
  /*  Nametable Mapping: $5105                                     */
  /*---------------------------------------------------------------*/
  case 0x5105:
    Map5_NtMapping = byData;
    Map5_ApplyNtMapping();
    break;

  /*---------------------------------------------------------------*/
  /*  Fill Mode: $5106-$5107                                       */
  /*---------------------------------------------------------------*/
  case 0x5106:
    Map5_FillTile = byData;
    Map5_UpdateFillNt();
    break;

  case 0x5107:
    Map5_FillColor = byData & 0x03;
    Map5_UpdateFillNt();
    break;

  /*---------------------------------------------------------------*/
  /*  PRG Bank Registers: $5113-$5117                              */
  /*---------------------------------------------------------------*/
  case 0x5113:
    Map5_PrgBank[0] = byData;
    SRAMBANK = Map5_WRAMPAGE(byData & 0x07);
    break;

  case 0x5114:
    Map5_PrgBank[1] = byData;
    Map5_Sync_Prg_Banks();
    break;

  case 0x5115:
    Map5_PrgBank[2] = byData;
    Map5_Sync_Prg_Banks();
    break;

  case 0x5116:
    Map5_PrgBank[3] = byData;
    Map5_Sync_Prg_Banks();
    break;

  case 0x5117:
    Map5_PrgBank[4] = byData | 0x80; /* $E000-$FFFF is always ROM */
    Map5_Sync_Prg_Banks();
    break;

  /*---------------------------------------------------------------*/
  /*  CHR Bank Registers Set A: $5120-$5127                        */
  /*---------------------------------------------------------------*/
  case 0x5120:
  case 0x5121:
  case 0x5122:
  case 0x5123:
  case 0x5124:
  case 0x5125:
  case 0x5126:
  case 0x5127:
    Map5_ChrBankA[wAddr & 0x07] = byData | ((WORD)Map5_ChrHigh << 8);
    Map5_LastChrA = 1;
    break;

  /*---------------------------------------------------------------*/
  /*  CHR Bank Registers Set B: $5128-$512B                        */
  /*---------------------------------------------------------------*/
  case 0x5128:
  case 0x5129:
  case 0x512A:
  case 0x512B:
    Map5_ChrBankB[wAddr & 0x03] = byData | ((WORD)Map5_ChrHigh << 8);
    Map5_LastChrA = 0;
    break;

  /*---------------------------------------------------------------*/
  /*  Upper CHR Bank Bits: $5130                                   */
  /*---------------------------------------------------------------*/
  case 0x5130:
    Map5_ChrHigh = byData & 0x03;
    break;

  /*---------------------------------------------------------------*/
  /*  Vertical Split Mode: $5200-$5202                             */
  /*---------------------------------------------------------------*/
  case 0x5200:
  case 0x5201:
  case 0x5202:
    /* Vertical split mode - not implemented */
    break;

  /*---------------------------------------------------------------*/
  /*  IRQ: $5203-$5204                                             */
  /*---------------------------------------------------------------*/
  case 0x5203:
    Map5_IrqTarget = byData;
    break;

  case 0x5204:
    Map5_IrqEnable = byData & 0x80;
    break;

  /*---------------------------------------------------------------*/
  /*  Multiplier: $5205-$5206                                      */
  /*---------------------------------------------------------------*/
  case 0x5205:
    Map5_MultA = byData;
    break;

  case 0x5206:
    Map5_MultB = byData;
    break;

  default:
    /*-------------------------------------------------------------*/
    /*  Extended RAM Writes: $5C00-$5FFF                            */
    /*-------------------------------------------------------------*/
    if (wAddr >= 0x5C00 && wAddr <= 0x5FFF)
    {
      /* ExRAM writes depend on mode:
       * Mode 0,1: write allowed (nametable/attr extension data)
       * Mode 2: general-purpose read/write
       * Mode 3: read-only
       */
      if (Map5_ExRamMode <= 2)
      {
        Map5_ExRam[wAddr - 0x5C00] = byData;
      }
    }
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Write to SRAM ($6000-$7FFF)                             */
/*-------------------------------------------------------------------*/
void Map5_Sram(WORD wAddr, BYTE byData)
{
  if (Map5_RamProt0 == 0x02 && Map5_RamProt1 == 0x01)
  {
    BYTE bank = Map5_PrgBank[0] & 0x07;
    Map5_Wram[0x2000 * bank + (wAddr - 0x6000)] = byData;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Write to ROM area ($8000-$FFFF)                         */
/*  Only writes to WRAM-mapped regions are allowed                   */
/*-------------------------------------------------------------------*/
void Map5_Write(WORD wAddr, BYTE byData)
{
  if (Map5_RamProt0 != 0x02 || Map5_RamProt1 != 0x01)
    return;

  int bank = -1;
  int offset = 0;

  switch (wAddr & 0xE000)
  {
  case 0x8000: /* $8000-$9FFF */
    if (!(Map5_PrgBank[1] & 0x80)) /* RAM mapped */
    {
      bank = Map5_PrgBank[1] & 0x07;
      offset = wAddr - 0x8000;
    }
    break;
  case 0xA000: /* $A000-$BFFF */
    if (!(Map5_PrgBank[2] & 0x80))
    {
      bank = Map5_PrgBank[2] & 0x07;
      offset = wAddr - 0xA000;
    }
    break;
  case 0xC000: /* $C000-$DFFF */
    if (!(Map5_PrgBank[3] & 0x80))
    {
      bank = Map5_PrgBank[3] & 0x07;
      offset = wAddr - 0xC000;
    }
    break;
  }

  if (bank >= 0)
    Map5_Wram[0x2000 * bank + offset] = byData;
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 VSync Function                                          */
/*  Handles per-frame updates: audio envelope/length clocking        */
/*-------------------------------------------------------------------*/
void Map5_VSync()
{
  /* Reset in-frame flag at VBlank */
  Map5_InFrame = 0;
  Map5_IrqCounter = 0;

  /* Clock MMC5 audio envelope and length counters.
   * The MMC5 uses a fixed ~240Hz internal timer for both.
   * At 60fps, that's 4 clocks per frame.
   */
  for (int tick = 0; tick < 4; tick++)
  {
    for (int ch = 0; ch < 2; ch++)
    {
      Map5_ClockEnvelope(ch);
      Map5_ClockLength(ch);
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 H-Sync Function                                         */
/*  Handles per-scanline IRQ generation                              */
/*-------------------------------------------------------------------*/
void Map5_HSync()
{
  if (PPU_Scanline < 240)
  {
    /* In visible scanline range */
    if (!Map5_InFrame)
    {
      Map5_InFrame = 1;
      Map5_IrqCounter = 0;
    }

    Map5_IrqCounter++;

    if (Map5_IrqCounter == Map5_IrqTarget)
    {
      Map5_IrqPending = 1;
      if (Map5_IrqEnable)
      {
        IRQ_REQ;
      }
    }
  }
  else if (PPU_Scanline == 240)
  {
    Map5_InFrame = 0;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Rendering Screen Function                               */
/*  Called when switching between BG and sprite rendering             */
/*  byMode: 1 = BG, 0 = Sprite                                     */
/*-------------------------------------------------------------------*/
void Map5_RenderScreen(BYTE byMode)
{
  Map5_Sync_Chr_Banks(byMode);
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Render Sound Function                                   */
/*  Generates nSamples of MMC5 expansion audio into                  */
/*  wave_buffers_extra[]                                             */
/*-------------------------------------------------------------------*/
void Map5_RenderSound(int nSamples)
{
  for (int i = 0; i < nSamples; i++)
  {
    int total = 0;

    for (int ch = 0; ch < 2; ch++)
    {
      /* Channel must be enabled and have length counter > 0 (or halt set) */
      if (Map5_PulseEnable[ch] &&
          (Map5_PulseLen[ch] > 0 || Map5_PulseHalt[ch]))
      {
        /* Get volume: either constant or envelope */
        BYTE vol = Map5_PulseConstVol[ch] ? Map5_PulseVolume[ch]
                                          : Map5_PulseEnvVol[ch];

        /* Advance phase and render waveform */
        Map5_PulseIndex[ch] += Map5_PulseSkip[ch];
        Map5_PulseIndex[ch] &= 0x1FFFFFFF;

        total += Map5_DutyWaves[Map5_PulseDuty[ch]]
                               [Map5_PulseIndex[ch] >> 24] *
                 vol;
      }
    }

    /* Average two pulse channels to fit in BYTE (max 510 / 2 = 255) */
    wave_buffers_extra[i] = (BYTE)((total + 1) >> 1);
  }
}
