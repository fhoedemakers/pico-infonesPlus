/*===================================================================*/
/*                                                                   */
/*  Mapper 5 (MMC5) - Reimplemented from scratch                     */
/*                                                                   */
/*  Reference: MMC5.pdf specification and murmnes mapper005.hpp      */
/*  Uses dynamic memory allocation via Frens::f_malloc / f_free      */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  MMC5 Dynamically Allocated Buffers                               */
/*-------------------------------------------------------------------*/

/* PRG-RAM: 64KB (8 banks of 8KB) */
#define MAP5_WRAM_SIZE (0x2000 * 8)
static BYTE *Map5_Wram;

/* ExRAM: 1KB internal extended RAM ($5C00-$5FFF) */
#define MAP5_EXRAM_SIZE 0x400
static BYTE *Map5_ExRam;

/* Fill-mode nametable: 1KB (used when nametable source = 3) */
#define MAP5_FILLNAM_SIZE 0x400
static BYTE *Map5_FillNam;

/*-------------------------------------------------------------------*/
/*  MMC5 Registers (static, small)                                   */
/*-------------------------------------------------------------------*/

/* PRG mode ($5100): 0=32K, 1=16K+16K, 2=16K+8K+8K, 3=8K×4 */
static BYTE Map5_PrgMode;

/* CHR mode ($5101): 0=8K, 1=4K, 2=2K, 3=1K */
static BYTE Map5_ChrMode;

/* WRAM write-protect ($5102, $5103) */
static BYTE Map5_WramProtect0;
static BYTE Map5_WramProtect1;

/* Extended RAM mode ($5104): 0-3 */
static BYTE Map5_GfxMode;

/* PRG bank registers ($5113-$5117) */
static BYTE Map5_PrgReg[8];

/* WRAM bank tracking (0xff = ROM mapped) */
static BYTE Map5_WramReg[8];

/* CHR bank registers: [bank][0=sprite, 1=BG] ($5120-$512B) */
static BYTE Map5_ChrReg[8][2];

/* Scanline IRQ */
static BYTE Map5_IrqEnable;  /* $5204 bit 7 */
static BYTE Map5_IrqTarget;  /* $5203: target scanline */
static BYTE Map5_IrqPending; /* IRQ pending flag */
static BYTE Map5_InFrame;    /* In-frame flag (scanline 0-239) */

/* 8-bit multiplier ($5205/$5206) */
static BYTE Map5_MultA;
static BYTE Map5_MultB;

/* The address of 8Kbytes unit of the Map5 WRAM */
#define Map5_WRAMPAGE(a) &Map5_Wram[((a) & 0x07) * 0x2000]

/*-------------------------------------------------------------------*/
/*  Mapper 5 Finalization - Free dynamic buffers                     */
/*-------------------------------------------------------------------*/
void Map5_Fin()
{
  if (Map5_Wram)
  {
    Frens::f_free(Map5_Wram);
    Map5_Wram = nullptr;
  }
  if (Map5_ExRam)
  {
    Frens::f_free(Map5_ExRam);
    Map5_ExRam = nullptr;
  }
  if (Map5_FillNam)
  {
    Frens::f_free(Map5_FillNam);
    Map5_FillNam = nullptr;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Sync Program Banks                                      */
/*-------------------------------------------------------------------*/
void Map5_SyncPrgBanks(void)
{
  switch (Map5_PrgMode)
  {
  case 0: /* 32KB mode: $5117 selects 32KB bank */
    Map5_WramReg[4] = 0xff;
    Map5_WramReg[5] = 0xff;
    Map5_WramReg[6] = 0xff;
    ROMBANK0 = ROMPAGE(((Map5_PrgReg[7] & 0x7c) + 0) % (NesHeader.byRomSize << 1));
    ROMBANK1 = ROMPAGE(((Map5_PrgReg[7] & 0x7c) + 1) % (NesHeader.byRomSize << 1));
    ROMBANK2 = ROMPAGE(((Map5_PrgReg[7] & 0x7c) + 2) % (NesHeader.byRomSize << 1));
    ROMBANK3 = ROMPAGE(((Map5_PrgReg[7] & 0x7c) + 3) % (NesHeader.byRomSize << 1));
    break;

  case 1: /* 16KB+16KB mode */
    /* $8000-$BFFF: $5115 (bit 7 selects ROM vs WRAM) */
    if (Map5_PrgReg[5] & 0x80)
    {
      Map5_WramReg[4] = 0xff;
      Map5_WramReg[5] = 0xff;
      ROMBANK0 = ROMPAGE(((Map5_PrgReg[5] & 0x7e) + 0) % (NesHeader.byRomSize << 1));
      ROMBANK1 = ROMPAGE(((Map5_PrgReg[5] & 0x7e) + 1) % (NesHeader.byRomSize << 1));
    }
    else
    {
      Map5_WramReg[4] = (Map5_PrgReg[5] & 0x06) + 0;
      Map5_WramReg[5] = (Map5_PrgReg[5] & 0x06) + 1;
      ROMBANK0 = Map5_WRAMPAGE(Map5_WramReg[4]);
      ROMBANK1 = Map5_WRAMPAGE(Map5_WramReg[5]);
    }
    /* $C000-$FFFF: $5117 always ROM */
    Map5_WramReg[6] = 0xff;
    ROMBANK2 = ROMPAGE(((Map5_PrgReg[7] & 0x7e) + 0) % (NesHeader.byRomSize << 1));
    ROMBANK3 = ROMPAGE(((Map5_PrgReg[7] & 0x7e) + 1) % (NesHeader.byRomSize << 1));
    break;

  case 2: /* 16KB+8KB+8KB mode */
    /* $8000-$BFFF: $5115 */
    if (Map5_PrgReg[5] & 0x80)
    {
      Map5_WramReg[4] = 0xff;
      Map5_WramReg[5] = 0xff;
      ROMBANK0 = ROMPAGE(((Map5_PrgReg[5] & 0x7e) + 0) % (NesHeader.byRomSize << 1));
      ROMBANK1 = ROMPAGE(((Map5_PrgReg[5] & 0x7e) + 1) % (NesHeader.byRomSize << 1));
    }
    else
    {
      Map5_WramReg[4] = (Map5_PrgReg[5] & 0x06) + 0;
      Map5_WramReg[5] = (Map5_PrgReg[5] & 0x06) + 1;
      ROMBANK0 = Map5_WRAMPAGE(Map5_WramReg[4]);
      ROMBANK1 = Map5_WRAMPAGE(Map5_WramReg[5]);
    }
    /* $C000-$DFFF: $5116 */
    if (Map5_PrgReg[6] & 0x80)
    {
      Map5_WramReg[6] = 0xff;
      ROMBANK2 = ROMPAGE((Map5_PrgReg[6] & 0x7f) % (NesHeader.byRomSize << 1));
    }
    else
    {
      Map5_WramReg[6] = Map5_PrgReg[6] & 0x07;
      ROMBANK2 = Map5_WRAMPAGE(Map5_WramReg[6]);
    }
    /* $E000-$FFFF: $5117 always ROM */
    ROMBANK3 = ROMPAGE((Map5_PrgReg[7] & 0x7f) % (NesHeader.byRomSize << 1));
    break;

  default: /* Mode 3: 8KB×4 */
    /* $8000-$9FFF: $5114 */
    if (Map5_PrgReg[4] & 0x80)
    {
      Map5_WramReg[4] = 0xff;
      ROMBANK0 = ROMPAGE((Map5_PrgReg[4] & 0x7f) % (NesHeader.byRomSize << 1));
    }
    else
    {
      Map5_WramReg[4] = Map5_PrgReg[4] & 0x07;
      ROMBANK0 = Map5_WRAMPAGE(Map5_WramReg[4]);
    }
    /* $A000-$BFFF: $5115 */
    if (Map5_PrgReg[5] & 0x80)
    {
      Map5_WramReg[5] = 0xff;
      ROMBANK1 = ROMPAGE((Map5_PrgReg[5] & 0x7f) % (NesHeader.byRomSize << 1));
    }
    else
    {
      Map5_WramReg[5] = Map5_PrgReg[5] & 0x07;
      ROMBANK1 = Map5_WRAMPAGE(Map5_WramReg[5]);
    }
    /* $C000-$DFFF: $5116 */
    if (Map5_PrgReg[6] & 0x80)
    {
      Map5_WramReg[6] = 0xff;
      ROMBANK2 = ROMPAGE((Map5_PrgReg[6] & 0x7f) % (NesHeader.byRomSize << 1));
    }
    else
    {
      Map5_WramReg[6] = Map5_PrgReg[6] & 0x07;
      ROMBANK2 = Map5_WRAMPAGE(Map5_WramReg[6]);
    }
    /* $E000-$FFFF: $5117 always ROM */
    ROMBANK3 = ROMPAGE((Map5_PrgReg[7] & 0x7f) % (NesHeader.byRomSize << 1));
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 5                                              */
/*-------------------------------------------------------------------*/
void Map5_Init()
{
  int nPage;

  /* Allocate dynamic buffers */
  Map5_Wram = (BYTE *)Frens::f_malloc(MAP5_WRAM_SIZE);
  Map5_ExRam = (BYTE *)Frens::f_malloc(MAP5_EXRAM_SIZE);
  Map5_FillNam = (BYTE *)Frens::f_malloc(MAP5_FILLNAM_SIZE);

  InfoNES_MemorySet(Map5_Wram, 0x00, MAP5_WRAM_SIZE);
  InfoNES_MemorySet(Map5_ExRam, 0x00, MAP5_EXRAM_SIZE);
  InfoNES_MemorySet(Map5_FillNam, 0x00, MAP5_FILLNAM_SIZE);

  /* Set Mapper Callbacks */
  MapperInit = Map5_Init;
  MapperWrite = Map5_Write;
  MapperSram = Map5_Sram;
  MapperApu = Map5_Apu;
  MapperReadApu = Map5_ReadApu;
  MapperVSync = Map0_VSync;
  MapperHSync = Map5_HSync;
  MapperPPU = Map0_PPU;
  MapperRenderScreen = Map5_RenderScreen;
  MapperFin = Map5_Fin;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Set ROM Banks - all to last page initially */
  ROMBANK0 = ROMLASTPAGE(0);
  ROMBANK1 = ROMLASTPAGE(0);
  ROMBANK2 = ROMLASTPAGE(0);
  ROMBANK3 = ROMLASTPAGE(0);

  /* Set PPU Banks */
  for (nPage = 0; nPage < 8; ++nPage)
    PPUBANK[nPage] = VROMPAGE(nPage);
  InfoNES_SetupChr();

  /* Initialize PRG registers */
  for (nPage = 4; nPage < 8; ++nPage)
  {
    Map5_PrgReg[nPage] = (NesHeader.byRomSize << 1) - 1;
    Map5_WramReg[nPage] = 0xff;
  }
  Map5_WramReg[3] = 0xff;

  /* Initialize CHR registers */
  for (BYTE byPage = 4; byPage < 8; ++byPage)
  {
    Map5_ChrReg[byPage][0] = byPage;
    Map5_ChrReg[byPage][1] = (byPage & 0x03) + 4;
  }

  /* Initialize mode registers */
  Map5_PrgMode = 3;
  Map5_ChrMode = 3;
  Map5_WramProtect0 = 0;
  Map5_WramProtect1 = 0;
  Map5_GfxMode = 0;

  /* Initialize IRQ */
  Map5_IrqEnable = 0;
  Map5_IrqTarget = 0;
  Map5_IrqPending = 0;
  Map5_InFrame = 0;

  /* Initialize multiplier */
  Map5_MultA = 0;
  Map5_MultB = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring(1, 1);
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Read from APU ($5000-$5FFF)                             */
/*-------------------------------------------------------------------*/
BYTE Map5_ReadApu(WORD wAddr)
{
  BYTE byRet = (BYTE)(wAddr >> 8);

  if (wAddr == 0x5204)
  {
    /* $5204 read: bit 7 = IRQ pending, bit 6 = in-frame */
    byRet = 0;
    if (Map5_InFrame)
      byRet |= 0x40;
    if (Map5_IrqPending)
      byRet |= 0x80;
    /* Reading $5204 acknowledges (clears) pending IRQ */
    Map5_IrqPending = 0;
  }
  else if (wAddr == 0x5205)
  {
    byRet = (BYTE)((Map5_MultA * Map5_MultB) & 0xFF);
  }
  else if (wAddr == 0x5206)
  {
    byRet = (BYTE)(((Map5_MultA * Map5_MultB) >> 8) & 0xFF);
  }
  else if (wAddr >= 0x5C00 && wAddr <= 0x5FFF)
  {
    /* ExRAM is readable */
    byRet = Map5_ExRam[wAddr - 0x5C00];
  }

  return byRet;
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Write to APU ($5000-$5FFF)                              */
/*-------------------------------------------------------------------*/
void Map5_Apu(WORD wAddr, BYTE byData)
{
  int nPage;

  switch (wAddr)
  {
  case 0x5100:
    Map5_PrgMode = byData & 0x03;
    Map5_SyncPrgBanks();
    break;

  case 0x5101:
    Map5_ChrMode = byData & 0x03;
    break;

  case 0x5102:
    Map5_WramProtect0 = byData & 0x03;
    break;

  case 0x5103:
    Map5_WramProtect1 = byData & 0x03;
    break;

  case 0x5104:
    Map5_GfxMode = byData & 0x03;
    break;

  case 0x5105:
    /* Nametable mapping: each 2-bit field selects source */
    for (nPage = 0; nPage < 4; nPage++)
    {
      BYTE byNamSrc = byData & 0x03;
      byData >>= 2;

      switch (byNamSrc)
      {
      case 0: /* CIRAM page 0 */
        PPUBANK[nPage + 8] = VRAMPAGE(0);
        break;
      case 1: /* CIRAM page 1 */
        PPUBANK[nPage + 8] = VRAMPAGE(1);
        break;
      case 2: /* ExRAM */
        PPUBANK[nPage + 8] = Map5_ExRam;
        break;
      case 3: /* Fill-mode */
        PPUBANK[nPage + 8] = Map5_FillNam;
        break;
      }
    }
    break;

  case 0x5106:
    /* Fill tile: fills the nametable portion (960 bytes) */
    InfoNES_MemorySet(Map5_FillNam, byData, 0x3C0);
    break;

  case 0x5107:
  {
    /* Fill attribute: expand 2-bit value to fill attribute table */
    BYTE byAttr = byData & 0x03;
    byAttr = byAttr | (byAttr << 2) | (byAttr << 4) | (byAttr << 6);
    InfoNES_MemorySet(&Map5_FillNam[0x3C0], byAttr, 0x400 - 0x3C0);
    break;
  }

  case 0x5113:
    Map5_WramReg[3] = byData & 0x07;
    SRAMBANK = Map5_WRAMPAGE(byData & 0x07);
    break;

  case 0x5114:
  case 0x5115:
  case 0x5116:
  case 0x5117:
    Map5_PrgReg[wAddr & 0x07] = byData;
    Map5_SyncPrgBanks();
    break;

  /* Sprite CHR bank registers ($5120-$5127) */
  case 0x5120:
  case 0x5121:
  case 0x5122:
  case 0x5123:
  case 0x5124:
  case 0x5125:
  case 0x5126:
  case 0x5127:
    Map5_ChrReg[wAddr & 0x07][0] = byData;
    break;

  /* BG CHR bank registers ($5128-$512B) */
  case 0x5128:
  case 0x5129:
  case 0x512A:
  case 0x512B:
    Map5_ChrReg[(wAddr & 0x03) + 0][1] = byData;
    Map5_ChrReg[(wAddr & 0x03) + 4][1] = byData;
    break;

  case 0x5200:
  case 0x5201:
  case 0x5202:
    /* Vertical split mode - not implemented */
    break;

  case 0x5203:
    Map5_IrqTarget = byData;
    break;

  case 0x5204:
    Map5_IrqEnable = byData & 0x80;
    /* Writing with bit 7 clear clears pending IRQ */
    if (!(byData & 0x80))
      Map5_IrqPending = 0;
    break;

  case 0x5205:
    Map5_MultA = byData;
    break;

  case 0x5206:
    Map5_MultB = byData;
    break;

  default:
    if (wAddr >= 0x5000 && wAddr <= 0x5015)
    {
      /* MMC5 expansion audio - not implemented */
    }
    else if (wAddr >= 0x5C00 && wAddr <= 0x5FFF)
    {
      /* ExRAM writes depend on graphics mode */
      switch (Map5_GfxMode)
      {
      case 0: /* Mode 0: ExRAM as extra nametable */
      case 1: /* Mode 1: ExRAM as extended attributes */
        Map5_ExRam[wAddr - 0x5C00] = byData;
        break;
      case 2: /* Mode 2: ExRAM as general-purpose RAM */
        Map5_ExRam[wAddr - 0x5C00] = byData;
        break;
      default: /* Mode 3: ExRAM is read-only */
        break;
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
  if (Map5_WramProtect0 == 0x02 && Map5_WramProtect1 == 0x01)
  {
    if (Map5_WramReg[3] != 0xff)
    {
      Map5_Wram[0x2000 * Map5_WramReg[3] + (wAddr - 0x6000)] = byData;
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Write to ROM area ($8000-$FFFF)                         */
/*  (Only hits when WRAM is mapped into ROM address space)           */
/*-------------------------------------------------------------------*/
void Map5_Write(WORD wAddr, BYTE byData)
{
  if (Map5_WramProtect0 == 0x02 && Map5_WramProtect1 == 0x01)
  {
    switch (wAddr & 0xE000)
    {
    case 0x8000: /* $8000-$9FFF */
      if (Map5_WramReg[4] != 0xff)
        Map5_Wram[0x2000 * Map5_WramReg[4] + (wAddr - 0x8000)] = byData;
      break;

    case 0xA000: /* $A000-$BFFF */
      if (Map5_WramReg[5] != 0xff)
        Map5_Wram[0x2000 * Map5_WramReg[5] + (wAddr - 0xA000)] = byData;
      break;

    case 0xC000: /* $C000-$DFFF */
      if (Map5_WramReg[6] != 0xff)
        Map5_Wram[0x2000 * Map5_WramReg[6] + (wAddr - 0xC000)] = byData;
      break;
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 H-Sync Function (Scanline IRQ)                          */
/*                                                                   */
/*  Uses direct PPU_Scanline comparison. IRQ is asserted every       */
/*  HSync while pending+enabled (persistent assertion pattern).      */
/*-------------------------------------------------------------------*/
void Map5_HSync()
{
  if (PPU_Scanline <= 239)
  {
    /* Visible scanlines: set in-frame flag */
    Map5_InFrame = 1;

    /* Compare current scanline against IRQ target */
    if (PPU_Scanline == Map5_IrqTarget)
    {
      Map5_IrqPending = 1;
    }

    /* Assert IRQ every HSync while pending and enabled */
    if (Map5_IrqPending && Map5_IrqEnable)
    {
      IRQ_REQ;
    }
  }
  else
  {
    /* Outside visible area: clear in-frame flag */
    Map5_InFrame = 0;
    /* Clear pending IRQ at VBlank */
    if (PPU_Scanline == SCAN_VBLANK_START)
    {
      Map5_IrqPending = 0;
    }
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Rendering Screen Function                               */
/*                                                                   */
/*  Switches CHR banks between sprite (byMode=0) and BG (byMode=1)  */
/*  based on the current CHR mode ($5101).                           */
/*-------------------------------------------------------------------*/
void Map5_RenderScreen(BYTE byMode)
{
  DWORD dwPage[8];
  DWORD dwVRomMask = (NesHeader.byVRomSize << 3);

  if (dwVRomMask == 0)
    return;

  switch (Map5_ChrMode)
  {
  case 0: /* 8KB mode */
    dwPage[7] = ((DWORD)Map5_ChrReg[7][byMode] << 3) % dwVRomMask;
    PPUBANK[0] = VROMPAGE(dwPage[7] + 0);
    PPUBANK[1] = VROMPAGE(dwPage[7] + 1);
    PPUBANK[2] = VROMPAGE(dwPage[7] + 2);
    PPUBANK[3] = VROMPAGE(dwPage[7] + 3);
    PPUBANK[4] = VROMPAGE(dwPage[7] + 4);
    PPUBANK[5] = VROMPAGE(dwPage[7] + 5);
    PPUBANK[6] = VROMPAGE(dwPage[7] + 6);
    PPUBANK[7] = VROMPAGE(dwPage[7] + 7);
    InfoNES_SetupChr();
    break;

  case 1: /* 4KB mode */
    dwPage[3] = ((DWORD)Map5_ChrReg[3][byMode] << 2) % dwVRomMask;
    dwPage[7] = ((DWORD)Map5_ChrReg[7][byMode] << 2) % dwVRomMask;
    PPUBANK[0] = VROMPAGE(dwPage[3] + 0);
    PPUBANK[1] = VROMPAGE(dwPage[3] + 1);
    PPUBANK[2] = VROMPAGE(dwPage[3] + 2);
    PPUBANK[3] = VROMPAGE(dwPage[3] + 3);
    PPUBANK[4] = VROMPAGE(dwPage[7] + 0);
    PPUBANK[5] = VROMPAGE(dwPage[7] + 1);
    PPUBANK[6] = VROMPAGE(dwPage[7] + 2);
    PPUBANK[7] = VROMPAGE(dwPage[7] + 3);
    InfoNES_SetupChr();
    break;

  case 2: /* 2KB mode */
    dwPage[1] = ((DWORD)Map5_ChrReg[1][byMode] << 1) % dwVRomMask;
    dwPage[3] = ((DWORD)Map5_ChrReg[3][byMode] << 1) % dwVRomMask;
    dwPage[5] = ((DWORD)Map5_ChrReg[5][byMode] << 1) % dwVRomMask;
    dwPage[7] = ((DWORD)Map5_ChrReg[7][byMode] << 1) % dwVRomMask;
    PPUBANK[0] = VROMPAGE(dwPage[1] + 0);
    PPUBANK[1] = VROMPAGE(dwPage[1] + 1);
    PPUBANK[2] = VROMPAGE(dwPage[3] + 0);
    PPUBANK[3] = VROMPAGE(dwPage[3] + 1);
    PPUBANK[4] = VROMPAGE(dwPage[5] + 0);
    PPUBANK[5] = VROMPAGE(dwPage[5] + 1);
    PPUBANK[6] = VROMPAGE(dwPage[7] + 0);
    PPUBANK[7] = VROMPAGE(dwPage[7] + 1);
    InfoNES_SetupChr();
    break;

  default: /* 1KB mode */
    dwPage[0] = (DWORD)Map5_ChrReg[0][byMode] % dwVRomMask;
    dwPage[1] = (DWORD)Map5_ChrReg[1][byMode] % dwVRomMask;
    dwPage[2] = (DWORD)Map5_ChrReg[2][byMode] % dwVRomMask;
    dwPage[3] = (DWORD)Map5_ChrReg[3][byMode] % dwVRomMask;
    dwPage[4] = (DWORD)Map5_ChrReg[4][byMode] % dwVRomMask;
    dwPage[5] = (DWORD)Map5_ChrReg[5][byMode] % dwVRomMask;
    dwPage[6] = (DWORD)Map5_ChrReg[6][byMode] % dwVRomMask;
    dwPage[7] = (DWORD)Map5_ChrReg[7][byMode] % dwVRomMask;
    PPUBANK[0] = VROMPAGE(dwPage[0]);
    PPUBANK[1] = VROMPAGE(dwPage[1]);
    PPUBANK[2] = VROMPAGE(dwPage[2]);
    PPUBANK[3] = VROMPAGE(dwPage[3]);
    PPUBANK[4] = VROMPAGE(dwPage[4]);
    PPUBANK[5] = VROMPAGE(dwPage[5]);
    PPUBANK[6] = VROMPAGE(dwPage[6]);
    PPUBANK[7] = VROMPAGE(dwPage[7]);
    InfoNES_SetupChr();
    break;
  }
}
