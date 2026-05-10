/*===================================================================*/
/*                                                                   */
/*  Mapper 210 (NAMCOT-175 / NAMCOT-340)                             */
/*                                                                   */
/*  Simplified Namco 163 variant without IRQ or expansion sound.     */
/*  NAMCOT-175: hardwired mirroring, battery SRAM (Chibi Maruko)     */
/*  NAMCOT-340: nametable banking via $C000-$DFFF (Splatterhouse)    */
/*                                                                   */
/*===================================================================*/

BYTE Map210_Is175;

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 210                                            */
/*-------------------------------------------------------------------*/
void Map210_Init()
{
  MapperInit = Map210_Init;
  MapperWrite = Map210_Write;
  MapperSram = Map0_Sram;
  MapperApu = Map0_Apu;
  MapperReadApu = Map0_ReadApu;
  MapperVSync = Map0_VSync;
  MapperHSync = Map0_HSync;
  MapperPPU = Map0_PPU;
  MapperRenderScreen = Map0_RenderScreen;

  /* NAMCOT-175 has battery SRAM; NAMCOT-340 does not */
  Map210_Is175 = ROM_SRAM ? 1 : 0;

  SRAMBANK = SRAM;

  ROMBANK0 = ROMPAGE(0);
  ROMBANK1 = ROMPAGE(1);
  ROMBANK2 = ROMLASTPAGE(1);
  ROMBANK3 = ROMLASTPAGE(0);

  if (NesHeader.byVRomSize > 0)
  {
    for (int nPage = 0; nPage < 8; ++nPage)
      PPUBANK[nPage] = VROMPAGE(nPage);
    InfoNES_SetupChr();
  }

  K6502_Set_Int_Wiring(1, 1);
}

/*-------------------------------------------------------------------*/
/*  Mapper 210 Write Function                                        */
/*-------------------------------------------------------------------*/
void Map210_Write(WORD wAddr, BYTE byData)
{
  switch (wAddr & 0xf800)
  {
  case 0x8000:
    byData %= (NesHeader.byVRomSize << 3);
    PPUBANK[0] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0x8800:
    byData %= (NesHeader.byVRomSize << 3);
    PPUBANK[1] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0x9000:
    byData %= (NesHeader.byVRomSize << 3);
    PPUBANK[2] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0x9800:
    byData %= (NesHeader.byVRomSize << 3);
    PPUBANK[3] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xa000:
    byData %= (NesHeader.byVRomSize << 3);
    PPUBANK[4] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xa800:
    byData %= (NesHeader.byVRomSize << 3);
    PPUBANK[5] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xb000:
    byData %= (NesHeader.byVRomSize << 3);
    PPUBANK[6] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xb800:
    byData %= (NesHeader.byVRomSize << 3);
    PPUBANK[7] = VROMPAGE(byData);
    InfoNES_SetupChr();
    break;

  case 0xc000:
    if (!Map210_Is175)
    {
      if (byData < 0xe0)
      {
        byData %= (NesHeader.byVRomSize << 3);
        PPUBANK[NAME_TABLE0] = VROMPAGE(byData);
      }
      else
      {
        PPUBANK[NAME_TABLE0] = VRAMPAGE(byData & 0x01);
      }
    }
    break;

  case 0xc800:
    if (!Map210_Is175)
    {
      if (byData < 0xe0)
      {
        byData %= (NesHeader.byVRomSize << 3);
        PPUBANK[NAME_TABLE1] = VROMPAGE(byData);
      }
      else
      {
        PPUBANK[NAME_TABLE1] = VRAMPAGE(byData & 0x01);
      }
    }
    break;

  case 0xd000:
    if (!Map210_Is175)
    {
      if (byData < 0xe0)
      {
        byData %= (NesHeader.byVRomSize << 3);
        PPUBANK[NAME_TABLE2] = VROMPAGE(byData);
      }
      else
      {
        PPUBANK[NAME_TABLE2] = VRAMPAGE(byData & 0x01);
      }
    }
    break;

  case 0xd800:
    if (!Map210_Is175)
    {
      if (byData < 0xe0)
      {
        byData %= (NesHeader.byVRomSize << 3);
        PPUBANK[NAME_TABLE3] = VROMPAGE(byData);
      }
      else
      {
        PPUBANK[NAME_TABLE3] = VRAMPAGE(byData & 0x01);
      }
    }
    break;

  case 0xe000:
    byData &= 0x3f;
    byData %= (NesHeader.byRomSize << 1);
    ROMBANK0 = ROMPAGE(byData);
    break;

  case 0xe800:
    byData &= 0x3f;
    byData %= (NesHeader.byRomSize << 1);
    ROMBANK1 = ROMPAGE(byData);
    break;

  case 0xf000:
    byData &= 0x3f;
    byData %= (NesHeader.byRomSize << 1);
    ROMBANK2 = ROMPAGE(byData);
    break;
  }
}
