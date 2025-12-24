/*===================================================================*/
/*                                                                   */
/*                        Mapper 5 (MMC5)                            */
/*                                                                   */
/*  Re-implemented based on nesdev.org MMC5 specification            */
/*  and Mesen2 reference implementation                              */
/*                                                                   */
/*===================================================================*/

/* MMC5 has 64KB of WRAM (8 banks of 8KB) */
BYTE Map5_Wram[0x2000 * 8];
BYTE Map5_Ex_Ram[0x400];
BYTE Map5_Ex_Vram[0x400];
BYTE Map5_Ex_Nam[0x400];

/* PRG bank registers */
BYTE Map5_Prg_Reg[8];
BYTE Map5_Wram_Reg[8];

/* CHR bank registers: [bank][mode] where mode 0=BG, 1=SPR */
BYTE Map5_Chr_Reg[16];
BYTE Map5_Chr_High_Reg[4];

/* IRQ registers */
BYTE Map5_IRQ_Enable;
BYTE Map5_IRQ_Status;
BYTE Map5_IRQ_Line;
BYTE Map5_IRQ_Counter;
BYTE Map5_In_Frame;

/* Multiplication registers */
DWORD Map5_Value0;
DWORD Map5_Value1;

/* Configuration registers */
BYTE Map5_Wram_Protect0;
BYTE Map5_Wram_Protect1;
BYTE Map5_Prg_Mode;
BYTE Map5_Chr_Mode;
BYTE Map5_Gfx_Mode;

/* Last CHR mode used (for sprite/BG switching) */
BYTE Map5_Last_Chr_Mode;

/* The address of 8Kbytes unit of the Map5 WRAM */
#define Map5_WRAMPAGE(a) &Map5_Wram[((a)&0x07) * 0x2000]

/*-------------------------------------------------------------------*/
/*  Initialize Mapper 5                                              */
/*-------------------------------------------------------------------*/
void Map5_Init()
{
  int nPage;

  /* Initialize Mapper */
  MapperInit = Map5_Init;

  /* Write to Mapper */
  MapperWrite = Map5_Write;

  /* Write to SRAM */
  MapperSram = Map5_Sram;

  /* Write to APU */
  MapperApu = Map5_Apu;

  /* Read from APU */
  MapperReadApu = Map5_ReadApu;

  /* Callback at VSync */
  MapperVSync = Map5_VSync;

  /* Callback at HSync */
  MapperHSync = Map5_HSync;

  /* Callback at PPU */
  MapperPPU = Map0_PPU;

  /* Callback at Rendering Screen ( 1:BG, 0:Sprite ) */
  MapperRenderScreen = Map5_RenderScreen;

  /* Clear memory */
  InfoNES_MemorySet(Map5_Wram, 0x00, sizeof(Map5_Wram));
  InfoNES_MemorySet(Map5_Ex_Ram, 0x00, sizeof(Map5_Ex_Ram));
  InfoNES_MemorySet(Map5_Ex_Vram, 0x00, sizeof(Map5_Ex_Vram));
  InfoNES_MemorySet(Map5_Ex_Nam, 0x00, sizeof(Map5_Ex_Nam));

  /* Initialize registers to power-on defaults */
  Map5_Prg_Mode = 3;
  Map5_Chr_Mode = 3;
  Map5_Gfx_Mode = 0;
  Map5_Wram_Protect0 = 0;
  Map5_Wram_Protect1 = 0;
  
  /* Initialize PRG registers */
  for (nPage = 0; nPage < 8; ++nPage)
  {
    Map5_Prg_Reg[nPage] = 0xFF;
    Map5_Wram_Reg[nPage] = 0xFF;
  }
  
  /* Initialize CHR registers */
  for (nPage = 0; nPage < 16; ++nPage)
  {
    Map5_Chr_Reg[nPage] = nPage;
  }
  
  for (nPage = 0; nPage < 4; ++nPage)
  {
    Map5_Chr_High_Reg[nPage] = 0;
  }

  /* Initialize IRQ */
  Map5_IRQ_Enable = 0;
  Map5_IRQ_Status = 0;
  Map5_IRQ_Line = 0;
  Map5_IRQ_Counter = 0;
  Map5_In_Frame = 0;

  /* Initialize multiplication */
  Map5_Value0 = 0xFF;
  Map5_Value1 = 0xFF;
  
  Map5_Last_Chr_Mode = 0;

  /* Set SRAM Bank to first WRAM bank */
  SRAMBANK = Map5_WRAMPAGE(0);

  /* Set initial ROM banks - last bank fixed to end */
  Map5_Sync_Prg_Banks();
  
  /* Set PPU Banks */
  for (nPage = 0; nPage < 8; ++nPage)
    PPUBANK[nPage] = VROMPAGE(nPage % (NesHeader.byVRomSize << 3));
  InfoNES_SetupChr();

  /* Set up wiring of the interrupt pin */
  K6502_Set_Int_Wiring(1, 1);
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Read from APU Function                                  */
/*-------------------------------------------------------------------*/
BYTE Map5_ReadApu(WORD wAddr)
{
  BYTE byRet = (BYTE)(wAddr >> 8);

  switch (wAddr)
  {
  case 0x5204:
    byRet = Map5_IRQ_Status;
    Map5_IRQ_Status = 0;
    break;

  case 0x5205:
    byRet = (BYTE)((Map5_Value0 * Map5_Value1) & 0x00ff);
    break;

  case 0x5206:
    byRet = (BYTE)(((Map5_Value0 * Map5_Value1) & 0xff00) >> 8);
    break;

  default:
    if (0x5c00 <= wAddr && wAddr <= 0x5fff)
    {
      byRet = Map5_Ex_Ram[wAddr - 0x5c00];
    }
    break;
  }
  return byRet;
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Write to APU Function                                   */
/*-------------------------------------------------------------------*/
void Map5_Apu(WORD wAddr, BYTE byData)
{
  int nPage;

  switch (wAddr)
  {
  case 0x5100:
    /* PRG mode */
    Map5_Prg_Mode = byData & 0x03;
    Map5_Sync_Prg_Banks();
    break;

  case 0x5101:
    /* CHR mode */
    Map5_Chr_Mode = byData & 0x03;
    break;

  case 0x5102:
    /* PRG RAM protect 0 */
    Map5_Wram_Protect0 = byData & 0x03;
    break;

  case 0x5103:
    /* PRG RAM protect 1 */
    Map5_Wram_Protect1 = byData & 0x03;
    break;

  case 0x5104:
    /* Extended RAM mode */
    Map5_Gfx_Mode = byData & 0x03;
    break;

  case 0x5105:
    /* Nametable mapping */
    for (nPage = 0; nPage < 4; nPage++)
    {
      BYTE byNamReg = (byData >> (nPage * 2)) & 0x03;

      switch (byNamReg)
      {
      case 0:
        /* Internal VRAM page 0 */
        PPUBANK[nPage + 8] = VRAMPAGE(0);
        break;
      case 1:
        /* Internal VRAM page 1 */
        PPUBANK[nPage + 8] = VRAMPAGE(1);
        break;
      case 2:
        /* Extended RAM (ExRAM as nametable) */
        PPUBANK[nPage + 8] = Map5_Ex_Ram;
        break;
      case 3:
        /* Fill mode (uses registers $5106/$5107) */
        PPUBANK[nPage + 8] = Map5_Ex_Nam;
        break;
      }
    }
    break;

  case 0x5106:
    /* Fill mode tile */
    InfoNES_MemorySet(Map5_Ex_Nam, byData, 0x3c0);
    break;

  case 0x5107:
    /* Fill mode attribute */
    byData &= 0x03;
    byData = byData | (byData << 2) | (byData << 4) | (byData << 6);
    InfoNES_MemorySet(&(Map5_Ex_Nam[0x3c0]), byData, 0x400 - 0x3c0);
    break;

  case 0x5113:
    /* PRG RAM bank at $6000-$7FFF */
    Map5_Prg_Reg[3] = byData;
    if ((byData & 0x80) == 0)
    {
      /* WRAM mode */
      SRAMBANK = Map5_WRAMPAGE(byData & 0x07);
    }
    break;

  case 0x5114:
  case 0x5115:
  case 0x5116:
  case 0x5117:
    /* PRG bank registers */
    Map5_Prg_Reg[wAddr & 0x07] = byData;
    Map5_Sync_Prg_Banks();
    break;

  case 0x5120:
  case 0x5121:
  case 0x5122:
  case 0x5123:
  case 0x5124:
  case 0x5125:
  case 0x5126:
  case 0x5127:
    /* CHR bank registers (background) */
    Map5_Chr_Reg[wAddr & 0x0F] = byData;
    break;

  case 0x5128:
  case 0x5129:
  case 0x512A:
  case 0x512B:
    /* CHR bank registers (sprite, upper bits) */
    {
      BYTE idx = wAddr & 0x03;
      Map5_Chr_High_Reg[idx] = byData;
      /* Also set the sprite CHR registers */
      Map5_Chr_Reg[8 + (idx * 2)] = byData;
      Map5_Chr_Reg[8 + (idx * 2) + 1] = byData;
    }
    break;

  case 0x5200:
    /* Vertical split mode (not implemented yet) */
    break;

  case 0x5201:
    /* Vertical split scroll (not implemented yet) */
    break;

  case 0x5202:
    /* Vertical split bank (not implemented yet) */
    break;

  case 0x5203:
    /* IRQ scanline compare */
    Map5_IRQ_Line = byData;
    break;

  case 0x5204:
    /* IRQ enable */
    Map5_IRQ_Enable = byData & 0x80;
    break;

  case 0x5205:
    /* Multiplicand */
    Map5_Value0 = byData;
    break;

  case 0x5206:
    /* Multiplier */
    Map5_Value1 = byData;
    break;

  default:
    if (0x5000 <= wAddr && wAddr <= 0x5015)
    {
      /* Extra sound (not implemented) */
    }
    else if (0x5C00 <= wAddr && wAddr <= 0x5FFF)
    {
      /* Extended RAM */
      Map5_Ex_Ram[wAddr - 0x5C00] = byData;
    }
    break;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Write to SRAM Function                                  */
/*-------------------------------------------------------------------*/
void Map5_Sram(WORD wAddr, BYTE byData)
{
  /* Check if WRAM writes are enabled */
  if (Map5_Wram_Protect0 == 0x02 && Map5_Wram_Protect1 == 0x01)
  {
    /* Write to current SRAM bank */
    SRAMBANK[wAddr - 0x6000] = byData;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Write Function                                          */
/*-------------------------------------------------------------------*/
void Map5_Write(WORD wAddr, BYTE byData)
{
  /* MMC5 ROM areas are not writable unless they're mapped to WRAM */
  /* This is handled by the PRG bank configuration */
  /* Writes to ROM areas do nothing unless WRAM is mapped there */
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 V-Sync Function                                         */
/*-------------------------------------------------------------------*/
void Map5_VSync()
{
  /* VBlank has started */
  Map5_In_Frame = 0;
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 H-Sync Function                                         */
/*-------------------------------------------------------------------*/
void Map5_HSync()
{
  /* Track if we're in the visible frame */
  if (PPU_Scanline == 0)
  {
    Map5_In_Frame = 1;
    Map5_IRQ_Counter = 0;
    Map5_IRQ_Status &= 0x7F;
  }
  else if (PPU_Scanline < 240 && Map5_In_Frame)
  {
    /* Increment scanline counter during visible scanlines */
    Map5_IRQ_Counter = PPU_Scanline;
    
    /* Check for IRQ */
    if (Map5_IRQ_Counter == Map5_IRQ_Line)
    {
      Map5_IRQ_Status |= 0x80;
      if (Map5_IRQ_Enable)
      {
        IRQ_REQ;
      }
    }
  }
  else if (PPU_Scanline == 240)
  {
    /* End of visible frame */
    Map5_In_Frame = 0;
    Map5_IRQ_Status |= 0x40;
  }
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Rendering Screen Function                               */
/*-------------------------------------------------------------------*/
void Map5_RenderScreen(BYTE byMode)
{
  DWORD dwPage[8];
  int i;

  /* byMode: 1 = Background, 0 = Sprite */
  /* Map CHR banks based on mode and CHR size */
  
  switch (Map5_Chr_Mode)
  {
  case 0:
    /* 8KB mode - use last register */
    if (byMode)
    {
      /* Background - use register 7 */
      dwPage[0] = ((DWORD)Map5_Chr_Reg[7] << 3) % (NesHeader.byVRomSize << 3);
    }
    else
    {
      /* Sprite - use upper bits from $5128-$512B */
      dwPage[0] = ((DWORD)Map5_Chr_High_Reg[3] << 3) % (NesHeader.byVRomSize << 3);
    }
    
    for (i = 0; i < 8; i++)
    {
      PPUBANK[i] = VROMPAGE(dwPage[0] + i);
    }
    break;

  case 1:
    /* 4KB mode - use registers 3 and 7 */
    if (byMode)
    {
      /* Background */
      dwPage[0] = ((DWORD)Map5_Chr_Reg[3] << 2) % (NesHeader.byVRomSize << 3);
      dwPage[1] = ((DWORD)Map5_Chr_Reg[7] << 2) % (NesHeader.byVRomSize << 3);
    }
    else
    {
      /* Sprite */
      dwPage[0] = ((DWORD)Map5_Chr_High_Reg[1] << 2) % (NesHeader.byVRomSize << 3);
      dwPage[1] = ((DWORD)Map5_Chr_High_Reg[3] << 2) % (NesHeader.byVRomSize << 3);
    }
    
    for (i = 0; i < 4; i++)
    {
      PPUBANK[i] = VROMPAGE(dwPage[0] + i);
      PPUBANK[i + 4] = VROMPAGE(dwPage[1] + i);
    }
    break;

  case 2:
    /* 2KB mode - use registers 1, 3, 5, 7 */
    if (byMode)
    {
      /* Background */
      dwPage[0] = ((DWORD)Map5_Chr_Reg[1] << 1) % (NesHeader.byVRomSize << 3);
      dwPage[1] = ((DWORD)Map5_Chr_Reg[3] << 1) % (NesHeader.byVRomSize << 3);
      dwPage[2] = ((DWORD)Map5_Chr_Reg[5] << 1) % (NesHeader.byVRomSize << 3);
      dwPage[3] = ((DWORD)Map5_Chr_Reg[7] << 1) % (NesHeader.byVRomSize << 3);
    }
    else
    {
      /* Sprite */
      dwPage[0] = ((DWORD)Map5_Chr_High_Reg[0] << 1) % (NesHeader.byVRomSize << 3);
      dwPage[1] = ((DWORD)Map5_Chr_High_Reg[1] << 1) % (NesHeader.byVRomSize << 3);
      dwPage[2] = ((DWORD)Map5_Chr_High_Reg[2] << 1) % (NesHeader.byVRomSize << 3);
      dwPage[3] = ((DWORD)Map5_Chr_High_Reg[3] << 1) % (NesHeader.byVRomSize << 3);
    }
    
    for (i = 0; i < 2; i++)
    {
      PPUBANK[i] = VROMPAGE(dwPage[0] + i);
      PPUBANK[i + 2] = VROMPAGE(dwPage[1] + i);
      PPUBANK[i + 4] = VROMPAGE(dwPage[2] + i);
      PPUBANK[i + 6] = VROMPAGE(dwPage[3] + i);
    }
    break;

  default:
    /* 1KB mode - use all 8 registers */
    if (byMode)
    {
      /* Background - use registers 0-7 */
      for (i = 0; i < 8; i++)
      {
        dwPage[i] = (DWORD)Map5_Chr_Reg[i] % (NesHeader.byVRomSize << 3);
        PPUBANK[i] = VROMPAGE(dwPage[i]);
      }
    }
    else
    {
      /* Sprite - use registers 8-15 (set from $5128-$512B) */
      for (i = 0; i < 8; i++)
      {
        dwPage[i] = (DWORD)Map5_Chr_Reg[8 + i] % (NesHeader.byVRomSize << 3);
        PPUBANK[i] = VROMPAGE(dwPage[i]);
      }
    }
    break;
  }
  
  InfoNES_SetupChr();
}

/*-------------------------------------------------------------------*/
/*  Mapper 5 Sync Program Banks Function                             */
/*-------------------------------------------------------------------*/
void Map5_Sync_Prg_Banks(void)
{
  BYTE bank;
  
  switch (Map5_Prg_Mode)
  {
  case 0:
    /* 32KB mode - use only register $5117 */
    bank = Map5_Prg_Reg[7] & 0x7C;  /* Ignore bottom 2 bits for 32KB alignment */
    
    if (Map5_Prg_Reg[7] & 0x80)
    {
      /* ROM mode */
      ROMBANK0 = ROMPAGE((bank + 0) % (NesHeader.byRomSize << 1));
      ROMBANK1 = ROMPAGE((bank + 1) % (NesHeader.byRomSize << 1));
      ROMBANK2 = ROMPAGE((bank + 2) % (NesHeader.byRomSize << 1));
      ROMBANK3 = ROMPAGE((bank + 3) % (NesHeader.byRomSize << 1));
    }
    break;

  case 1:
    /* 16KB+16KB mode - use registers $5115 and $5117 */
    
    /* First 16KB bank at $8000-$BFFF */
    bank = Map5_Prg_Reg[5] & 0x7E;  /* Ignore bottom bit for 16KB alignment */
    
    if (Map5_Prg_Reg[5] & 0x80)
    {
      /* ROM mode */
      ROMBANK0 = ROMPAGE((bank + 0) % (NesHeader.byRomSize << 1));
      ROMBANK1 = ROMPAGE((bank + 1) % (NesHeader.byRomSize << 1));
    }
    else
    {
      /* WRAM mode */
      ROMBANK0 = Map5_WRAMPAGE((bank >> 1) & 0x07);
      ROMBANK1 = Map5_WRAMPAGE(((bank >> 1) + 1) & 0x07);
    }
    
    /* Second 16KB bank at $C000-$FFFF */
    bank = Map5_Prg_Reg[7] & 0x7E;
    ROMBANK2 = ROMPAGE((bank + 0) % (NesHeader.byRomSize << 1));
    ROMBANK3 = ROMPAGE((bank + 1) % (NesHeader.byRomSize << 1));
    break;

  case 2:
    /* 16KB+8KB+8KB mode - use registers $5115, $5116, $5117 */
    
    /* First 16KB bank at $8000-$BFFF */
    bank = Map5_Prg_Reg[5] & 0x7E;
    
    if (Map5_Prg_Reg[5] & 0x80)
    {
      /* ROM mode */
      ROMBANK0 = ROMPAGE((bank + 0) % (NesHeader.byRomSize << 1));
      ROMBANK1 = ROMPAGE((bank + 1) % (NesHeader.byRomSize << 1));
    }
    else
    {
      /* WRAM mode */
      ROMBANK0 = Map5_WRAMPAGE((bank >> 1) & 0x07);
      ROMBANK1 = Map5_WRAMPAGE(((bank >> 1) + 1) & 0x07);
    }
    
    /* 8KB bank at $C000-$DFFF */
    bank = Map5_Prg_Reg[6] & 0x7F;
    
    if (Map5_Prg_Reg[6] & 0x80)
    {
      /* ROM mode */
      ROMBANK2 = ROMPAGE(bank % (NesHeader.byRomSize << 1));
    }
    else
    {
      /* WRAM mode */
      ROMBANK2 = Map5_WRAMPAGE(bank & 0x07);
    }
    
    /* Last 8KB bank at $E000-$FFFF (always ROM) */
    bank = Map5_Prg_Reg[7] & 0x7F;
    ROMBANK3 = ROMPAGE(bank % (NesHeader.byRomSize << 1));
    break;

  default:
    /* Mode 3: 8KB+8KB+8KB+8KB mode - use all registers $5114-$5117 */
    
    /* Bank at $8000-$9FFF */
    bank = Map5_Prg_Reg[4] & 0x7F;
    
    if (Map5_Prg_Reg[4] & 0x80)
    {
      /* ROM mode */
      ROMBANK0 = ROMPAGE(bank % (NesHeader.byRomSize << 1));
    }
    else
    {
      /* WRAM mode */
      ROMBANK0 = Map5_WRAMPAGE(bank & 0x07);
    }
    
    /* Bank at $A000-$BFFF */
    bank = Map5_Prg_Reg[5] & 0x7F;
    
    if (Map5_Prg_Reg[5] & 0x80)
    {
      /* ROM mode */
      ROMBANK1 = ROMPAGE(bank % (NesHeader.byRomSize << 1));
    }
    else
    {
      /* WRAM mode */
      ROMBANK1 = Map5_WRAMPAGE(bank & 0x07);
    }
    
    /* Bank at $C000-$DFFF */
    bank = Map5_Prg_Reg[6] & 0x7F;
    
    if (Map5_Prg_Reg[6] & 0x80)
    {
      /* ROM mode */
      ROMBANK2 = ROMPAGE(bank % (NesHeader.byRomSize << 1));
    }
    else
    {
      /* WRAM mode */
      ROMBANK2 = Map5_WRAMPAGE(bank & 0x07);
    }
    
    /* Bank at $E000-$FFFF (always ROM) */
    bank = Map5_Prg_Reg[7] & 0x7F;
    ROMBANK3 = ROMPAGE(bank % (NesHeader.byRomSize << 1));
    break;
  }
}
