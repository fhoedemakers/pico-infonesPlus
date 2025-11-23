#include "InfoNES_SaveState.h"
#include "InfoNES.h"
#include "InfoNES_System.h"
#include "InfoNES_Mapper.h"
#include "InfoNES_pAPU.h"
#include "K6502.h"
#include "ff.h"
#include <cstring>
#include <memory>

// 6502 registers
extern WORD PC;
extern BYTE SP;
extern BYTE F;
extern BYTE A;
extern BYTE X;
extern BYTE Y;
extern BYTE IRQ_State;
extern BYTE IRQ_Wiring;
extern BYTE NMI_State;
extern BYTE NMI_Wiring;
extern int  g_wPassedClocks;
extern int  g_wCurrentClocks;

// Core emulator globals
extern BYTE *RAM;
extern BYTE *SRAM;
extern BYTE *PPURAM;
extern BYTE *SPRRAM;
extern BYTE *ChrBuf;
extern BYTE *PPUBANK[16];
extern BYTE *ROM;
extern BYTE *ROMBANK[4];
extern BYTE *VROM; // CHR ROM base
extern struct NesHeader_tag NesHeader;
extern BYTE MapperNo;
extern BYTE ROM_Mirroring;

// PPU state (horizontal scroll only in your build)
extern BYTE PPU_R0, PPU_R1, PPU_R2, PPU_R3, PPU_R7;
extern BYTE PPU_Scr_H_Byte;
extern BYTE PPU_Scr_H_Bit;
extern WORD PPU_Addr;
extern WORD PPU_Temp;
extern WORD PPU_Increment;
extern WORD PPU_Scanline;
extern BYTE PPU_NameTableBank;
extern BYTE *PPU_BG_Base;
extern BYTE *PPU_SP_Base;
extern WORD PPU_SP_Height;
extern int  SpriteJustHit;
extern BYTE byVramWriteEnable;
extern BYTE PPU_Latch_Flag;
extern BYTE PPU_UpDown_Clip;
extern BYTE FrameIRQ_Enable;
extern WORD FrameStep;

extern WORD FrameSkip;
extern WORD FrameCnt;
extern BYTE ChrBufUpdate;
extern WORD PalTable[32];

extern BYTE APU_Reg[0x18];
extern DWORD PAD1_Latch;
extern DWORD PAD2_Latch;
extern DWORD PAD_System;
extern DWORD PAD1_Bit;
extern DWORD PAD2_Bit;

extern void InfoNES_Mirroring(int nType);

extern "C" {
  void Mapper_Save(void*& blob, size_t& size);
  int  Mapper_Load(const void* blob, size_t size);
  void pAPU_Save(void*& blob, size_t& size);
  int  pAPU_Load(const void* blob, size_t size);
}
__attribute__((weak)) void Mapper_Save(void*& blob, size_t& size){ blob=nullptr; size=0; }
__attribute__((weak)) int  Mapper_Load(const void* blob, size_t size){ (void)blob; (void)size; return 0; }
__attribute__((weak)) void pAPU_Save(void*& blob, size_t& size){ blob=nullptr; size=0; }
__attribute__((weak)) int  pAPU_Load(const void* blob, size_t size){ (void)blob; (void)size; return 0; }

struct SaveHeader {
  char     magic[8]; // "INFOST\1"
  uint32_t version;
  uint32_t mapperNo;
  uint32_t flags;    // bit0: CHR RAM present
};

struct SaveCore {
  // CPU
  WORD PC;
  BYTE SP;
  BYTE F;
  BYTE A;
  BYTE X;
  BYTE Y;
  BYTE IRQ_State;
  BYTE IRQ_Wiring;
  BYTE NMI_State;
  BYTE NMI_Wiring;
  int  g_wPassedClocks;
  int  g_wCurrentClocks;

  // PPU (no vertical scroll fields)
  BYTE PPU_R0, PPU_R1, PPU_R2, PPU_R3, PPU_R7;
  WORD PPU_Addr, PPU_Temp;
  WORD PPU_Increment;
  WORD PPU_Scanline;
  BYTE PPU_NameTableBank;
  WORD PPU_SP_Height;
  BYTE PPU_Scr_H_Byte;
  BYTE PPU_Scr_H_Bit;
  BYTE PPU_Latch_Flag;
  BYTE PPU_UpDown_Clip;
  BYTE FrameIRQ_Enable;
  WORD FrameStep;
  int  SpriteJustHit;

  // Timing / misc
  WORD FrameSkip;
  WORD FrameCnt;
  BYTE ChrBufUpdate;
  BYTE byVramWriteEnable;
  BYTE ROM_Mirroring;
  BYTE reserved0;

  // Palette
  WORD PalTable[32];

  // Bank indices
  BYTE     ppuBankIndex[16];
  uint16_t prgBankIndex[4];

  // APU
  BYTE APU_Reg[0x18];

  // Pads
  DWORD PAD1_Latch;
  DWORD PAD2_Latch;
  DWORD PAD_System;
  DWORD PAD1_Bit;
  DWORD PAD2_Bit;
};

static inline BYTE* chrBase()
{
  return (NesHeader.byVRomSize > 0) ? VROM : PPURAM;
}

static void fillBankIndices(SaveCore& c)
{
  BYTE* base = chrBase();
  for (int i=0;i<16;i++)
    c.ppuBankIndex[i] = (BYTE)((PPUBANK[i] - base) / 0x400);
  for (int i=0;i<4;i++)
    c.prgBankIndex[i] = (uint16_t)((ROMBANK[i] - ROM) / 0x2000);
}

static void restorePPUBanks(const SaveCore& c)
{
  BYTE* base = chrBase();
  for (int i=0;i<16;i++)
    PPUBANK[i] = base + c.ppuBankIndex[i] * 0x400;
}

static void restorePRGBanks(const SaveCore& c)
{
  for (int i=0;i<4;i++)
    ROMBANK[i] = ROM + c.prgBankIndex[i] * 0x2000;
}

static void recalcPatternBases()
{
  BYTE *base0 = ChrBuf;
  BYTE *base1 = ChrBuf + 256 * 64;
  PPU_BG_Base = (PPU_R0 & 0x10) ? base1 : base0;
  PPU_SP_Base = (PPU_R0 & 0x08) ? base1 : base0;
  ChrBufUpdate = 1; // trigger tile decode refresh
}

int InfoNES_SaveState(const char* path)
{
  SaveHeader hdr{};
  memcpy(hdr.magic,"INFOST\1",8);
  hdr.version  = 1;
  hdr.mapperNo = MapperNo;
  hdr.flags    = (NesHeader.byVRomSize == 0) ? 1u : 0u;

  SaveCore core{};
  core.PC=PC; core.SP=SP; core.F=F; core.A=A; core.X=X; core.Y=Y;
  core.IRQ_State=IRQ_State; core.IRQ_Wiring=IRQ_Wiring;
  core.NMI_State=NMI_State; core.NMI_Wiring=NMI_Wiring;
  core.g_wPassedClocks=g_wPassedClocks;
  core.g_wCurrentClocks=g_wCurrentClocks;

  core.PPU_R0=PPU_R0; core.PPU_R1=PPU_R1; core.PPU_R2=PPU_R2;
  core.PPU_R3=PPU_R3; core.PPU_R7=PPU_R7;
  core.PPU_Addr=PPU_Addr; core.PPU_Temp=PPU_Temp;
  core.PPU_Increment=PPU_Increment;
  core.PPU_Scanline=PPU_Scanline;
  core.PPU_NameTableBank=PPU_NameTableBank;
  core.PPU_SP_Height=PPU_SP_Height;
  core.PPU_Scr_H_Byte=PPU_Scr_H_Byte;
  core.PPU_Scr_H_Bit=PPU_Scr_H_Bit;
  core.PPU_Latch_Flag=PPU_Latch_Flag;
  core.PPU_UpDown_Clip=PPU_UpDown_Clip;
  core.FrameIRQ_Enable=FrameIRQ_Enable;
  core.FrameStep=FrameStep;
  core.SpriteJustHit=SpriteJustHit;

  core.FrameSkip=FrameSkip;
  core.FrameCnt=FrameCnt;
  core.ChrBufUpdate=ChrBufUpdate;
  core.byVramWriteEnable=byVramWriteEnable;
  core.ROM_Mirroring=ROM_Mirroring;
  memcpy(core.PalTable, PalTable, sizeof core.PalTable);
  memcpy(core.APU_Reg, APU_Reg, sizeof core.APU_Reg);

  core.PAD1_Latch=PAD1_Latch;
  core.PAD2_Latch=PAD2_Latch;
  core.PAD_System=PAD_System;
  core.PAD1_Bit=PAD1_Bit;
  core.PAD2_Bit=PAD2_Bit;

  fillBankIndices(core);

  void* mapperBlob=nullptr; size_t mapperSize=0;
  Mapper_Save(mapperBlob, mapperSize);
  void* apuBlob=nullptr; size_t apuSize=0;
  pAPU_Save(apuBlob, apuSize);

  FIL fp;
  if (f_open(&fp, path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
    return -1;

  auto w=[&](const void* buf, UINT len)->bool {
    UINT bw; return f_write(&fp, buf, len, &bw)==FR_OK && bw==len;
  };

  if (!w(&hdr,sizeof hdr) ||
      !w(&core,sizeof core) ||
      !w(RAM,RAM_SIZE) ||
      !w(SPRRAM,SPRRAM_SIZE) ||
      !w(SRAM,SRAM_SIZE))
  { f_close(&fp); return -1; }

  if (hdr.flags & 1u) // CHR RAM
  {
    if (!w(PPURAM,0x2000)) { f_close(&fp); return -1; }
  }

  if (!w(&mapperSize,sizeof mapperSize)) { f_close(&fp); return -1; }
  if (mapperSize && !w(mapperBlob,mapperSize)) { f_close(&fp); return -1; }
  if (!w(&apuSize,sizeof apuSize)) { f_close(&fp); return -1; }
  if (apuSize && !w(apuBlob,apuSize)) { f_close(&fp); return -1; }

  f_close(&fp);
  return 0;
}

int InfoNES_LoadState(const char* path)
{
  FIL fp;
  if (f_open(&fp, path, FA_READ) != FR_OK)
    return -1;

  auto r=[&](void* buf, UINT len)->bool {
    UINT br; return f_read(&fp, buf, len, &br)==FR_OK && br==len;
  };

  SaveHeader hdr{};
  if (!r(&hdr,sizeof hdr) ||
      memcmp(hdr.magic,"INFOST\1",8)!=0 ||
      hdr.version!=1 ||
      hdr.mapperNo!=MapperNo)
  { f_close(&fp); return -1; }

  SaveCore core{};
  if (!r(&core,sizeof core) ||
      !r(RAM,RAM_SIZE) ||
      !r(SPRRAM,SPRRAM_SIZE) ||
      !r(SRAM,SRAM_SIZE))
  { f_close(&fp); return -1; }

  if (hdr.flags & 1u)
  {
    if (!r(PPURAM,0x2000)) { f_close(&fp); return -1; }
  }

  size_t mapperSize=0;
  if (!r(&mapperSize,sizeof mapperSize)) { f_close(&fp); return -1; }
  std::unique_ptr<BYTE[]> mapperBuf;
  if (mapperSize)
  {
    mapperBuf.reset(new BYTE[mapperSize]);
    if (!r(mapperBuf.get(),mapperSize)) { f_close(&fp); return -1; }
  }

  size_t apuSize=0;
  if (!r(&apuSize,sizeof apuSize)) { f_close(&fp); return -1; }
  std::unique_ptr<BYTE[]> apuBuf;
  if (apuSize)
  {
    apuBuf.reset(new BYTE[apuSize]);
    if (!r(apuBuf.get(),apuSize)) { f_close(&fp); return -1; }
  }

  f_close(&fp);

  // CPU
  PC=core.PC; SP=core.SP; F=core.F; A=core.A; X=core.X; Y=core.Y;
  IRQ_State=core.IRQ_State; IRQ_Wiring=core.IRQ_Wiring;
  NMI_State=core.NMI_State; NMI_Wiring=core.NMI_Wiring;
  g_wPassedClocks=core.g_wPassedClocks;
  g_wCurrentClocks=core.g_wCurrentClocks;

  // PPU
  PPU_R0=core.PPU_R0; PPU_R1=core.PPU_R1; PPU_R2=core.PPU_R2;
  PPU_R3=core.PPU_R3; PPU_R7=core.PPU_R7;
  PPU_Addr=core.PPU_Addr; PPU_Temp=core.PPU_Temp;
  PPU_Increment=core.PPU_Increment;
  PPU_Scanline=core.PPU_Scanline;
  PPU_NameTableBank=core.PPU_NameTableBank;
  PPU_SP_Height=core.PPU_SP_Height;
  PPU_Scr_H_Byte=core.PPU_Scr_H_Byte;
  PPU_Scr_H_Bit=core.PPU_Scr_H_Bit;
  PPU_Latch_Flag=core.PPU_Latch_Flag;
  PPU_UpDown_Clip=core.PPU_UpDown_Clip;
  FrameIRQ_Enable=core.FrameIRQ_Enable;
  FrameStep=core.FrameStep;
  SpriteJustHit=core.SpriteJustHit;

  // Misc
  FrameSkip=core.FrameSkip;
  FrameCnt=core.FrameCnt;
  byVramWriteEnable=core.byVramWriteEnable;
  ROM_Mirroring=core.ROM_Mirroring;
  memcpy(PalTable, core.PalTable, sizeof PalTable);
  memcpy(APU_Reg, core.APU_Reg, sizeof APU_Reg);

  PAD1_Latch=core.PAD1_Latch;
  PAD2_Latch=core.PAD2_Latch;
  PAD_System=core.PAD_System;
  PAD1_Bit=core.PAD1_Bit;
  PAD2_Bit=core.PAD2_Bit;

  restorePPUBanks(core);
  restorePRGBanks(core);
  InfoNES_Mirroring(ROM_Mirroring);

  recalcPatternBases(); // ensure background refresh

  if (Mapper_Load(mapperBuf.get(), mapperSize) < 0) return -1;
  if (pAPU_Load(apuBuf.get(), apuSize) < 0) return -1;

  return 0;
}