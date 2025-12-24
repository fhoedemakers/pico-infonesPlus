/*===================================================================*/
/*                                                                   */
/*         Mapper 206 (Namco 108 variant) - MMC3 derivative         */
/*                                                                   */
/*===================================================================*/

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

  /* Initialize Mapper 4 variables */
  for ( int i = 0; i < 8; i++ )
  {
    Map4_Regs[ i ] = 0;
  }
  
  Map4_Regs[ 0 ] = 0;  /* Ensure CHR/PRG swap bits are 0 */
  Map4_Regs[ 1 ] = 0;
  
  /* Initialize mirroring register to opposite of header bit for MMC3 compatibility
   * Per NESdev forum: https://forums.nesdev.org/viewtopic.php?t=25121
   * When Mapper 206 games run as MMC3, the mirroring register must be initialized
   * to the inverse of the iNES header's nametable arrangement bit.
   * Header bit 0 (horizontal) -> MMC3 register = 1 (horizontal)
   * Header bit 1 (vertical) -> MMC3 register = 0 (vertical)
   */
  if ( !ROM_FourScr )
  {
    Map4_Regs[ 2 ] = ( NesHeader.byInfo1 & 0x01 ) ? 0x00 : 0x01;
    if ( Map4_Regs[ 2 ] & 0x01 )
    {
      InfoNES_Mirroring( 0 ); // horizontal
    } else {
      InfoNES_Mirroring( 1 ); // vertical
    }
  }

  Map4_Rom_Bank = 0;
  Map4_Prg0 = 0;
  Map4_Prg1 = 1;
  Map4_Chr01 = 0;
  Map4_Chr23 = 2;
  Map4_Chr4 = 4;
  Map4_Chr5 = 5;
  Map4_Chr6 = 6;
  Map4_Chr7 = 7;

  /* Set up CPU and PPU banks using Mapper 4's functions */
  Map4_Set_CPU_Banks();
  Map4_Set_PPU_Banks();

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 206 Write Function                                        */
/*-------------------------------------------------------------------*/
void Map206_Write( WORD wAddr, BYTE byData )
{
  DWORD dwBankNum;

  /* Mapper 206 only responds to $8000-$8001 */
  switch ( wAddr & 0x8001 )
  {
    case 0x8000:
      /* Mask out CHR/PRG swap bits (bits 6-7) to keep them always 0 */
      Map4_Regs[ 0 ] = byData & 0x3F;
      break;

    case 0x8001:
      Map4_Regs[ 1 ] = byData;
      dwBankNum = Map4_Regs[ 1 ];

      switch ( Map4_Regs[ 0 ] & 0x07 )
      {
        /* Set PPU Banks */
        case 0x00:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map4_Chr01 = dwBankNum;
            Map4_Set_PPU_Banks();
          }
          break;

        case 0x01:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map4_Chr23 = dwBankNum;
            Map4_Set_PPU_Banks();
          }
          break;

        case 0x02:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map4_Chr4 = dwBankNum;
            Map4_Set_PPU_Banks();
          }
          break;

        case 0x03:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map4_Chr5 = dwBankNum;
            Map4_Set_PPU_Banks();
          }
          break;

        case 0x04:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map4_Chr6 = dwBankNum;
            Map4_Set_PPU_Banks();
          }
          break;

        case 0x05:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map4_Chr7 = dwBankNum;
            Map4_Set_PPU_Banks();
          }
          break;

        /* Set ROM Banks */
        case 0x06:
          Map4_Prg0 = dwBankNum;
          Map4_Set_CPU_Banks();
          break;

        case 0x07:
          Map4_Prg1 = dwBankNum;
          Map4_Set_CPU_Banks();
          break;
      }
      break;
  }
}
