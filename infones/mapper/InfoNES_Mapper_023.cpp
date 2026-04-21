/*===================================================================*/
/*                                                                   */
/*                  Mapper 23 (Konami VRC2 type B)                   */
/*                                                                   */
/*===================================================================*/

BYTE Map23_Regs[ 9 ];

BYTE Map23_IRQ_Enable;
BYTE Map23_IRQ_Cnt;
BYTE Map23_IRQ_Latch;

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 23                                             */
/*-------------------------------------------------------------------*/
void Map23_Init()
{
  /* Initialize Mapper */
  MapperInit = Map23_Init;

  /* Write to Mapper */
  MapperWrite = Map23_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map23_HSync;

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

  /* Set PPU Banks */
  if ( NesHeader.byVRomSize > 0 )
  {
    for ( int nPage = 0; nPage < 8; ++nPage )
      PPUBANK[ nPage ] = VROMPAGE( nPage );
    InfoNES_SetupChr();
  }

  /* Initialize State Flag */
  Map23_Regs[ 0 ] = 0;
  Map23_Regs[ 1 ] = 1;
  Map23_Regs[ 2 ] = 2;
  Map23_Regs[ 3 ] = 3;
  Map23_Regs[ 4 ] = 4;
  Map23_Regs[ 5 ] = 5;
  Map23_Regs[ 6 ] = 6;
  Map23_Regs[ 7 ] = 7;
  Map23_Regs[ 8 ] = 0;

  Map23_IRQ_Enable = 0;
  Map23_IRQ_Cnt = 0;
  Map23_IRQ_Latch = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 ); 
}

/*-------------------------------------------------------------------*/
/*  Mapper 23 Write Function                                         */
/*-------------------------------------------------------------------*/
void Map23_Write( WORD wAddr, BYTE byData )
{
  /* VRC2b/VRC4e/VRC4f differ only in which address bits (A0/A1 vs A2/A3)
   * select the sub-register within each $Xxxx block. Canonicalize by OR-ing
   * (A0|A2) and (A1|A3) to get a unified sub-index that works for all
   * variants. This handles any address writes like 0x8FFF → 0x8003 for VRC4f. */
  wAddr = ( wAddr & 0xF000 ) | ( ( ( wAddr >> 2 ) | wAddr ) & 0x0003 );

  switch ( wAddr )
  {
    /* VRC4f uses A0/A1 (sub-indices 0..3 at +0..+3); VRC4e uses A2/A3
     * (sub-indices 0..3 at +0/+4/+8/+C). All four sub-indices in the
     * $8000 and $A000 blocks select the same PRG register, so accept both
     * variants' addresses. */
    case 0x8000:
    case 0x8001:
    case 0x8002:
    case 0x8003:
      byData %= ( NesHeader.byRomSize << 1 );
      if ( Map23_Regs[ 8 ] )
      {
        ROMBANK2 = ROMPAGE( byData );
      } else {
        ROMBANK0 = ROMPAGE( byData );
      }
      break;

    /* $9000/$9001 (VRC4f) and $9000/$9004 (VRC4e) are the mirroring
     * register; sub-index 1 is a hardware mirror of sub-index 0. */
    case 0x9000:
    case 0x9001:
      switch ( byData & 0x03 )
      {
       case 0x00:
          InfoNES_Mirroring( 1 );
          break;
        case 0x01:
          InfoNES_Mirroring( 0 );
          break;
        case 0x02:
          InfoNES_Mirroring( 3 );
          break;
        case 0x03:
          InfoNES_Mirroring( 2 );
          break;
      }
      break;

    /* PRG swap mode bit. $9002/$9003 for VRC4f, $9008/$900C for VRC4e. */
    case 0x9002:
    case 0x9003:
      Map23_Regs[ 8 ] = byData & 0x02;
      break;

    case 0xa000:
    case 0xa001:
    case 0xa002:
    case 0xa003:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK1 = ROMPAGE( byData );
      break;

    case 0xb000:
      Map23_Regs[ 0 ] = ( Map23_Regs[ 0 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 0 ] = VROMPAGE( Map23_Regs[ 0 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xb001:
      Map23_Regs[ 0 ] = ( Map23_Regs[ 0 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 0 ] = VROMPAGE( Map23_Regs[ 0 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xb002:
      Map23_Regs[ 1 ] = ( Map23_Regs[ 1 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 1 ] = VROMPAGE( Map23_Regs[ 1 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xb003:
      Map23_Regs[ 1 ] = ( Map23_Regs[ 1 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 1 ] = VROMPAGE( Map23_Regs[ 1 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xc000:
      Map23_Regs[ 2 ] = ( Map23_Regs[ 2 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 2 ] = VROMPAGE( Map23_Regs[ 2 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xc001:
      Map23_Regs[ 2 ] = ( Map23_Regs[ 2 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 2 ] = VROMPAGE( Map23_Regs[ 2 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xc002:
      Map23_Regs[ 3 ] = ( Map23_Regs[ 3 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 3 ] = VROMPAGE( Map23_Regs[ 3 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xc003:
      Map23_Regs[ 3 ] = ( Map23_Regs[ 3 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 3 ] = VROMPAGE( Map23_Regs[ 3 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xd000:
      Map23_Regs[ 4 ] = ( Map23_Regs[ 4 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 4 ] = VROMPAGE( Map23_Regs[ 4 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xd001:
      Map23_Regs[ 4 ] = ( Map23_Regs[ 4 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 4 ] = VROMPAGE( Map23_Regs[ 4 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xd002:
      Map23_Regs[ 5 ] = ( Map23_Regs[ 5 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 5 ] = VROMPAGE( Map23_Regs[ 5 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xd003:
      Map23_Regs[ 5 ] = ( Map23_Regs[ 5 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 5 ] = VROMPAGE( Map23_Regs[ 5 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;   
      
    case 0xe000:
      Map23_Regs[ 6 ] = ( Map23_Regs[ 6 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 6 ] = VROMPAGE( Map23_Regs[ 6 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xe001:
      Map23_Regs[ 6 ] = ( Map23_Regs[ 6 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 6 ] = VROMPAGE( Map23_Regs[ 6 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xe002:
      Map23_Regs[ 7 ] = ( Map23_Regs[ 7 ] & 0xf0 ) | ( byData & 0x0f );
      PPUBANK[ 7 ] = VROMPAGE( Map23_Regs[ 7 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    case 0xe003:
      Map23_Regs[ 7 ] = ( Map23_Regs[ 7 ] & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      PPUBANK[ 7 ] = VROMPAGE( Map23_Regs[ 7 ] % ( NesHeader.byVRomSize << 3 ) );
      InfoNES_SetupChr();
      break;

    /* IRQ latch low. $F000 is sub-index 0 in every variant. */
    case 0xf000:
      Map23_IRQ_Latch = ( Map23_IRQ_Latch & 0xf0 ) | ( byData & 0x0f );
      break;

    case 0xf001:
      Map23_IRQ_Latch = ( Map23_IRQ_Latch & 0x0f ) | ( ( byData & 0x0f ) << 4 );
      break;

    case 0xf002:
      Map23_IRQ_Enable = byData & 0x03;
      if ( Map23_IRQ_Enable & 0x02 )
      {
        Map23_IRQ_Cnt = Map23_IRQ_Latch;
      }
      break;

    case 0xf003:
      if ( Map23_IRQ_Enable & 0x01 )
      {
        Map23_IRQ_Enable |= 0x02;
      } else {
        Map23_IRQ_Enable &= 0x01;
      }
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 23 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void Map23_HSync()
{
/*
 *  Callback at HSync
 *
 */
  if ( Map23_IRQ_Enable & 0x02 )
  {
    if ( Map23_IRQ_Cnt == 0xff )
    {
      IRQ_REQ;

      Map23_IRQ_Cnt = Map23_IRQ_Latch;
      if ( Map23_IRQ_Enable & 0x01 )
      {
        Map23_IRQ_Enable |= 0x02;
      } else {
        Map23_IRQ_Enable &= 0x01;
      }
    } else {
      Map23_IRQ_Cnt++;
    }
  }
}
