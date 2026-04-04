/*===================================================================*/
/*                                                                   */
/*               Mapper 206 (DxROM / Namcot 118)                     */
/*                                                                   */
/*===================================================================*/

/*  Simplified Namco 108 variant that reuses Mapper 4's bank logic:
 *  - All writes in $8000-$FFFF alias to either $8000 or $8001
 *  - Bits 6-7 of the MMC3 bank select register do not exist
 *  - No IRQ support or software-controllable mirroring
 *  - Namcot 34xx CHR wiring forces bit 6 high on the 1 KB banks
 */

/*-------------------------------------------------------------------*/
/*  Mapper 206 Set PPU Banks Function                                */
/*-------------------------------------------------------------------*/
static void Map206_Set_PPU_Banks()
{
  if ( NesHeader.byVRomSize > 0 )
  {
    PPUBANK[ 0 ] = VROMPAGE( ( Map4_Chr01 + 0 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 1 ] = VROMPAGE( ( Map4_Chr01 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 2 ] = VROMPAGE( ( Map4_Chr23 + 0 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 3 ] = VROMPAGE( ( Map4_Chr23 + 1 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 4 ] = VROMPAGE( ( Map4_Chr4 | 0x40 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 5 ] = VROMPAGE( ( Map4_Chr5 | 0x40 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 6 ] = VROMPAGE( ( Map4_Chr6 | 0x40 ) % ( NesHeader.byVRomSize << 3 ) );
    PPUBANK[ 7 ] = VROMPAGE( ( Map4_Chr7 | 0x40 ) % ( NesHeader.byVRomSize << 3 ) );
    InfoNES_SetupChr();
  }
}

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

  /* Callback at HSync - No IRQ */
  MapperHSync = Map0_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map0_RenderScreen;

  /* Set SRAM Banks */
  SRAMBANK = SRAM;

  /* Initialize Mapper 4 State Registers */
  for ( int nPage = 0; nPage < 8; nPage++ )
  {
    Map4_Regs[ nPage ] = 0x00;
  }

  /* Set ROM Banks */
  Map4_Prg0 = 0;
  Map4_Prg1 = 1;
  Map4_Set_CPU_Banks();

  /* Power-on CHR state follows Namco 108 data latches. */
  Map4_Chr01 = 0;
  Map4_Chr23 = 0;
  Map4_Chr4  = 0;
  Map4_Chr5  = 0;
  Map4_Chr6  = 0;
  Map4_Chr7  = 0;

  if ( NesHeader.byVRomSize > 0 )
  {
    Map206_Set_PPU_Banks();
  }

  /* Clear IRQ state (unused but keep clean) */
  Map4_IRQ_Enable = 0;
  Map4_IRQ_Cnt = 0;
  Map4_IRQ_Latch = 0;
  Map4_IRQ_Request = 0;
  Map4_IRQ_Present = 0;
  Map4_IRQ_Present_Vbl = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 );
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
      Map4_Regs[ 0 ] = byData & 0x07;
      break;

    case 0x8001:
      Map4_Regs[ 1 ] = byData;

      if ( ( Map4_Regs[ 0 ] & 0x07 ) <= 0x05 )
      {
        dwBankNum = byData & 0x3f;
      } else {
        dwBankNum = byData & 0x0f;
      }

      switch ( Map4_Regs[ 0 ] & 0x07 )
      {
        /* Set PPU Banks */
        case 0x00:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map4_Chr01 = dwBankNum;
            Map206_Set_PPU_Banks();
          }
          break;

        case 0x01:
          if ( NesHeader.byVRomSize > 0 )
          {
            dwBankNum &= 0xfe;
            Map4_Chr23 = dwBankNum;
            Map206_Set_PPU_Banks();
          }
          break;

        case 0x02:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map4_Chr4 = dwBankNum;
            Map206_Set_PPU_Banks();
          }
          break;

        case 0x03:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map4_Chr5 = dwBankNum;
            Map206_Set_PPU_Banks();
          }
          break;

        case 0x04:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map4_Chr6 = dwBankNum;
            Map206_Set_PPU_Banks();
          }
          break;

        case 0x05:
          if ( NesHeader.byVRomSize > 0 )
          {
            Map4_Chr7 = dwBankNum;
            Map206_Set_PPU_Banks();
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
