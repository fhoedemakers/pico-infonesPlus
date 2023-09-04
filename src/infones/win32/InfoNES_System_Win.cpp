/*===================================================================*/
/*                                                                   */
/*  InfoNES_System_Win.cpp : The function which depends on a system  */
/*                           (for Windows)                           */
/*                                                                   */
/*  2000/05/12  InfoNES Project                                      */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/
#include <windows.h>
#include <mmsystem.h>
#include <limits.h>
#include <stdio.h>
#include <crtdbg.h>
#include <stdarg.h>

#include "../InfoNES.h"
#include "../InfoNES_System.h"
#include "InfoNES_Resource_Win.h"
#include "InfoNES_Sound_Win.h"
#include "../InfoNES_pAPU.h"

/*-------------------------------------------------------------------*/
/*  ROM image file information                                       */
/*-------------------------------------------------------------------*/

char szRomName[ 256 ];
char szSaveName[ 256 ];
int nSRAM_SaveFlag;

/*-------------------------------------------------------------------*/
/*  Variables for Windows                                            */
/*-------------------------------------------------------------------*/
#define APP_NAME     "InfoNES v0.93J"
 
HWND hWndMain;
WNDCLASS wc;
HACCEL hAccel;

byte *pScreenMem;
HBITMAP hScreenBmp;
LOGPALETTE *plpal;
BITMAPINFO *bmi;

// Palette data
WORD NesPalette[ 64 ] =
{
  0x39ce, 0x1071, 0x0015, 0x2013, 0x440e, 0x5402, 0x5000, 0x3c20,
  0x20a0, 0x0100, 0x0140, 0x00e2, 0x0ceb, 0x0000, 0x0000, 0x0000,
  0x5ef7, 0x01dd, 0x10fd, 0x401e, 0x5c17, 0x700b, 0x6ca0, 0x6521,
  0x45c0, 0x0240, 0x02a0, 0x0247, 0x0211, 0x0000, 0x0000, 0x0000,
  0x7fff, 0x1eff, 0x2e5f, 0x223f, 0x79ff, 0x7dd6, 0x7dcc, 0x7e67,
  0x7ae7, 0x4342, 0x2769, 0x2ff3, 0x03bb, 0x0000, 0x0000, 0x0000,
  0x7fff, 0x579f, 0x635f, 0x6b3f, 0x7f1f, 0x7f1b, 0x7ef6, 0x7f75,
  0x7f94, 0x73f4, 0x57d7, 0x5bf9, 0x4ffe, 0x0000, 0x0000, 0x0000
};

// Screen Size Magnification
WORD wScreenMagnification = 1;
#define NES_MENU_HEIGHT     54
#define NES_MENU_WIDTH			8

/*-------------------------------------------------------------------*/
/*  Variables for Emulation Thread                                   */
/*-------------------------------------------------------------------*/
HANDLE m_hThread;
DWORD m_ThreadID = NULL;

/*-------------------------------------------------------------------*/
/*  Variables for Timer & Wait loop                                  */
/*-------------------------------------------------------------------*/
#define LINE_PER_TIMER      789
#define TIMER_PER_LINE      50

WORD wLines;
WORD wLinePerTimer;
MMRESULT uTimerID;
BOOL bWaitFlag;
CRITICAL_SECTION WaitFlagCriticalSection;
BOOL bAutoFrameskip = TRUE;

/*-------------------------------------------------------------------*/
/*  Variables for Sound Emulation                                    */
/*-------------------------------------------------------------------*/
DIRSOUND* lpSndDevice = NULL;

/*-------------------------------------------------------------------*/
/*  Variables for Expiration                                         */
/*-------------------------------------------------------------------*/
#define EXPIRED_YEAR    2001
#define EXPIRED_MONTH   3
#define EXPIRED_MSG     "This software has been expired.\nPlease download newer one."     

/*-------------------------------------------------------------------*/
/*  Function prototypes ( Windows specific )                         */
/*-------------------------------------------------------------------*/

LRESULT CALLBACK MainWndproc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
#if 0
LRESULT CALLBACK AboutDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
#endif
void ShowTitle( HWND hWnd );
void SetWindowSize( WORD wMag );
int LoadSRAM();
int SaveSRAM();
int CreateScreen( HWND hWnd );
void DestroyScreen();
static void InfoNES_StartTimer(); 
static void InfoNES_StopTimer();
static void CALLBACK TimerFunc( UINT nID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

/*===================================================================*/
/*                                                                   */
/*                WinMain() : Application main                       */
/*                                                                   */
/*===================================================================*/
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{	
  /*-------------------------------------------------------------------*/
  /*  Create a window                                                  */
  /*-------------------------------------------------------------------*/

  wc.style = 0;
  wc.lpfnWndProc = MainWndproc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_ICON) );
  wc.hCursor = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
  wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
  wc.lpszClassName = "InfoNESClass";
  if ( !RegisterClass( &wc ) )
    return FALSE;

  hWndMain = CreateWindowEx( 0,
                             "InfoNESClass",
                             APP_NAME,
                             WS_VISIBLE | WS_POPUP | WS_OVERLAPPEDWINDOW,
                             200,
                             120,
                             NES_DISP_WIDTH * wScreenMagnification + NES_MENU_WIDTH,
                             NES_DISP_HEIGHT * wScreenMagnification + NES_MENU_HEIGHT,
                             NULL,
                             NULL,
                             hInstance,
                             NULL );

  if ( !hWndMain )
    return FALSE;

  ShowWindow( hWndMain, nCmdShow );
  UpdateWindow( hWndMain );

#if 0
  /*-------------------------------------------------------------------*/
  /*  Expired or Not?                                                  */
  /*-------------------------------------------------------------------*/
  SYSTEMTIME st;
  GetLocalTime( &st );

  if ( st.wYear > EXPIRED_YEAR || st.wMonth > EXPIRED_MONTH)
  {
    InfoNES_MessageBox( EXPIRED_MSG );
    exit( -1 );
  }
#endif

  /*-------------------------------------------------------------------*/
  /*  Init Resources                                                   */
  /*-------------------------------------------------------------------*/
  InfoNES_StartTimer();
  CreateScreen( hWndMain );
  ShowTitle( hWndMain );

  /*-------------------------------------------------------------------*/
  /*  For Drag and Drop Function                                       */
  /*-------------------------------------------------------------------*/
  if ( lpCmdLine[ 0 ] != '\0' )
  {
    // If included space characters, strip dobule quote marks
    if ( lpCmdLine[ 0 ] == '"' )
    {
      lpCmdLine[ strlen( lpCmdLine ) - 1 ] = '\0';
      lpCmdLine++;
    }

    // Load cassette
    if ( InfoNES_Load( lpCmdLine ) == 0 )
    {
      // Set a ROM image name
      strcpy( szRomName, lpCmdLine );

      // Load SRAM
      LoadSRAM();

	    // Create Emulation Thread
		  m_hThread = CreateThread( (LPSECURITY_ATTRIBUTES)NULL, (DWORD)0,
			  (LPTHREAD_START_ROUTINE)InfoNES_Main, (LPVOID)NULL, (DWORD)0, &m_ThreadID);
    }
  }

  /*-------------------------------------------------------------------*/
  /*  The Message Pump                                                 */
  /*-------------------------------------------------------------------*/
	MSG	msg;

  while (TRUE)
  {
    if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    {
      if (!GetMessage(&msg, NULL, 0, 0 ))
				break;
	  
			// Translate and dispatch the message
			if (0 == TranslateAccelerator(hWndMain, hAccel, &msg))
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg);
			}  
    } 
		else
		{
      // Make sure we go to sleep if we have nothing else to do
      WaitMessage();
		}
  }

  /*-------------------------------------------------------------------*/
  /*  Release Resources                                                */
  /*-------------------------------------------------------------------*/
  if (NULL != m_hThread) {
    // Terminate Emulation Thread
	  TerminateThread(m_hThread, (DWORD)0);  m_hThread=NULL;
		SaveSRAM();
    InfoNES_Fin();
  }
  DestroyScreen();
  InfoNES_StopTimer();
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*                MainWndProc() : Window procedure                   */
/*                                                                   */
/*===================================================================*/
LRESULT CALLBACK MainWndproc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  OPENFILENAME ofn;
  char szFileName[ 256 ];

  switch ( message )
  {
    case WM_ERASEBKGND:
      return 1;

    case WM_DESTROY:
      PostQuitMessage( 0 );
      break;
    
    case WM_ACTIVATE:
      // Show title screen if emulation thread dosent exist
      if ( NULL == m_hThread )
        ShowTitle( hWnd );
      break;

    case WM_COMMAND:
      switch ( LOWORD( wParam ) )
      {
        case IDC_BTN_OPEN:
          /*-------------------------------------------------------------------*/
          /*  Open button                                                      */
          /*-------------------------------------------------------------------*/
					
					// Do nothing if emulation thread exists
					if (NULL != m_hThread)
						break;

          memset( &ofn, 0, sizeof ofn );
          szFileName[ 0 ] = '\0';
          ofn.lStructSize = sizeof ofn;
          ofn.hwndOwner = hWnd;
          ofn.hInstance = wc.hInstance;

          ofn.lpstrFilter = NULL; 
          ofn.lpstrCustomFilter = NULL; 
          ofn.nMaxCustFilter = 0; 
          ofn.nFilterIndex = 0; 
          ofn.lpstrFile = szFileName; 
          ofn.nMaxFile = sizeof szFileName; 
          ofn.lpstrFileTitle = NULL; 
          ofn.nMaxFileTitle = 0; 
          ofn.lpstrInitialDir = NULL;
          ofn.lpstrTitle = NULL; 
          ofn.Flags = 0; 
          ofn.nFileOffset; 
          ofn.nFileExtension = 0; 
          ofn.lpstrDefExt = NULL; 
          ofn.lCustData = 0; 
          ofn.lpfnHook = NULL; 
          ofn.lpTemplateName = NULL; 

          if ( GetOpenFileName( &ofn ) )
          {
            // Load cassette
            if ( InfoNES_Load( szFileName ) == 0 )
            {
              // Set a ROM image name
              strcpy( szRomName, szFileName );

              // Load SRAM
              LoadSRAM();

							// Create Emulation Thread
							m_hThread=CreateThread((LPSECURITY_ATTRIBUTES)NULL, (DWORD)0,
								(LPTHREAD_START_ROUTINE)InfoNES_Main, (LPVOID)NULL, (DWORD)0, &m_ThreadID);
            }
          }
          break;

        case IDC_BTN_STOP:
          /*-------------------------------------------------------------------*/
          /*  Stop button                                                      */
          /*-------------------------------------------------------------------*/

					if (NULL != m_hThread)
					{
					  // Terminate Emulation Thread
						TerminateThread(m_hThread, (DWORD)0);  m_hThread=NULL;
					  SaveSRAM();
            InfoNES_Fin();
            InfoNES_StopTimer();
            DestroyScreen();

            // Preperations
            CreateScreen( hWndMain );
            InfoNES_StartTimer();
          } 
          // Show Title Screen
          ShowTitle( hWnd );
          break;

        case IDC_BTN_RESET:
          /*-------------------------------------------------------------------*/
          /*  Reset button                                                     */
          /*-------------------------------------------------------------------*/
 
					// Do nothing if emulation thread does not exists
					if (NULL != m_hThread)
					{
            // Terminate Emulation Thread
						TerminateThread(m_hThread, (DWORD)0);  m_hThread=NULL;
						SaveSRAM();
            InfoNES_Fin();
            InfoNES_StopTimer();
            DestroyScreen();

            // Create Emulation Thread
            CreateScreen( hWndMain );
            InfoNES_StartTimer();
            InfoNES_Load( szRomName );
            LoadSRAM();
						m_hThread=CreateThread((LPSECURITY_ATTRIBUTES)NULL, (DWORD)0,
							(LPTHREAD_START_ROUTINE)InfoNES_Main, (LPVOID)NULL, (DWORD)0, &m_ThreadID);
          } else {
            // Show Title Screen
            ShowTitle( hWnd );
          }
          break;
        
				case IDC_BTN_EXIT:
          /*-------------------------------------------------------------------*/
          /*  Exit button                                                      */
          /*-------------------------------------------------------------------*/
					if (NULL != m_hThread)
          {
            // Terminate Emulation Thread
						TerminateThread(m_hThread, (DWORD)0);  m_hThread=NULL;
					  SaveSRAM();
            InfoNES_Fin();
            InfoNES_StopTimer();   
            DestroyScreen();
          }
					// Received key/menu command to exit app
          PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;

       case IDC_BTN_FAST:
          /*-------------------------------------------------------------------*/
          /*  Fast & Slow button                                               */
          /*-------------------------------------------------------------------*/
			    // Increment Frameskip Counter
				  FrameSkip++;
				  break;

			  case IDC_BTN_SLOW:
				  // Decrement Frameskip Counter
          if (FrameSkip <= 0)
            break;
				  FrameSkip--;
          break;

        case IDC_BTN_SINGLE:
          /*-------------------------------------------------------------------*/
          /*  Screen Size x1, x2, x3 button                                    */
          /*-------------------------------------------------------------------*/  
          SetWindowSize( 1 );

          // Check x1 button
          CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_SINGLE, MF_CHECKED );
          CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_DOUBLE, MF_UNCHECKED );
          CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_TRIPLE, MF_UNCHECKED );

          // Clear FrameSkip
          FrameSkip = 0;
          break;

        case IDC_BTN_DOUBLE:
          SetWindowSize( 2 );

          // Check x2 button
          CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_SINGLE, MF_UNCHECKED );
          CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_DOUBLE, MF_CHECKED );
          CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_TRIPLE, MF_UNCHECKED );

          // Clear FrameSkip
          FrameSkip = 0;
          break;

        case IDC_BTN_TRIPLE:
          SetWindowSize( 3 );

          // Check x3 button
          CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_SINGLE, MF_UNCHECKED );
          CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_DOUBLE, MF_UNCHECKED );
          CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_TRIPLE, MF_CHECKED );

          // Clear FrameSkip
          FrameSkip = 0;
          break;

        case IDC_BTN_CLIP_VERTICAL:
          /*-------------------------------------------------------------------*/
          /*  Screen Clip Up and Down button                                   */
          /*-------------------------------------------------------------------*/ 
          PPU_UpDown_Clip = ( PPU_UpDown_Clip ? 0 : 1 );          

          // Check or Uncheck it
          if ( PPU_UpDown_Clip )
          {
            CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_CLIP_VERTICAL, MF_CHECKED );
          } else {
            CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_CLIP_VERTICAL, MF_UNCHECKED );          
          }          
          break;

				case IDC_BTN_FRAMESKIP:
					/*-------------------------------------------------------------------*/
          /*  Auto Frameskip button                                            */
          /*-------------------------------------------------------------------*/
          bAutoFrameskip = ( bAutoFrameskip ? 0 : 1 );

          // Check or Uncheck it
          if ( bAutoFrameskip )
          {
            CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_FRAMESKIP, MF_CHECKED );
          } else {
            CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_FRAMESKIP, MF_UNCHECKED );          
          }                      
          break;

				case IDC_BTN_MUTE:
					/*-------------------------------------------------------------------*/
          /*  APU Mute button                                                  */
          /*-------------------------------------------------------------------*/
          APU_Mute = ( APU_Mute ? 0 : 1 );

          if (lpSndDevice != NULL && !lpSndDevice->SoundMute( APU_Mute ) )
          {
            InfoNES_MessageBox( "SoundMute() Failed." );
            exit(0);
          }

          // Check or Uncheck it
          if ( APU_Mute )
          {
            CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_MUTE, MF_CHECKED );
          } else {
            CheckMenuItem( GetMenu( hWndMain ), IDC_BTN_MUTE, MF_UNCHECKED );          
          }                      
          break;

        case IDC_BTN_INFO:
					/*-------------------------------------------------------------------*/
          /*  ROM information                                                  */
          /*-------------------------------------------------------------------*/
          if (NULL != m_hThread) 
          {
            char pszInfo[1024];
            sprintf( pszInfo, "Mapper\t\t%d\nPRG ROM\t\t%dKB\nCHR ROM\t\t%dKB\n" \
                              "Mirroring\t\t%s\nSRAM\t\t%s\n4 Screen\t\t%s\nTrainer\t\t%s",
                              MapperNo, NesHeader.byRomSize * 16, NesHeader.byVRomSize * 8,
                              ( ROM_Mirroring ? "V" : "H" ), ( ROM_SRAM ? "Yes" : "No" ), 
                              ( ROM_FourScr ? "Yes" : "No" ), ( ROM_Trainer ? "Yes" : "No" ) );
            MessageBox( hWndMain, pszInfo, APP_NAME, MB_OK | MB_ICONINFORMATION );              
          } else {
            // Show Title Screen
            ShowTitle( hWnd );
          }
          break;

        case IDC_BTN_ABOUT:
					/*-------------------------------------------------------------------*/
          /*  About button                                                     */
          /*-------------------------------------------------------------------*/
#if 0
          DialogBox(wc.hInstance, 
                    MAKEINTRESOURCE(IDD_DIALOG), 
                    hWndMain, 
                    (DLGPROC)AboutDlgProc);
#else
          {
            /* Version Infomation */
            char pszInfo[1024];
            sprintf( pszInfo, "%s\nA fast and portable NES emulator\n"
			                        "Copyright (C) 1999-2004 Jay's Factory <jays_factory@excite.co.jp>",
			                        APP_NAME );
            MessageBox( hWndMain, pszInfo, APP_NAME, MB_OK | MB_ICONINFORMATION );   
          }
#endif
          break;
      }
			break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

#if 0
/*===================================================================*/
/*                                                                   */
/*           AboutDlgProc() : The About Dialog Box Procedure         */
/*                                                                   */
/*===================================================================*/
LRESULT CALLBACK AboutDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      return TRUE;
    case WM_COMMAND:
      switch ( LOWORD(wParam) )
      {
        case IDOK:
          EndDialog(hDlg, TRUE);
          return TRUE;
      }
      break;
    }
    return FALSE;
}
#endif

/*===================================================================*/
/*                                                                   */
/*      ShowTitle() : Show Title Screen Procedure                    */
/*                                                                   */
/*===================================================================*/
void ShowTitle( HWND hWnd )
{
  HDC hDC = GetDC( hWnd );
  HDC hMemDC = CreateCompatibleDC( hDC );
  HBITMAP hTitleBmp = LoadBitmap( wc.hInstance, MAKEINTRESOURCE( IDB_BITMAP ) );

  // Blt Title Bitmap
  SelectObject( hMemDC, hTitleBmp );

  StretchBlt( hDC, 0, 0, NES_DISP_WIDTH * wScreenMagnification, 
              NES_DISP_HEIGHT * wScreenMagnification, hMemDC, 
              0, 0, NES_DISP_WIDTH, NES_DISP_HEIGHT, SRCCOPY );

  SelectObject( hMemDC, hTitleBmp );

  DeleteDC( hMemDC );
  ReleaseDC( hWnd, hDC );
}

/*===================================================================*/
/*                                                                   */
/*            CreateScreen() : Create InfoNES screen                 */
/*                                                                   */
/*===================================================================*/
int CreateScreen( HWND hWnd )
{
  /*-------------------------------------------------------------------*/
  /*  Create a InfoNES screen                                          */
  /*-------------------------------------------------------------------*/
  HDC hDC = GetDC( hWnd );

  BITMAPINFOHEADER bi;

  bi.biSize = sizeof( BITMAPINFOHEADER );
  bi.biWidth = NES_DISP_WIDTH;
  bi.biHeight = NES_DISP_HEIGHT * -1;
  bi.biPlanes = 1;
  bi.biBitCount = 16;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = NES_DISP_WIDTH * NES_DISP_HEIGHT * 2;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;

  hScreenBmp = CreateDIBSection( hDC, 
                                 (BITMAPINFO *)&bi,
                                 DIB_RGB_COLORS, 
                                 (void **)&pScreenMem, 
                                 0,
                                 0 ); 
  ReleaseDC( hWnd, hDC );

  if ( !hScreenBmp ) { return -1; } 
  else {  return 0; }
}

/*===================================================================*/
/*                                                                   */
/*          DestroyScreen() : Destroy InfoNES screen                 */
/*                                                                   */
/*===================================================================*/
void DestroyScreen()
{
  if ( !hScreenBmp ) { DeleteObject( hScreenBmp ); }
}

/*===================================================================*/
/*                                                                   */
/*           LoadSRAM() : Load a SRAM                                */
/*                                                                   */
/*===================================================================*/
int LoadSRAM()
{
/*
 *  Load a SRAM
 *
 *  Return values
 *     0 : Normally
 *    -1 : SRAM data couldn't be read
 */

  FILE *fp;
  unsigned char pSrcBuf[ SRAM_SIZE ];
  unsigned char chData;
  unsigned char chTag;
  int nRunLen;
  int nDecoded;
  int nDecLen;
  int nIdx;

  // It doesn't need to save it
  nSRAM_SaveFlag = 0;

  // It is finished if the ROM doesn't have SRAM
  if ( !ROM_SRAM )
    return 0;

  // There is necessity to save it
  nSRAM_SaveFlag = 1;

  // The preparation of the SRAM file name
  strcpy( szSaveName, szRomName );
  strcpy( strrchr( szSaveName, '.' ) + 1, "srm" );

  /*-------------------------------------------------------------------*/
  /*  Read a SRAM data                                                 */
  /*-------------------------------------------------------------------*/

  // Open SRAM file
  fp = fopen( szSaveName, "rb" );
  if ( fp == NULL )
    return -1;

  // Read SRAM data
  fread( pSrcBuf, SRAM_SIZE, 1, fp );

  // Close SRAM file
  fclose( fp );

  /*-------------------------------------------------------------------*/
  /*  Extract a SRAM data                                              */
  /*-------------------------------------------------------------------*/

  nDecoded = 0;
  nDecLen = 0;

  chTag = pSrcBuf[ nDecoded++ ];

  while ( nDecLen < 8192 )
  {
    chData = pSrcBuf[ nDecoded++ ];

    if ( chData == chTag )
    {
      chData = pSrcBuf[ nDecoded++ ];
      nRunLen = pSrcBuf[ nDecoded++ ];
      for ( nIdx = 0; nIdx < nRunLen + 1; ++nIdx )
      {
        SRAM[ nDecLen++ ] = chData;
      }
    }
    else
    {
      SRAM[ nDecLen++ ] = chData;
    }
  }

  // Successful
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*           SaveSRAM() : Save a SRAM                                */
/*                                                                   */
/*===================================================================*/
int SaveSRAM()
{
/*
 *  Save a SRAM
 *
 *  Return values
 *     0 : Normally
 *    -1 : SRAM data couldn't be written
 */

  FILE *fp;
  int nUsedTable[ 256 ];
  unsigned char chData;
  unsigned char chPrevData;
  unsigned char chTag;
  int nIdx;
  int nEncoded;
  int nEncLen;
  int nRunLen;
  unsigned char pDstBuf[ SRAM_SIZE ];

  if ( !nSRAM_SaveFlag )
    return 0;  // It doesn't need to save it

  /*-------------------------------------------------------------------*/
  /*  Compress a SRAM data                                             */
  /*-------------------------------------------------------------------*/

  memset( nUsedTable, 0, sizeof nUsedTable );

  for ( nIdx = 0; nIdx < SRAM_SIZE; ++nIdx )
  {
    ++nUsedTable[ SRAM[ nIdx++ ] ];
  }
  for ( nIdx = 1, chTag = 0; nIdx < 256; ++nIdx )
  {
    if ( nUsedTable[ nIdx ] < nUsedTable[ chTag ] )
      chTag = nIdx;
  }

  nEncoded = 0;
  nEncLen = 0;
  nRunLen = 1;

  pDstBuf[ nEncLen++ ] = chTag;

  chPrevData = SRAM[ nEncoded++ ];

  while ( nEncoded < SRAM_SIZE && nEncLen < SRAM_SIZE - 133 )
  {
    chData = SRAM[ nEncoded++ ];

    if ( chPrevData == chData && nRunLen < 256 )
      ++nRunLen;
    else
    {
      if ( nRunLen >= 4 || chPrevData == chTag )
      {
        pDstBuf[ nEncLen++ ] = chTag;
        pDstBuf[ nEncLen++ ] = chPrevData;
        pDstBuf[ nEncLen++ ] = nRunLen - 1;
      }
      else
      {
        for ( nIdx = 0; nIdx < nRunLen; ++nIdx )
          pDstBuf[ nEncLen++ ] = chPrevData;
      }

      chPrevData = chData;
      nRunLen = 1;
    }

  }
  if ( nRunLen >= 4 || chPrevData == chTag )
  {
    pDstBuf[ nEncLen++ ] = chTag;
    pDstBuf[ nEncLen++ ] = chPrevData;
    pDstBuf[ nEncLen++ ] = nRunLen - 1;
  }
  else
  {
    for ( nIdx = 0; nIdx < nRunLen; ++nIdx )
      pDstBuf[ nEncLen++ ] = chPrevData;
  }

  /*-------------------------------------------------------------------*/
  /*  Write a SRAM data                                                */
  /*-------------------------------------------------------------------*/

  // Open SRAM file
  fp = fopen( szSaveName, "wb" );
  if ( fp == NULL )
    return -1;

  // Write SRAM data
  fwrite( pDstBuf, nEncLen, 1, fp );

  // Close SRAM file
  fclose( fp );

  // Successful
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*                  InfoNES_Menu() : Menu screen                     */
/*                                                                   */
/*===================================================================*/
int InfoNES_Menu()
{
/*
 *  Menu screen
 *
 *  Return values
 *     0 : Normally
 *    -1 : Exit InfoNES
 */

	// Nothing to do here
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*               InfoNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*===================================================================*/
int InfoNES_ReadRom( const char *pszFileName )
{
/*
 *  Read ROM image file
 *
 *  Parameters
 *    const char *pszFileName          (Read)
 *
 *  Return values
 *     0 : Normally
 *    -1 : Error
 */

  FILE *fp;

  /* Open ROM file */
  fp = fopen( pszFileName, "rb" );
  if ( fp == NULL )
    return -1;

  /* Read ROM Header */
  fread( &NesHeader, sizeof NesHeader, 1, fp );
  if ( memcmp( NesHeader.byID, "NES\x1a", 4 ) != 0 )
  {
    /* not .nes file */
    fclose( fp );
    return -1;
  }

  /* Clear SRAM */
  memset( SRAM, 0, SRAM_SIZE );

  /* If trainer presents Read Triner at 0x7000-0x71ff */
  if ( NesHeader.byInfo1 & 4 )
  {
    fread( &SRAM[ 0x1000 ], 512, 1, fp );
  }

  /* Allocate Memory for ROM Image */
  ROM = (BYTE *)malloc( NesHeader.byRomSize * 0x4000 );

  /* Read ROM Image */
  fread( ROM, 0x4000, NesHeader.byRomSize, fp );

  if ( NesHeader.byVRomSize > 0 )
  {
    /* Allocate Memory for VROM Image */
    VROM = (BYTE *)malloc( NesHeader.byVRomSize * 0x2000 );

    /* Read VROM Image */
    fread( VROM, 0x2000, NesHeader.byVRomSize, fp );
  }

  /* File close */
  fclose( fp );

  /* Successful */
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*           InfoNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                   */
/*===================================================================*/
void InfoNES_ReleaseRom()
{
/*
 *  Release a memory for ROM
 *
 */

  if ( ROM )
  {
    free( ROM );
    ROM = NULL;
  }

  if ( VROM )
  {
    free( VROM );
    VROM = NULL;
  }
}

/*===================================================================*/
/*                                                                   */
/*      InfoNES_LoadFrame() :                                        */
/*           Transfer the contents of work frame on the screen       */
/*                                                                   */
/*===================================================================*/
void InfoNES_LoadFrame()
{
/*
 *  Transfer the contents of work frame on the screen
 *
 */

  // Set screen data
  memcpy( pScreenMem, WorkFrame, NES_DISP_WIDTH * NES_DISP_HEIGHT * 2 );

  // Screen update
  HDC hDC = GetDC( hWndMain );

  HDC hMemDC = CreateCompatibleDC( hDC );

  HBITMAP hOldBmp = (HBITMAP)SelectObject( hMemDC, hScreenBmp );

  StretchBlt( hDC, 0, 0, NES_DISP_WIDTH * wScreenMagnification, 
              NES_DISP_HEIGHT * wScreenMagnification, hMemDC, 
              0, 0, NES_DISP_WIDTH, NES_DISP_HEIGHT, SRCCOPY );

  SelectObject( hMemDC, hOldBmp );

  DeleteDC( hMemDC );
  ReleaseDC( hWndMain, hDC );
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_PadState() : Get a joypad state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_PadState( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem )
{
/*
 *  Get a joypad state
 *
 *  Parameters
 *    DWORD *pdwPad1                   (Write)
 *      Joypad 1 State
 *
 *    DWORD *pdwPad2                   (Write)
 *      Joypad 2 State
 *
 *    DWORD *pdwSystem                 (Write)
 *      Input for InfoNES
 *
 */

  static DWORD dwSysOld;
  DWORD dwTemp;

  /* Joypad 1 */
  *pdwPad1 =   ( GetAsyncKeyState( 'X' )        < 0 ) |
             ( ( GetAsyncKeyState( 'Z' )        < 0 ) << 1 ) |
             ( ( GetAsyncKeyState( 'A' )        < 0 ) << 2 ) |
             ( ( GetAsyncKeyState( 'S' )        < 0 ) << 3 ) |
             ( ( GetAsyncKeyState( VK_UP )      < 0 ) << 4 ) |
             ( ( GetAsyncKeyState( VK_DOWN )    < 0 ) << 5 ) |
             ( ( GetAsyncKeyState( VK_LEFT )    < 0 ) << 6 ) |
             ( ( GetAsyncKeyState( VK_RIGHT )   < 0 ) << 7 );

  *pdwPad1 = *pdwPad1 | ( *pdwPad1 << 8 );

  /* Joypad 2 */
  *pdwPad2 = 0;

  /* Input for InfoNES */
  dwTemp = ( GetAsyncKeyState( VK_ESCAPE )  < 0 );
  
  /* Only the button pushed newly should be inputted */
  *pdwSystem = ~dwSysOld & dwTemp;
  
  /* keep this input */
  dwSysOld = dwTemp;

  /* Deal with a message */
  MSG msg;
  while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
  {
    if ( GetMessage( &msg, NULL, 0, 0 ) )
    {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }
  }
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemoryCopy() : memcpy                         */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemoryCopy( void *dest, const void *src, int count )
{
/*
 *  memcpy
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the copied blockfs destination
 *
 *    const void *src                  (Read)
 *      Points to the starting address of the block of memory to copy
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to copy
 *
 *  Return values
 *    Pointer of destination
 */

  CopyMemory( dest, src, count );
  return dest;
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemorySet() : Get a joypad state              */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemorySet( void *dest, int c, int count )
{
/*
 *  memset
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the block of memory to fill
 *
 *    int c                            (Read)
 *      Specifies the byte value with which to fill the memory block
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to fill
 *
 *  Return values
 *    Pointer of destination
 */

  FillMemory( dest, count, c ); 
  return dest;
}

/*===================================================================*/
/*                                                                   */
/*                DebugPrint() : Print debug message                 */
/*                                                                   */
/*===================================================================*/
void InfoNES_DebugPrint( char *pszMsg )
{
  _RPT0( _CRT_WARN, pszMsg );
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundInit() : Sound Emulation Initialize           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundInit( void ) {}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen( int samples_per_sync, int sample_rate ) 
{
  lpSndDevice = new DIRSOUND( hWndMain );

  if ( !lpSndDevice->SoundOpen( samples_per_sync, sample_rate ) )
  {
    InfoNES_MessageBox( "SoundOpen() Failed." );
    exit(0);
  }

  // if sound mute, stop sound
  if ( APU_Mute )
  {
    if (!lpSndDevice->SoundMute( APU_Mute ) )
    {
      InfoNES_MessageBox( "SoundMute() Failed." );
      exit(0);
    }
  }

  return(TRUE);
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundClose( void ) 
{
  lpSndDevice->SoundClose();
  delete lpSndDevice;
	lpSndDevice = NULL;
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SoundOutput() : Sound Output Waves             */           
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundOutput( int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5 ) 
{
  BYTE wave[ rec_freq ];
  
  for ( int i = 0; i < rec_freq; i++)
  {
    wave[i] = ( wave1[i] + wave2[i] + wave3[i] + wave4[i] + wave5[i] ) / 5;
  }
#if 1
  if (!lpSndDevice->SoundOutput( samples, wave ) )
#else
  if (!lpSndDevice->SoundOutput( samples, wave3 ) )
#endif
  {
    InfoNES_MessageBox( "SoundOutput() Failed." );
    exit(0);
  }
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_StartTimer() : Start MM Timer                  */           
/*                                                                   */
/*===================================================================*/
static void InfoNES_StartTimer()
{
  TIMECAPS caps;

  timeGetDevCaps( &caps, sizeof(caps) );
  timeBeginPeriod( caps.wPeriodMin );

  uTimerID = 
    timeSetEvent( caps.wPeriodMin * TIMER_PER_LINE, caps.wPeriodMin, TimerFunc, 0, (UINT)TIME_PERIODIC );

  // Calculate proper timing
  wLinePerTimer = LINE_PER_TIMER * caps.wPeriodMin;

  // Initialize timer variables
  wLines = 0;
  bWaitFlag = TRUE;

  // Initialize Critical Section Object
  InitializeCriticalSection( &WaitFlagCriticalSection );
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_StopTimer() : Stop MM Timer                    */           
/*                                                                   */
/*===================================================================*/
static void InfoNES_StopTimer()
{
  if ( 0 != uTimerID )
  {
    TIMECAPS caps;
    timeKillEvent( uTimerID );
    uTimerID = 0;
    timeGetDevCaps( &caps, sizeof(caps) );
    timeEndPeriod( caps.wPeriodMin * TIMER_PER_LINE );
  }
  // Delete Critical Section Object
  DeleteCriticalSection( &WaitFlagCriticalSection );
}

/*===================================================================*/
/*                                                                   */
/*           TimerProc() : MM Timer Callback Function                */
/*                                                                   */
/*===================================================================*/
static void CALLBACK TimerFunc( UINT nID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
  if ( NULL != m_hThread )
  {  
    EnterCriticalSection( &WaitFlagCriticalSection );
    bWaitFlag = FALSE;
    LeaveCriticalSection( &WaitFlagCriticalSection );
  }
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_Wait() : Wait Emulation if required            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Wait()
{
  wLines++;
  if ( wLines < wLinePerTimer )
    return;
  wLines = 0;

  if ( bAutoFrameskip )
  {
    // Auto Frameskipping
    if ( !bWaitFlag )
    {
      // Increment Frameskip Counter
	    FrameSkip++;
    } 
#if 1
    else {
      // Decrement Frameskip Counter
      if ( FrameSkip > 2 )
      {
		    FrameSkip--;  
      }
    }
#endif
  }

  // Wait if bWaitFlag is TRUE
  if ( bAutoFrameskip )
  {
    while ( bWaitFlag )
      Sleep(0);
  }

  // set bWaitFlag is TRUE
  EnterCriticalSection( &WaitFlagCriticalSection );
  bWaitFlag = TRUE;
  LeaveCriticalSection( &WaitFlagCriticalSection );
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_MessageBox() : Print System Message            */
/*                                                                   */
/*===================================================================*/
void InfoNES_MessageBox( char *pszMsg, ... )
{
  char pszErr[ 1024 ];
  va_list args;

  va_start( args, pszMsg );
  vsprintf( pszErr, pszMsg, args );  pszErr[ 1023 ] = '\0';
  va_end( args );
  MessageBox( hWndMain, pszErr, APP_NAME, MB_OK | MB_ICONSTOP );
}

/*===================================================================*/
/*                                                                   */
/*            SetWindowSize() : Set Window Size                      */
/*                                                                   */
/*===================================================================*/
void SetWindowSize( WORD wMag )
{          
  wScreenMagnification = wMag;

  SetWindowPos( hWndMain, HWND_TOPMOST, 0, 0, 
								NES_DISP_WIDTH * wMag + NES_MENU_WIDTH, 
                NES_DISP_HEIGHT * wMag + NES_MENU_HEIGHT, SWP_NOMOVE );
          
  // Show title screen if emulation thread dosent exist
  if ( NULL == m_hThread )
    ShowTitle( hWndMain );
}
