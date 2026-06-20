/*===================================================================*/
/*                                                                   */
/*           Mapper 75 (Konami VRC 1 and Jaleco SS8805)              */
/*                                                                   */
/*===================================================================*/

BYTE Map75_Regs[ 2 ];

/* Deferred-commit state: VRC1 has no scanline IRQ. Games (notably
   Ganbare Goemon! - Karakuri Douchuu) split HUD vs. playfield CHR by
   writing $E000/$F000 at the end of the last HUD scanline. With the
   line-based renderer that write would otherwise corrupt the current
   scanline; we stage writes in `_Next`, promote them to `_Ready` at
   each HSync, and commit `_Ready` to PPUBANK[] on the following HSync
   so they take effect on the NEXT rendered scanline. */
static BYTE Map75_Chr_Next[ 2 ];
static BYTE Map75_Chr_Ready[ 2 ];
static BYTE Map75_PendingMask_Next;
static BYTE Map75_PendingMask_Ready;
static BYTE Map75_Mirror_Next;
static BYTE Map75_Mirror_Ready;
static BYTE Map75_Mirror_PendingNext;
static BYTE Map75_Mirror_PendingReady;

void Map75_HSync();

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 75                                             */
/*-------------------------------------------------------------------*/
void Map75_Init()
{
  /* Initialize Mapper */
  MapperInit = Map75_Init;

  /* Write to Mapper */
  MapperWrite = Map75_Write;

  /* Write to SRAM */
  MapperSram = Map0_Sram;

  /* Write to APU */
  MapperApu = Map0_Apu;

  /* Read from APU */
  MapperReadApu = Map0_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map0_VSync;

  /* Callback at HSync */
  MapperHSync = Map75_HSync;

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
  Map75_Regs[ 0 ] = 0;
  Map75_Regs[ 1 ] = 1;

  Map75_Chr_Next[ 0 ]  = Map75_Chr_Ready[ 0 ]  = 0;
  Map75_Chr_Next[ 1 ]  = Map75_Chr_Ready[ 1 ]  = 1;
  Map75_PendingMask_Next  = 0;
  Map75_PendingMask_Ready = 0;
  Map75_Mirror_Next       = 0;
  Map75_Mirror_Ready      = 0;
  Map75_Mirror_PendingNext  = 0;
  Map75_Mirror_PendingReady = 0;

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring( 1, 1 );
}

/*-------------------------------------------------------------------*/
/*  Mapper 75 Write Function                                         */
/*-------------------------------------------------------------------*/
void Map75_Write( WORD wAddr, BYTE byData )
{
  switch ( wAddr & 0xf000 )
  {
    /* Set ROM Banks */
    case 0x8000:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK0 = ROMPAGE( byData );
      break;

    case 0x9000:
      /* Stage mirroring + CHR high bits for next-scanline commit. */
      Map75_Mirror_Next        = ( byData & 0x01 ) ? 0 : 1;
      Map75_Mirror_PendingNext = 1;

      Map75_Chr_Next[ 0 ] = ( Map75_Chr_Next[ 0 ] & 0x0f ) | ( ( byData & 0x02 ) << 3 );
      Map75_Chr_Next[ 1 ] = ( Map75_Chr_Next[ 1 ] & 0x0f ) | ( ( byData & 0x04 ) << 2 );
      Map75_PendingMask_Next |= 0x03;

      /* Keep legacy register state in sync for any future readers. */
      Map75_Regs[ 0 ] = Map75_Chr_Next[ 0 ];
      Map75_Regs[ 1 ] = Map75_Chr_Next[ 1 ];
      break;

    /* Set ROM Banks */
    case 0xA000:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK1 = ROMPAGE( byData );
      break;

    /* Set ROM Banks */
    case 0xC000:
      byData %= ( NesHeader.byRomSize << 1 );
      ROMBANK2 = ROMPAGE( byData );
      break;

    case 0xE000:
      Map75_Chr_Next[ 0 ] = ( Map75_Chr_Next[ 0 ] & 0x10 ) | ( byData & 0x0f );
      Map75_PendingMask_Next |= 0x01;
      Map75_Regs[ 0 ] = Map75_Chr_Next[ 0 ];
      break;

    case 0xF000:
      Map75_Chr_Next[ 1 ] = ( Map75_Chr_Next[ 1 ] & 0x10 ) | ( byData & 0x0f );
      Map75_PendingMask_Next |= 0x02;
      Map75_Regs[ 1 ] = Map75_Chr_Next[ 1 ];
      break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 75 H-Sync Function                                        */
/*-------------------------------------------------------------------*/
void Map75_HSync()
{
  /* Commit writes that were staged during the previous scanline,
     then promote writes from the just-finished CPU step into the
     ready slot for the next HSync. Net effect: a write during CPU
     step N takes effect on rendered scanline N+1, matching real-HW
     HBlank-timed CHR-bank swaps closely enough to remove the
     bottom-HUD-line flicker without going pixel-accurate. */
  if ( Map75_PendingMask_Ready & 0x01 )
  {
    PPUBANK[ 0 ] = VROMPAGE( ( Map75_Chr_Ready[ 0 ] << 2 ) + 0 );
    PPUBANK[ 1 ] = VROMPAGE( ( Map75_Chr_Ready[ 0 ] << 2 ) + 1 );
    PPUBANK[ 2 ] = VROMPAGE( ( Map75_Chr_Ready[ 0 ] << 2 ) + 2 );
    PPUBANK[ 3 ] = VROMPAGE( ( Map75_Chr_Ready[ 0 ] << 2 ) + 3 );
  }
  if ( Map75_PendingMask_Ready & 0x02 )
  {
    PPUBANK[ 4 ] = VROMPAGE( ( Map75_Chr_Ready[ 1 ] << 2 ) + 0 );
    PPUBANK[ 5 ] = VROMPAGE( ( Map75_Chr_Ready[ 1 ] << 2 ) + 1 );
    PPUBANK[ 6 ] = VROMPAGE( ( Map75_Chr_Ready[ 1 ] << 2 ) + 2 );
    PPUBANK[ 7 ] = VROMPAGE( ( Map75_Chr_Ready[ 1 ] << 2 ) + 3 );
  }
  if ( Map75_Mirror_PendingReady )
  {
    InfoNES_Mirroring( Map75_Mirror_Ready );
  }

  Map75_Chr_Ready[ 0 ] = Map75_Chr_Next[ 0 ];
  Map75_Chr_Ready[ 1 ] = Map75_Chr_Next[ 1 ];
  Map75_PendingMask_Ready   = Map75_PendingMask_Next;
  Map75_PendingMask_Next    = 0;
  Map75_Mirror_Ready        = Map75_Mirror_Next;
  Map75_Mirror_PendingReady = Map75_Mirror_PendingNext;
  Map75_Mirror_PendingNext  = 0;
}
