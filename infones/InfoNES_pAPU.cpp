/*===================================================================*/
/*                                                                   */
/*  InfoNES_pAPU.cpp : InfoNES Sound Emulation Function              */
/*                                                                   */
/*  2000/05/29  InfoNES Project ( based on DarcNES and NesterJ )     */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/
#include "K6502.h"
#include "K6502_rw.h"
#include "InfoNES_System.h"
#include "InfoNES_pAPU.h"
#include <algorithm>
#include <string.h>

/*-------------------------------------------------------------------*/
/*   APU Event resources                                             */
/*-------------------------------------------------------------------*/

struct ApuEvent_t ApuEventQueue[APU_EVENT_MAX];
int cur_event;
WORD entertime;

/*-------------------------------------------------------------------*/
/*   APU Register Write Functions                                    */
/*-------------------------------------------------------------------*/

#define APU_WRITEFUNC(name, evtype)                                \
  void ApuWrite##name(WORD addr, BYTE value)                       \
  {                                                                \
    ApuEventQueue[cur_event].time = getPassedClocks() - entertime; \
    ApuEventQueue[cur_event].type = APUET_W_##evtype;              \
    ApuEventQueue[cur_event].data = value;                         \
    cur_event++;                                                   \
  }

// 普通にバグってる

APU_WRITEFUNC(C1a, C1A);
APU_WRITEFUNC(C1b, C1B);
APU_WRITEFUNC(C1c, C1C);
APU_WRITEFUNC(C1d, C1D);

APU_WRITEFUNC(C2a, C2A);
APU_WRITEFUNC(C2b, C2B);
APU_WRITEFUNC(C2c, C2C);
APU_WRITEFUNC(C2d, C2D);

APU_WRITEFUNC(C3a, C3A);
APU_WRITEFUNC(C3b, C3B);
APU_WRITEFUNC(C3c, C3C);
APU_WRITEFUNC(C3d, C3D);

APU_WRITEFUNC(C4a, C4A);
APU_WRITEFUNC(C4b, C4B);
APU_WRITEFUNC(C4c, C4C);
APU_WRITEFUNC(C4d, C4D);

APU_WRITEFUNC(C5a, C5A);
APU_WRITEFUNC(C5b, C5B);
APU_WRITEFUNC(C5c, C5C);
APU_WRITEFUNC(C5d, C5D);

APU_WRITEFUNC(Control, CTRL);

ApuWritefunc pAPUSoundRegs[20] =
    {
        ApuWriteC1a,
        ApuWriteC1b,
        ApuWriteC1c,
        ApuWriteC1d,
        ApuWriteC2a,
        ApuWriteC2b,
        ApuWriteC2c,
        ApuWriteC2d,
        ApuWriteC3a,
        ApuWriteC3b,
        ApuWriteC3c,
        ApuWriteC3d,
        ApuWriteC4a,
        ApuWriteC4b,
        ApuWriteC4c,
        ApuWriteC4d,
        ApuWriteC5a,
        ApuWriteC5b,
        ApuWriteC5c,
        ApuWriteC5d,
};

/*-------------------------------------------------------------------*/
/*   APU resources                                                   */
/*-------------------------------------------------------------------*/

BYTE wave_buffers[5][735]; /* 44100 / 60 = 735 samples per sync */

BYTE ApuCtrl;
BYTE ApuCtrlNew;

/*-------------------------------------------------------------------*/
/*   APU Quality resources                                           */
/*-------------------------------------------------------------------*/

int ApuQuality;

DWORD ApuPulseMagic;
DWORD ApuTriangleMagic;
DWORD ApuNoiseMagic;
unsigned int ApuSamplesPerSync16;
unsigned int ApuCyclesPerSample;
unsigned int ApuSampleRate;
DWORD ApuCycleRate;

struct ApuQualityData_t
{
  DWORD pulse_magic;
  DWORD triangle_magic;
  DWORD noise_magic;
  unsigned int samples_per_sync_16;
  unsigned int cycles_per_sample;
  unsigned int sample_rate;
  DWORD cycle_rate;
} ApuQual[] = {
    // {0xa2567000, 0xa2567000, 0xa2567000, 183, 164, 11025, 1062658},
    // {0x512b3800, 0x512b3800, 0x512b3800, 367, 82, 22050, 531329},
    // {0x289d9c00, 0x289d9c00, 0x289d9c00, 735, 41, 44100, 265664},
    {0xa2567000, 0xa2567000, 0xa2567000, 45963, 164, 11025, 664935},
    {0x512b3800, 0x512b3800, 0x512b3800, 91926, 82, 22050, 1329870},
    {0x289d9c00, 0x289d9c00, 0x289d9c00, 184402, 41, 44100, 2659741},
};

// 44100/60/262*65536 = 183850.99236641222
// (44100*1.001)/60/262*65536 = 184034.84335877863
// (44100*1.003)/60/262*65536 = 184402.54534351142

// cycle_rate
// 1789773 / 44100 * 65536 = 2659740.665034014

/*-------------------------------------------------------------------*/
/*  Rectangle Wave #1 resources                                      */
/*-------------------------------------------------------------------*/
BYTE ApuC1a,
    ApuC1b, ApuC1c, ApuC1d;

BYTE *ApuC1Wave;
DWORD ApuC1Skip;
DWORD ApuC1Index;
int32_t ApuC1EnvPhase;
BYTE ApuC1EnvVol;
BYTE ApuC1Atl;
int32_t ApuC1SweepPhase;
DWORD ApuC1Freq;

/*-------------------------------------------------------------------*/
/*  Rectangle Wave #2 resources                                      */
/*-------------------------------------------------------------------*/
BYTE ApuC2a, ApuC2b, ApuC2c, ApuC2d;

BYTE *ApuC2Wave;
DWORD ApuC2Skip;
DWORD ApuC2Index;
int32_t ApuC2EnvPhase;
BYTE ApuC2EnvVol;
BYTE ApuC2Atl;
int32_t ApuC2SweepPhase;
DWORD ApuC2Freq;

/*-------------------------------------------------------------------*/
/*  Triangle Wave resources                                          */
/*-------------------------------------------------------------------*/
BYTE ApuC3a, ApuC3b, ApuC3c, ApuC3d;

DWORD ApuC3Skip;
DWORD ApuC3Index;
BYTE ApuC3Atl;
DWORD ApuC3Llc; /* Linear Length Counter */
bool ApuC3ReloadFlag;
// BYTE ApuC3WriteLatency;
// BYTE ApuC3CounterStarted;

/*-------------------------------------------------------------------*/
/*  Noise resources                                                  */
/*-------------------------------------------------------------------*/
BYTE ApuC4a, ApuC4b, ApuC4c, ApuC4d;

DWORD ApuC4Sr;  /* Shift register */
DWORD ApuC4Fdc; /* Frequency divide counter */
DWORD ApuC4Skip;
DWORD ApuC4Index;
BYTE ApuC4Atl;
BYTE ApuC4EnvVol;
int32_t ApuC4EnvPhase;

/*-------------------------------------------------------------------*/
/*  DPCM resources                                                   */
/*-------------------------------------------------------------------*/
BYTE ApuC5Reg[4];
BYTE ApuC5Enable;
BYTE ApuC5Looping;
BYTE ApuC5CurByte;
BYTE ApuC5DpcmValue;

int ApuC5Freq;
int ApuC5Phaseacc;

WORD ApuC5Address, ApuC5CacheAddr;
int ApuC5DmaLength, ApuC5CacheDmaLength;

/*-------------------------------------------------------------------*/
/*  Wave Data                                                        */
/*-------------------------------------------------------------------*/
BYTE __not_in_flash_func(pulse_25)[0x20] = {
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

BYTE __not_in_flash_func(pulse_50)[0x20] = {
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

BYTE __not_in_flash_func(pulse_75)[0x20] = {
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

BYTE __not_in_flash_func(pulse_87)[0x20] = {
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x00,
    0x00,
    0x00,
    0x00,
};

BYTE __not_in_flash_func(triangle_50)[0x20] = {
    0x00,
    0x10,
    0x20,
    0x30,
    0x40,
    0x50,
    0x60,
    0x70,
    0x80,
    0x90,
    0xa0,
    0xb0,
    0xc0,
    0xd0,
    0xe0,
    0xf0,
    0xff,
    0xef,
    0xdf,
    0xcf,
    0xbf,
    0xaf,
    0x9f,
    0x8f,
    0x7f,
    0x6f,
    0x5f,
    0x4f,
    0x3f,
    0x2f,
    0x1f,
    0x0f,
};

BYTE *__not_in_flash_func(pulse_waves)[4] = {
    pulse_87,
    pulse_75,
    pulse_50,
    pulse_25,
};

/*-------------------------------------------------------------------*/
/*  Active Time Left Data                                            */
/*-------------------------------------------------------------------*/
BYTE __not_in_flash_func(ApuAtl)[0x20] =
    {
        5,
        127,
        10,
        1,
        19,
        2,
        40,
        3,
        80,
        4,
        30,
        5,
        7,
        6,
        13,
        7,
        6,
        8,
        12,
        9,
        24,
        10,
        48,
        11,
        96,
        12,
        36,
        13,
        8,
        14,
        16,
        15,
};

/*-------------------------------------------------------------------*/
/* Frequency Limit of Rectangle Channels                             */
/*-------------------------------------------------------------------*/
WORD __not_in_flash_func(ApuFreqLimit)[8] =
    {
        0x3FF, 0x555, 0x666, 0x71C, 0x787, 0x7C1, 0x7E0, 0x7F0};

/*-------------------------------------------------------------------*/
/* Noise Frequency Lookup Table                                      */
/*-------------------------------------------------------------------*/
DWORD __not_in_flash_func(ApuNoiseFreq)[16] =
    {
        4, 8, 16, 32, 64, 96, 128, 160,
        202, 254, 380, 508, 762, 1016, 2034, 4068};

/*-------------------------------------------------------------------*/
/* DMC Transfer Clocks Table                                          */
/*-------------------------------------------------------------------*/
DWORD __not_in_flash_func(ApuDpcmCycles)[16] =
    {
        428, 380, 340, 320, 286, 254, 226, 214,
        190, 160, 142, 128, 106, 85, 72, 54};

/*===================================================================*/
/*                                                                   */
/*      ApuRenderingWave1() : Rendering Rectangular Wave #1          */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/* Write registers of rectangular wave #1                            */
/*-------------------------------------------------------------------*/

int __not_in_flash_func(ApuWriteWave1)(int cycles, int event)
{
  /* APU Reg Write Event */
  while ((event < cur_event) && (ApuEventQueue[event].time < cycles))
  {
    if ((ApuEventQueue[event].type & APUET_MASK) == APUET_C1)
    {
      switch (ApuEventQueue[event].type & 0x03)
      {
      case 0:
        ApuC1a = ApuEventQueue[event].data;
        ApuC1Wave = pulse_waves[ApuC1DutyCycle >> 6];
        break;

      case 1:
        ApuC1b = ApuEventQueue[event].data;
        break;

      case 2:
        ApuC1c = ApuEventQueue[event].data;
        ApuC1Freq = ((((WORD)ApuC1d & 0x07) << 8) + ApuC1c);
        ApuC1Atl = ApuAtl[(ApuC1d & 0xf8) >> 3];

        if (ApuC1Freq)
        {
          ApuC1Skip = ApuPulseMagic / (ApuC1Freq / 2);
        }
        else
        {
          ApuC1Skip = 0;
        }
        break;

      case 3:
        ApuC1d = ApuEventQueue[event].data;
        ApuC1Freq = ((((WORD)ApuC1d & 0x07) << 8) + ApuC1c);
        ApuC1Atl = ApuAtl[(ApuC1d & 0xf8) >> 3];

        if (ApuC1Freq)
        {
          ApuC1Skip = ApuPulseMagic / (ApuC1Freq / 2);
        }
        else
        {
          ApuC1Skip = 0;
        }

        ApuC1EnvVol = 15;
        break;
      }
    }
    else if (ApuEventQueue[event].type == APUET_W_CTRL)
    {
      ApuCtrlNew = ApuEventQueue[event].data;

      if (!(ApuEventQueue[event].data & (1 << 0)))
      {
        ApuC1Atl = 0;
      }
    }
    event++;
  }
  return event;
}

/*-------------------------------------------------------------------*/
/* Rendering rectangular wave #1                                     */
/*-------------------------------------------------------------------*/

void __not_in_flash_func(ApuRenderingWave1)(int n)
{
  ApuCtrlNew = ApuCtrl;
  ApuWriteWave1(ApuCyclesPerSample * (n + 1), 0);

  if ((ApuCtrlNew & 0x01) && (ApuC1Atl || ApuC1Hold) &&
      !(ApuC1Freq < 8 || (!ApuC1SweepIncDec && ApuC1Freq > ApuC1FreqLimit)))
  {
    auto vol = ApuC1Env ? ApuC1Vol : ApuC1EnvVol;
    for (unsigned int i = 0; i < n; i++)
    {
      /* Wave Rendering */
      ApuC1Index += ApuC1Skip;
      ApuC1Index &= 0x1fffffff;
      wave_buffers[0][i] = ApuC1Wave[ApuC1Index >> 24] * vol;
    }
  }
  else
  {
    memset(wave_buffers[0], 0, 2 * n);
  }
}

/*===================================================================*/
/*                                                                   */
/*      ApuRenderingWave2() : Rendering Rectangular Wave #2          */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/* Write registers of rectangular wave #2                           */
/*-------------------------------------------------------------------*/

int __not_in_flash_func(ApuWriteWave2)(int cycles, int event)
{
  /* APU Reg Write Event */
  while ((event < cur_event) && (ApuEventQueue[event].time < cycles))
  {
    if ((ApuEventQueue[event].type & APUET_MASK) == APUET_C2)
    {
      switch (ApuEventQueue[event].type & 0x03)
      {
      case 0:
        ApuC2a = ApuEventQueue[event].data;
        ApuC2Wave = pulse_waves[ApuC2DutyCycle >> 6];
        break;

      case 1:
        ApuC2b = ApuEventQueue[event].data;
        break;

      case 2:
        ApuC2c = ApuEventQueue[event].data;
        ApuC2Freq = ((((WORD)ApuC2d & 0x07) << 8) + ApuC2c);
        ApuC2Atl = ApuAtl[(ApuC2d & 0xf8) >> 3];

        if (ApuC2Freq)
        {
          ApuC2Skip = ApuPulseMagic / (ApuC2Freq / 2);
        }
        else
        {
          ApuC2Skip = 0;
        }
        break;

      case 3:
        ApuC2d = ApuEventQueue[event].data;
        ApuC2Freq = ((((WORD)ApuC2d & 0x07) << 8) + ApuC2c);
        ApuC2Atl = ApuAtl[(ApuC2d & 0xf8) >> 3];

        if (ApuC2Freq)
        {
          ApuC2Skip = ApuPulseMagic / (ApuC2Freq / 2);
        }
        else
        {
          ApuC2Skip = 0;
        }
        ApuC2EnvVol = 15;
        break;
      }
    }
    else if (ApuEventQueue[event].type == APUET_W_CTRL)
    {
      ApuCtrlNew = ApuEventQueue[event].data;

      if (!(ApuEventQueue[event].data & (1 << 1)))
      {
        ApuC2Atl = 0;
      }
    }
    event++;
  }
  return event;
}

/*-------------------------------------------------------------------*/
/* Rendering rectangular wave #2                                     */
/*-------------------------------------------------------------------*/

void __not_in_flash_func(ApuRenderingWave2)(int n)
{
  ApuCtrlNew = ApuCtrl;
  ApuWriteWave2(ApuCyclesPerSample * (n + 1), 0);

  if ((ApuCtrlNew & 0x02) && (ApuC2Atl || ApuC2Hold) &&
      !(ApuC2Freq < 8 || (!ApuC2SweepIncDec && ApuC2Freq > ApuC2FreqLimit)))
  {
    auto vol = ApuC2Env ? ApuC2Vol : ApuC2EnvVol;
    for (unsigned int i = 0; i < n; i++)
    {
      /* Wave Rendering */
      ApuC2Index += ApuC2Skip;
      ApuC2Index &= 0x1fffffff;
      wave_buffers[1][i] = ApuC2Wave[ApuC2Index >> 24] * vol;
    }
  }
  else
  {
    memset(wave_buffers[1], 0, 2 * n);
  }
}

/*===================================================================*/
/*                                                                   */
/*      ApuRenderingWave3() : Rendering Triangle Wave                */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/* Write registers of triangle wave #3                              */
/*-------------------------------------------------------------------*/

int __not_in_flash_func(ApuWriteWave3)(int cycles, int event)
{
  /* APU Reg Write Event */
  while ((event < cur_event) && (ApuEventQueue[event].time < cycles))
  {
    if ((ApuEventQueue[event].type & APUET_MASK) == APUET_C3)
    {
      switch (ApuEventQueue[event].type & 3)
      {
      case 0:
        ApuC3a = ApuEventQueue[event].data;
        break;

      case 1:
        ApuC3b = ApuEventQueue[event].data;
        break;

      case 2:
        ApuC3c = ApuEventQueue[event].data;
        if (ApuC3Freq)
        {
          ApuC3Skip = ApuTriangleMagic / ApuC3Freq;
        }
        else
        {
          ApuC3Skip = 0;
        }
        break;

      case 3:
        ApuC3d = ApuEventQueue[event].data;
        ApuC3Atl = ApuC3LengthCounter;
        ApuC3ReloadFlag = true;
        if (ApuC3Freq)
        {
          ApuC3Skip = ApuTriangleMagic / ApuC3Freq;
        }
        else
        {
          ApuC3Skip = 0;
        }
      }
    }
    else if (ApuEventQueue[event].type == APUET_W_CTRL)
    {
      ApuCtrlNew = ApuEventQueue[event].data;

      if (!(ApuEventQueue[event].data & (1 << 2)))
      {
        ApuC3Atl = 0;
        ApuC3Llc = 0;
      }
    }
    event++;
  }
  return event;
}

/*-------------------------------------------------------------------*/
/* Rendering triangle wave #3                                        */
/*-------------------------------------------------------------------*/

void __not_in_flash_func(ApuRenderingWave3)(int n)
{
  ApuCtrlNew = ApuCtrl;
  ApuWriteWave3(ApuCyclesPerSample * (n + 1), 0);

  if ((ApuCtrlNew & 0x04) && ApuC3Atl > 0 && ApuC3Llc > 0 && ApuC3Freq >= 8)
  {
    for (unsigned int i = 0; i < n; i++)
    {
      /* Wave Rendering */
      ApuC3Index += ApuC3Skip;
      ApuC3Index &= 0x1fffffff;
      wave_buffers[2][i] = triangle_50[ApuC3Index >> 24];
    }
  }
  else
  {
    memset(wave_buffers[2], 0, 2 * n);
  }
}

/*===================================================================*/
/*                                                                   */
/*      ApuRenderingWave4() : Rendering Noise                        */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/* Write registers of noise channel #4                              */
/*-------------------------------------------------------------------*/

int __not_in_flash_func(ApuWriteWave4)(int cycles, int event)
{
  /* APU Reg Write Event */
  while ((event < cur_event) && (ApuEventQueue[event].time < cycles))
  {
    if ((ApuEventQueue[event].type & APUET_MASK) == APUET_C4)
    {
      switch (ApuEventQueue[event].type & 3)
      {
      case 0:
        ApuC4a = ApuEventQueue[event].data;
        break;

      case 1:
        ApuC4b = ApuEventQueue[event].data;
        break;

      case 2:
        ApuC4c = ApuEventQueue[event].data;

        // if (ApuC4Small)
        // {
        //   ApuC4Sr = 0x001f;
        // }
        // else
        // {
        //   ApuC4Sr = 0x01ff;
        // }

        /* Frequency */
        if (ApuC4Freq)
        {
          ApuC4Skip = ApuNoiseMagic / ApuC4Freq;
        }
        else
        {
          ApuC4Skip = 0;
        }
        ApuC4Atl = ApuC4LengthCounter;
        break;

      case 3:
        ApuC4d = ApuEventQueue[event].data;

        /* Frequency */
        if (ApuC4Freq)
        {
          ApuC4Skip = ApuNoiseMagic / ApuC4Freq;
        }
        else
        {
          ApuC4Skip = 0;
        }
        ApuC4Atl = ApuC4LengthCounter;
        ApuC4EnvVol = 15;
      }
    }
    else if (ApuEventQueue[event].type == APUET_W_CTRL)
    {
      ApuCtrlNew = ApuEventQueue[event].data;

      if (!(ApuEventQueue[event].data & (1 << 3)))
      {
        ApuC4Atl = 0;
      }
    }
    event++;
  }
  return event;
}

/*-------------------------------------------------------------------*/
/* Rendering noise channel #4                                        */
/*-------------------------------------------------------------------*/

void __not_in_flash_func(ApuRenderingWave4)(int n)
{
  ApuCtrlNew = ApuCtrl;
  ApuWriteWave4(ApuCyclesPerSample * (n + 1), 0);

  if ((ApuCtrlNew & 0x08) && ApuC4Atl)
  {
    int shift = ApuC4Small ? 6 : 1;
    for (unsigned int i = 0; i < n; i++)
    {
      /* Wave Rendering */
      ApuC4Index += ApuC4Skip;
      if (ApuC4Index > 0xffffff)
      {
        int f = (ApuC4Sr ^ (ApuC4Sr >> shift)) & 1;
        ApuC4Sr = (ApuC4Sr >> 1) | (f << 14);

        ApuC4Index &= 0xffffff;
      }

      if (!(ApuC4Sr & 1))
      {
        if (ApuC4Env)
        {
          wave_buffers[3][i] = ApuC4Vol;
        }
        else
        {
          wave_buffers[3][i] = ApuC4EnvVol;
        }
      }
      else
      {
        wave_buffers[3][i] = 0;
      }
    }
  }
  else
  {
    memset(wave_buffers[3], 0, n << 1);
  }
}

/*===================================================================*/
/*                                                                   */
/*      ApuRenderingWave5() : Rendering DPCM channel #5              */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/* Write registers of DPCM channel #5                               */
/*-------------------------------------------------------------------*/

int __not_in_flash_func(ApuWriteWave5)(int cycles, int event)
{
  /* APU Reg Write Event */
  while ((event < cur_event) && (ApuEventQueue[event].time < cycles))
  {
    if ((ApuEventQueue[event].type & APUET_MASK) == APUET_C5)
    {
      ApuC5Reg[ApuEventQueue[event].type & 3] = ApuEventQueue[event].data;

      switch (ApuEventQueue[event].type & 3)
      {
      case 0:
        ApuC5Freq = ApuDpcmCycles[(ApuEventQueue[event].data & 0x0F)] << 16;
        ApuC5Looping = ApuEventQueue[event].data & 0x40;
        break;
      case 1:
        ApuC5DpcmValue = (ApuEventQueue[event].data & 0x7F) >> 1;
        break;
      case 2:
        ApuC5CacheAddr = 0xC000 + (WORD)(ApuEventQueue[event].data << 6);
        break;
      case 3:
        ApuC5CacheDmaLength = ((ApuEventQueue[event].data << 4) + 1) << 3;
        break;
      }
    }
    else if (ApuEventQueue[event].type == APUET_W_CTRL)
    {
      ApuCtrlNew = ApuEventQueue[event].data;

      if (!(ApuEventQueue[event].data & (1 << 4)))
      {
        ApuC5Enable = 0;
        ApuC5DmaLength = 0;
      }
      else
      {
        ApuC5Enable = 0xFF;
        if (!ApuC5DmaLength)
        {
          ApuC5Address = ApuC5CacheAddr;
          ApuC5DmaLength = ApuC5CacheDmaLength;
        }
      }
    }
    event++;
  }
  return event;
}

/*-------------------------------------------------------------------*/
/* Rendering DPCM channel #5                                         */
/*-------------------------------------------------------------------*/

void __not_in_flash_func(ApuRenderingWave5)(int n)
{
  ApuCtrlNew = ApuCtrl;
  ApuWriteWave5(ApuCyclesPerSample * (n + 1), 0);

  if (ApuCtrlNew & 0x10)
  {
    for (unsigned int i = 0; i < n; i++)
    {
      if (ApuC5DmaLength)
      {
        ApuC5Phaseacc -= ApuCycleRate;

        while (ApuC5Phaseacc < 0)
        {
          ApuC5Phaseacc += ApuC5Freq;
          if (!(ApuC5DmaLength & 7))
          {
            ApuC5CurByte = K6502_Read(ApuC5Address);
            if (0xFFFF == ApuC5Address)
              ApuC5Address = 0x8000;
            else
              ApuC5Address++;
          }
          if (!(--ApuC5DmaLength))
          {
            if (ApuC5Looping)
            {
              ApuC5Address = ApuC5CacheAddr;
              ApuC5DmaLength = ApuC5CacheDmaLength;
            }
            else
            {
              ApuC5Enable = 0;
              break;
            }
          }

          // positive delta
          if (ApuC5CurByte & (1 << ((ApuC5DmaLength & 7) ^ 7)))
          {
            if (ApuC5DpcmValue < 0x3F)
              ApuC5DpcmValue += 1;
          }
          else
          {
            // negative delta
            if (ApuC5DpcmValue > 1)
              ApuC5DpcmValue -= 1;
          }
        }
      }

      /* Wave Rendering */
      wave_buffers[4][i] = ApuC5DpcmValue;
    }
  }
  else
  {
    memset(wave_buffers[4], 0, n << 1);
  }
}

/*===================================================================*/
/*                                                                   */
/*     InfoNES_pApuVsync() : Callback Function per Vsync             */
/*                                                                   */
/*===================================================================*/

void InfoNES_pAPUVsync()
{
  if (ApuC1Atl)
  {
    ApuC1Atl--;
  }

  /* Envelope decay at a rate of ( Envelope Delay + 1 ) / 240 secs */
  ApuC1EnvPhase -= 4;
  while (ApuC1EnvPhase < 0)
  {
    ApuC1EnvPhase += ApuC1EnvDelay;

    if (ApuC1Hold)
    {
      ApuC1EnvVol = (ApuC1EnvVol - 1) & 0x0f;
    }
    else if (ApuC1EnvVol > 0)
    {
      --ApuC1EnvVol;
    }
  }

  /* Frequency sweeping at a rate of ( Sweep Delay + 1) / 120 secs */
  if (ApuC1SweepOn && ApuC1SweepShifts)
  {
    ApuC1SweepPhase -= 2; /* 120/60 */
    while (ApuC1SweepPhase < 0)
    {
      ApuC1SweepPhase += ApuC1SweepDelay;

      if (ApuC1SweepIncDec) /* ramp up */
      {
        /* Rectangular #1 */
        ApuC1Freq += ~(ApuC1Freq >> ApuC1SweepShifts);
      }
      else
      {
        /* ramp down */
        ApuC1Freq += (ApuC1Freq >> ApuC1SweepShifts);
      }

      ApuC1Skip = ApuPulseMagic / (ApuC1Freq / 2);
    }
  }

  if (ApuC2Atl)
  {
    ApuC2Atl--;
  }

  /* Envelope decay at a rate of ( Envelope Delay + 1 ) / 240 secs */
  ApuC2EnvPhase -= 4;
  while (ApuC2EnvPhase < 0)
  {
    ApuC2EnvPhase += ApuC2EnvDelay;

    if (ApuC2Hold)
    {
      ApuC2EnvVol = (ApuC2EnvVol - 1) & 0x0f;
    }
    else if (ApuC2EnvVol > 0)
    {
      --ApuC2EnvVol;
    }
  }
  //  printf("C2:%d:%d:%d\n", ApuC2Env, ApuC2EnvPhase, ApuC2EnvVol);

  /* Frequency sweeping at a rate of ( Sweep Delay + 1) / 120 secs */
  if (ApuC2SweepOn && ApuC2SweepShifts)
  {
    ApuC2SweepPhase -= 2; /* 120/60 */
    while (ApuC2SweepPhase < 0)
    {
      ApuC2SweepPhase += ApuC2SweepDelay;

      if (ApuC2SweepIncDec) /* ramp up */
      {
        /* Rectangular #2 */
        ApuC2Freq -= (ApuC2Freq >> ApuC2SweepShifts);
      }
      else
      {
        /* ramp down */
        ApuC2Freq += (ApuC2Freq >> ApuC2SweepShifts);
      }
      ApuC2Skip = ApuPulseMagic / (ApuC2Freq / 2);
    }
  }

  if (ApuC3ReloadFlag)
  {
    ApuC3Llc = ApuC3LinearLength;
  }
  else
  {
    if (ApuC3Llc > 0)
    {
      ApuC3Llc = std::max<int>(0, (int)ApuC3Llc - 4 * 64);
    }
  }
  if (!ApuC3Holdnote)
  {
    ApuC3ReloadFlag = false;
  }

  if (ApuC3Atl > 0 && !ApuC3Holdnote)
  {
    ApuC3Atl--;
  }
  // printf("3: %d, %d, %d, %d\n", ApuC3Atl, ApuC3Llc, ApuC3Freq, ApuC3Holdnote);

  if (ApuC4Atl && !ApuC4Hold)
  {
    ApuC4Atl--;
  }

  /* Envelope decay at a rate of ( Envelope Delay + 1 ) / 240 secs */
  ApuC4EnvPhase -= 4;
  while (ApuC4EnvPhase < 0)
  {
    ApuC4EnvPhase += ApuC4EnvDelay;

    if (ApuC4Hold)
    {
      ApuC4EnvVol = (ApuC4EnvVol - 1) & 0x0f;
    }
    else if (ApuC4EnvVol > 0)
    {
      --ApuC4EnvVol;
    }
  }
  // printf("C2: %02x %02x %02x %02x: %d %d\n", ApuC2a, ApuC2b, ApuC2c, ApuC2d, ApuC2Atl, ApuC2EnvVol);
  // printf("C4: %02x  %02x %02x: %d %d\n", ApuC4a, ApuC4c, ApuC4d, ApuC4Atl, ApuC4EnvVol);
  // printf("C5: %02x %02x %02x %02x, lp%d, v%02x, %04x, %d\n",
  //        ApuC5Reg[0], ApuC5Reg[1], ApuC5Reg[2], ApuC5Reg[3],
  //        ApuC5Looping, ApuC5DpcmValue, ApuC5Address, ApuC5DmaLength);
}

/*===================================================================*/
/*                                                                   */
/*     InfoNES_pApuHsync() : Callback Function per Hsync             */
/*                                                                   */
/*===================================================================*/

uint32_t leftSamples16 = 0;

void __not_in_flash_func(InfoNES_pAPUHsync)(bool enabled)
{
  auto n16 = ApuSamplesPerSync16 + leftSamples16;
  auto n = n16 >> 16;
  leftSamples16 = n16 - (n << 16);

  int bufferLeft = InfoNES_GetSoundBufferSize();
  n = std::min<int>(bufferLeft, n);

  if (enabled)
  {
    ApuRenderingWave1(n);
    ApuRenderingWave2(n);
    ApuRenderingWave3(n);
    ApuRenderingWave4(n);
    ApuRenderingWave5(n);
    ApuCtrl = ApuCtrlNew;
  }
  else
  {
    memset(&wave_buffers[0][0], 0, n);
    memset(&wave_buffers[1][0], 0, n);
    memset(&wave_buffers[2][0], 0, n);
    memset(&wave_buffers[3][0], 0, n);
    memset(&wave_buffers[4][0], 0, n);
  }

  InfoNES_SoundOutput(n,
                      wave_buffers[0], wave_buffers[1], wave_buffers[2],
                      wave_buffers[3], wave_buffers[4]);

  entertime = getPassedClocks();
  cur_event = 0;
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_pApuInit() : Initialize pApu                   */
/*                                                                   */
/*===================================================================*/

void InfoNES_pAPUInit(void)
{
  /* Sound Hardware Init */
  InfoNES_SoundInit();

  ApuQuality = pAPU_QUALITY - 1; // 1: 22050, 2: 44100 [samples/sec]

  ApuPulseMagic = ApuQual[ApuQuality].pulse_magic;
  ApuTriangleMagic = ApuQual[ApuQuality].triangle_magic;
  ApuNoiseMagic = ApuQual[ApuQuality].noise_magic;
  ApuSamplesPerSync16 = ApuQual[ApuQuality].samples_per_sync_16;
  ApuCyclesPerSample = ApuQual[ApuQuality].cycles_per_sample;
  ApuSampleRate = ApuQual[ApuQuality].sample_rate;
  ApuCycleRate = ApuQual[ApuQuality].cycle_rate;

  InfoNES_SoundOpen((ApuSamplesPerSync16 + 65535) >> 16, ApuSampleRate);

  /*-------------------------------------------------------------------*/
  /* Initialize Rectangular, Noise Wave's Regs                         */
  /*-------------------------------------------------------------------*/
  ApuCtrl = ApuCtrlNew = 0;
  ApuC1Wave = pulse_50;
  ApuC2Wave = pulse_50;

  ApuC1a = ApuC1b = ApuC1c = ApuC1d = 0;
  ApuC2a = ApuC2b = ApuC2c = ApuC2d = 0;
  ApuC4a = ApuC4b = ApuC4c = ApuC4d = 0;

  ApuC1Skip = ApuC2Skip = ApuC4Skip = 0;
  ApuC1Index = ApuC2Index = ApuC4Index = 0;
  ApuC1EnvPhase = ApuC2EnvPhase = ApuC4EnvPhase = 0;
  ApuC1EnvVol = ApuC2EnvVol = ApuC4EnvVol = 0;
  ApuC1Atl = ApuC2Atl = ApuC4Atl = 0;
  ApuC1SweepPhase = ApuC2SweepPhase = 0;
  ApuC1Freq = ApuC2Freq = ApuC4Freq = 0;
  ApuC4Sr = 1;
  ApuC4Fdc = 0;

  /*-------------------------------------------------------------------*/
  /*   Initialize Triangle Wave's Regs                                 */
  /*-------------------------------------------------------------------*/
  ApuC3a = ApuC3b = ApuC3c = ApuC3d = 0;
  ApuC3Atl = ApuC3Llc = 0;
  ApuC3ReloadFlag = false;
  // ApuC3WriteLatency = 3; /* Magic Number */
  //  ApuC3CounterStarted = 0x00;

  /*-------------------------------------------------------------------*/
  /*   Initialize DPCM's Regs                                          */
  /*-------------------------------------------------------------------*/
  ApuC5Reg[0] = ApuC5Reg[1] = ApuC5Reg[2] = ApuC5Reg[3] = 0;
  ApuC5Enable = ApuC5Looping = ApuC5CurByte = ApuC5DpcmValue = 0;
  ApuC5Freq = ApuC5Phaseacc;
  ApuC5Address = ApuC5CacheAddr = 0;
  ApuC5DmaLength = ApuC5CacheDmaLength = 0;

  /*-------------------------------------------------------------------*/
  /*   Initialize Wave Buffers                                         */
  /*-------------------------------------------------------------------*/
  InfoNES_MemorySet((void *)wave_buffers[0], 0, 735);
  InfoNES_MemorySet((void *)wave_buffers[1], 0, 735);
  InfoNES_MemorySet((void *)wave_buffers[2], 0, 735);
  InfoNES_MemorySet((void *)wave_buffers[3], 0, 735);
  InfoNES_MemorySet((void *)wave_buffers[4], 0, 735);

  entertime = getPassedClocks();
  cur_event = 0;
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_pApuDone() : Finalize pApu                     */
/*                                                                   */
/*===================================================================*/

void InfoNES_pAPUDone(void)
{
  InfoNES_SoundClose();
}

/*
 * End of InfoNES_pAPU.cpp
 */
