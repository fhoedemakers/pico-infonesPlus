/*===================================================================*/
/*                                                                   */
/*                   Mapper 184 (Sunsoft-1)                          */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 184                                            */
/*-------------------------------------------------------------------*/
void Map184_Init()
{
  /* Initialize Mapper */
  MapperInit = Map184_Init;

  /* Write to Mapper */
  MapperWrite = Map0_Write;

  /* Write to SRAM */
  MapperSram = Map184_Sram;

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

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE( 0 );
  ROMBANK1 = ROMPAGE( 1 );
  ROMBANK2 = ROMPAGE( 2 );
  ROMBANK3 = ROMPAGE( 3 );

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 );
}

/*-------------------------------------------------------------------*/
/*  Mapper 184 Write to SRAM Function                                */
/*-------------------------------------------------------------------*/
void Map184_Sram( WORD wAddr, BYTE byData )
{
  DWORD dwLowBank  = ( byData & 0x07 ) << 2;
  DWORD dwHighBank = ( ( ( byData >> 4 ) & 0x07 ) | 0x04 ) << 2;

  DWORD dwVRomMask = ( NesHeader.byVRomSize << 3 ) - 1;

  /* Set PPU Banks for $0000-$0FFF */
  PPUBANK[ 0 ] = VROMPAGE( ( dwLowBank + 0 ) & dwVRomMask );
  PPUBANK[ 1 ] = VROMPAGE( ( dwLowBank + 1 ) & dwVRomMask );
  PPUBANK[ 2 ] = VROMPAGE( ( dwLowBank + 2 ) & dwVRomMask );
  PPUBANK[ 3 ] = VROMPAGE( ( dwLowBank + 3 ) & dwVRomMask );

  /* Set PPU Banks for $1000-$1FFF */
  PPUBANK[ 4 ] = VROMPAGE( ( dwHighBank + 0 ) & dwVRomMask );
  PPUBANK[ 5 ] = VROMPAGE( ( dwHighBank + 1 ) & dwVRomMask );
  PPUBANK[ 6 ] = VROMPAGE( ( dwHighBank + 2 ) & dwVRomMask );
  PPUBANK[ 7 ] = VROMPAGE( ( dwHighBank + 3 ) & dwVRomMask );

  InfoNES_SetupChr();
}
