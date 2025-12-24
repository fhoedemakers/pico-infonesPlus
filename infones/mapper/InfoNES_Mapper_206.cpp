/*===================================================================*/
/*                                                                   */
/*                  Mapper 206 (Namco 108 variant)                   */
/*                                                                   */
/*===================================================================*/

BYTE  Map206_Regs[ 8 ];
DWORD Map206_Chr01, Map206_Chr23;
DWORD Map206_Chr4, Map206_Chr5, Map206_Chr6, Map206_Chr7;

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 206                                            */
/*-------------------------------------------------------------------*/
void Map206_Init()
{
  /* Initialize Mapper */
  MapperInit = Map206_Init;

  /* Write to Mapper */
  MapperWrite = Map206_Write;

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

  /* Set ROM Banks */
  ROMBANK0 = ROMPAGE( 0 );
  ROMBANK1 = ROMPAGE( 1 );
  ROMBANK2 = ROMLASTPAGE( 1 );
  ROMBANK3 = ROMLASTPAGE( 0 );

  /* Initialize registers */
  for ( int i = 0; i < 8; i++ )
  {
    Map206_Regs[ i ] = 0;
  }
  
  Map206_Chr01 = 0;
  Map206_Chr23 = 2;
  Map206_Chr4 = 4;
  Map206_Chr5 = 5;
  Map206_Chr6 = 6;
  Map206_Chr7 = 7;

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    Map206_Set_PPU_Banks();
  }

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 206 Set PPU Banks Function                                */
/*-------------------------------------------------------------------*/
void Map206_Set_PPU_Banks()
{
  if ( NesHeader.byVRomSize > 0 )
  {
    PPUBANK[ 0 ] = VROMPAGE( Map206_Chr01 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 1 ] = VROMPAGE( ( Map206_Chr01 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 2 ] = VROMPAGE( Map206_Chr23 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 3 ] = VROMPAGE( ( Map206_Chr23 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 4 ] = VROMPAGE( Map206_Chr4 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 5 ] = VROMPAGE( Map206_Chr5 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 6 ] = VROMPAGE( Map206_Chr6 % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 7 ] = VROMPAGE( Map206_Chr7 % ( NesHeader.byVRomSize << 3 ) );
    InfoNES_SetupChr();
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 206 Write Function                                        */
/*-------------------------------------------------------------------*/
void Map206_Write( WORD wAddr, BYTE byData )
{
  DWORD dwBankNum;

  switch ( wAddr & 0x8001 )
  {
    case 0x8000:
      // Mask out CHR/PRG swap bits (bits 6-7) to keep them always 0
      Map206_Regs[ 0 ] = byData & 0x3F;
      break;

    case 0x8001:
      Map206_Regs[ 1 ] = byData;
      dwBankNum = Map206_Regs[ 1 ];

      switch ( Map206_Regs[ 0 ] & 0x07 )
      {
        /* Set PPU Banks */
        case 0x00:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map206_Chr01 = dwBankNum;
            Map206_Set_PPU_Banks();
          }
          break;

        case 0x01:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map206_Chr23 = dwBankNum;
            Map206_Set_PPU_Banks();
          }
          break;

        case 0x02:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map206_Chr4 = dwBankNum;
            Map206_Set_PPU_Banks();
          }
          break;

        case 0x03:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map206_Chr5 = dwBankNum;
            Map206_Set_PPU_Banks();
          }
          break;

        case 0x04:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map206_Chr6 = dwBankNum;
            Map206_Set_PPU_Banks();
          }
          break;

        case 0x05:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map206_Chr7 = dwBankNum;
            Map206_Set_PPU_Banks();
          }
          break;

        /* Set ROM Banks */
        case 0x06:
          dwBankNum %= ( NesHeader.byRomSize << 1 );
          ROMBANK0 = ROMPAGE( dwBankNum );
          break;

        case 0x07:
          dwBankNum %= ( NesHeader.byRomSize << 1 );
          ROMBANK1 = ROMPAGE( dwBankNum );
          break;
      }
      break;
  }
}
