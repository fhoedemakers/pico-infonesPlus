/*===================================================================*/
/*                                                                   */
/*                    Mapper 85 (Konami VRC7)                        */
/*                                                                   */
/*===================================================================*/

/* CHR RAM (256KB) for the rare CHR-RAM-only VRC7 carts (e.g. Lagrange
 * Point). Allocated lazily from PSRAM in Map85_Init when the iNES
 * header reports zero CHR ROM; freed in InfoNES_Fin. */
BYTE *Map85_Chr_Ram;

/* Register state. Held in shadow regs and applied via Map85_Sync so the
 * ROM-vs-RAM CHR routing only needs to live in one place. */
BYTE Map85_PrgReg[3];   /* PRG select 0..2 ($8000, $A000, $C000 banks) */
BYTE Map85_ChrReg[8];   /* CHR select 0..7 (1KB units) */

/* VRC7 IRQ */
BYTE Map85_IRQ_Latch;
BYTE Map85_IRQ_Cnt;
BYTE Map85_IRQ_Enable;
BYTE Map85_IRQ_AckEnable;  /* "M" bit: E is restored to this on $F010 ack */
BYTE Map85_IRQ_Mode;       /* 0 = scanline, 1 = CPU cycle (cycle mode unsupported) */

#define Map85_VROMPAGE(a) &Map85_Chr_Ram[(a) * 0x400]
#define Map85_CHRPAGE(a) (NesHeader.byVRomSize > 0 \
  ? VROMPAGE((a) % (NesHeader.byVRomSize << 3)) \
  : Map85_VROMPAGE((a) & 0xff))

/*-------------------------------------------------------------------*/
/*  Apply current register state to PRG/CHR banks                    */
/*-------------------------------------------------------------------*/
static void Map85_Sync()
{
  const BYTE prgBanks = NesHeader.byRomSize << 1;  /* 8KB units */
  ROMBANK0 = ROMPAGE(Map85_PrgReg[0] % prgBanks);
  ROMBANK1 = ROMPAGE(Map85_PrgReg[1] % prgBanks);
  ROMBANK2 = ROMPAGE(Map85_PrgReg[2] % prgBanks);
  ROMBANK3 = ROMLASTPAGE(0);

  for (int i = 0; i < 8; ++i)
    PPUBANK[i] = Map85_CHRPAGE(Map85_ChrReg[i]);
  InfoNES_SetupChr();
}

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 85                                             */
/*-------------------------------------------------------------------*/
void Map85_Init()
{
  MapperInit         = Map85_Init;
  MapperWrite        = Map85_Write;
  MapperSram         = Map0_Sram;
  MapperApu          = Map0_Apu;
  MapperReadApu      = Map0_ReadApu;
  MapperVSync        = Map0_VSync;
  MapperHSync        = Map85_HSync;
  MapperPPU          = Map0_PPU;
  MapperRenderScreen = Map0_RenderScreen;

  /* Allocate CHR RAM only for CHR-RAM-only carts. */
  if (NesHeader.byVRomSize == 0 && !Map85_Chr_Ram)
  {
    Map85_Chr_Ram = (BYTE *)Frens::f_malloc(0x100 * 0x400);
    InfoNES_MemorySet(Map85_Chr_Ram, 0, 0x100 * 0x400);
  }

  SRAMBANK = SRAM;

  /* Initial PRG layout: bank 0, bank 1, last-but-one, last (last is fixed). */
  Map85_PrgReg[0] = 0;
  Map85_PrgReg[1] = 1;
  Map85_PrgReg[2] = (NesHeader.byRomSize << 1) - 2;
  for (int i = 0; i < 8; ++i)
    Map85_ChrReg[i] = i;

  Map85_IRQ_Latch     = 0;
  Map85_IRQ_Cnt       = 0;
  Map85_IRQ_Enable    = 0;
  Map85_IRQ_AckEnable = 0;
  Map85_IRQ_Mode      = 0;

  Map85_Sync();

  K6502_Set_Int_Wiring(1, 1);
}

/*-------------------------------------------------------------------*/
/*  Mapper 85 Write Function                                         */
/*                                                                    */
/*  VRC7 register decoding: bits A12-A15 select the major group, and  */
/*  bit A3 (053988 pinout, e.g. Tiny Toon Adventures 2) OR bit A4     */
/*  (053982, Lagrange Point) selects the second register in each      */
/*  pair. Audio writes always live at $9010 (reg-select) and $9030    */
/*  (data) regardless of variant.                                     */
/*-------------------------------------------------------------------*/
void Map85_Write(WORD wAddr, BYTE byData)
{
  /* Audio register window: $9010-$901F = reg select, $9030-$903F = data.
   * Detect first so the bit-3/bit-4 collapse below doesn't catch them. */
  if ((wAddr & 0xF030) == 0x9010 || (wAddr & 0xF030) == 0x9030)
  {
    /* VRC7 expansion audio (YM2413/OPLL FM synth) is not emulated. */
    return;
  }

  const WORD group = wAddr & 0xF000;
  const BYTE pair  = (wAddr & 0x0018) ? 1 : 0;  /* second reg if A3 or A4 set */

  switch (group)
  {
  case 0x8000:
    Map85_PrgReg[pair] = byData;
    Map85_Sync();
    break;

  case 0x9000:
    if (pair == 0)
    {
      Map85_PrgReg[2] = byData;
      Map85_Sync();
    }
    /* pair==1 here means an audio mirror that didn't match $9010/$9030
     * exactly — safe to ignore. */
    break;

  case 0xA000:
    Map85_ChrReg[0 + pair] = byData;
    Map85_Sync();
    break;

  case 0xB000:
    Map85_ChrReg[2 + pair] = byData;
    Map85_Sync();
    break;

  case 0xC000:
    Map85_ChrReg[4 + pair] = byData;
    Map85_Sync();
    break;

  case 0xD000:
    Map85_ChrReg[6 + pair] = byData;
    Map85_Sync();
    break;

  case 0xE000:
    if (pair == 0)
    {
      switch (byData & 0x03)
      {
      case 0: InfoNES_Mirroring(1); break;  /* vertical */
      case 1: InfoNES_Mirroring(0); break;  /* horizontal */
      case 2: InfoNES_Mirroring(3); break;  /* one-screen low */
      case 3: InfoNES_Mirroring(2); break;  /* one-screen high */
      }
    }
    else
    {
      Map85_IRQ_Latch = byData;
    }
    break;

  case 0xF000:
    if (pair == 0)
    {
      /* IRQ Control */
      Map85_IRQ_AckEnable = byData & 0x01;
      Map85_IRQ_Enable    = (byData & 0x02) >> 1;
      Map85_IRQ_Mode      = (byData & 0x04) >> 2;
      if (Map85_IRQ_Enable)
        Map85_IRQ_Cnt = Map85_IRQ_Latch;
    }
    else
    {
      /* IRQ Acknowledge: clear pending, restore E from M */
      Map85_IRQ_Enable = Map85_IRQ_AckEnable;
    }
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 85 H-Sync Function                                        */
/*                                                                    */
/*  Scanline-mode IRQ: counter increments each scanline; on overflow */
/*  (0xFF -> 0x00) IRQ is asserted and counter reloads from latch.   */
/*  Cycle mode is not supported.                                     */
/*-------------------------------------------------------------------*/
void Map85_HSync()
{
  if (Map85_IRQ_Enable && Map85_IRQ_Mode == 0)
  {
    if (Map85_IRQ_Cnt == 0xFF)
    {
      Map85_IRQ_Cnt = Map85_IRQ_Latch;
      IRQ_REQ;
    }
    else
    {
      Map85_IRQ_Cnt++;
    }
  }
}
