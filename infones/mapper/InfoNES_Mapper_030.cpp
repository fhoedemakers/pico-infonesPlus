/*===================================================================*/
/*                                                                   */
/*               Mapper 30 (UNROM 512 / NESmaker)                    */
/*                                                                   */
/*===================================================================*/

// Mirroring mode derived from iNES header flags
// byInfo1 bit 0 = mirroring, bit 3 = four-screen
// 0x00 -> fixed horizontal
// 0x01 -> fixed vertical
// 0x08 -> 1-screen, mapper-controlled via bit 7
// 0x09 -> four-screen (not supported here, treat as 1-screen)
static BYTE Map30_MirrorMode;

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 30                                             */
/*-------------------------------------------------------------------*/
void Map30_Init()
{
  /* Initialize Mapper */
  MapperInit = Map30_Init;

  /* Write to Mapper */
  MapperWrite = Map30_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map0_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Determine mirroring mode from header */
  Map30_MirrorMode = NesHeader.byInfo1 & 0x09;

  /* Set ROM Banks: first 16KB switchable, last 16KB fixed */
  ROMBANK0 = ROMPAGE( 0 );
  ROMBANK1 = ROMPAGE( 1 );
  ROMBANK2 = ROMLASTPAGE( 1 );
  ROMBANK3 = ROMLASTPAGE( 0 );

  /* Set CHR-RAM Banks (8KB = 8 x 1KB pages) */
  PPUBANK[ 0 ] = CRAMPAGE( 0 );
  PPUBANK[ 1 ] = CRAMPAGE( 1 );
  PPUBANK[ 2 ] = CRAMPAGE( 2 );
  PPUBANK[ 3 ] = CRAMPAGE( 3 );
  PPUBANK[ 4 ] = CRAMPAGE( 4 );
  PPUBANK[ 5 ] = CRAMPAGE( 5 );
  PPUBANK[ 6 ] = CRAMPAGE( 6 );
  PPUBANK[ 7 ] = CRAMPAGE( 7 );
  InfoNES_SetupChr();

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 );
}

/*-------------------------------------------------------------------*/
/*  Mapper 30 Write Function                                         */
/*-------------------------------------------------------------------*/
void Map30_Write( WORD wAddr, BYTE byData )
{
  /*
   * Register format: [MCCP PPPP]
   *  P (bits 0-4): PRG ROM bank select (16KB at $8000)
   *  C (bits 5-6): CHR RAM bank select (8KB at PPU $0000)
   *  M (bit 7):    Mirroring select (1-screen mode only)
   */

  /* Set PRG ROM bank at $8000-$BFFF */
  BYTE byPrg = ( byData & 0x1F ) % NesHeader.byRomSize;
  byPrg <<= 1;
  ROMBANK0 = ROMPAGE( byPrg );
  ROMBANK1 = ROMPAGE( byPrg + 1 );

  /* Set CHR RAM bank at PPU $0000-$1FFF */
  BYTE byChr = ( ( byData >> 5 ) & 0x03 ) << 3;
  PPUBANK[ 0 ] = CRAMPAGE( byChr + 0 );
  PPUBANK[ 1 ] = CRAMPAGE( byChr + 1 );
  PPUBANK[ 2 ] = CRAMPAGE( byChr + 2 );
  PPUBANK[ 3 ] = CRAMPAGE( byChr + 3 );
  PPUBANK[ 4 ] = CRAMPAGE( byChr + 4 );
  PPUBANK[ 5 ] = CRAMPAGE( byChr + 5 );
  PPUBANK[ 6 ] = CRAMPAGE( byChr + 6 );
  PPUBANK[ 7 ] = CRAMPAGE( byChr + 7 );
  InfoNES_SetupChr();

  /* Set mirroring if 1-screen mode is active */
  if ( Map30_MirrorMode == 0x08 )
  {
    /* Bit 7: 0 = one-screen lower (0x2000), 1 = one-screen upper (0x2400) */
    if ( byData & 0x80 )
    {
      InfoNES_Mirroring( 2 );  /* One Screen 0x2400 */
    }
    else
    {
      InfoNES_Mirroring( 3 );  /* One Screen 0x2000 */
    }
  }
}
