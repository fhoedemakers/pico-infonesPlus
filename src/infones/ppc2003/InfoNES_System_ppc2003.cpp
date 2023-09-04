/*===================================================================*/
/*                                                                   */
/*  InfoNES_System_ppc2003.cpp : WindowsCE specific File             */
/*                                                                   */
/*  2001/12/08  Mopeo                                                */
/*  2001/12/20  Y.Nagamidori                                         */
/*  2003/06/10  Jay (again:)                                         */
/*                                                                   */
/*  This file is originated from InfoNES_System_Wce.cpp              */
/*===================================================================*/

int g_nKeyConfig[9];

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include <windows.h>
#include <aygshell.h>
#include <Commctrl.h>
#include <Commdlg.h>
#include <stdlib.h>
#include <stdio.h>

#include "gx.h"
#pragma comment(lib, "gx.lib")

#include "../InfoNES.h"
#include "../InfoNES_System.h"
#include "../InfoNES_pAPU.h"
#include "resource.h"

#define MENU_HEIGHT 26
#define SCREEN_TOP 26

#define WM_SUSPEND (WM_USER + 1024)

/*-------------------------------------------------------------------*/
/*  ROM image file information                                       */
/*-------------------------------------------------------------------*/
#define SRAM_FILE_EXT "srm"

char	g_szRomName[ MAX_PATH ] = "";
char	g_szSaveName[ MAX_PATH ];
int		g_nSRAM_SaveFlag;

/*-------------------------------------------------------------------*/
/*  Global Variables                                                 */
/*-------------------------------------------------------------------*/

#define VERSION  "InfoNES v0.93J"
const TCHAR c_szAppName[] = TEXT("InfoNES Pocket PC 2003 Application");
const TCHAR c_szTitle[]   = TEXT("InfoNES");

HWND		g_hwnd;
HWND		g_hwndCB;
HINSTANCE	g_hInst;
GXDisplayProperties g_gxdp;
HANDLE		g_hThread = NULL;
DWORD		g_dwThreadID;
BOOL		g_bThreadRun;
BOOL		g_bExitThread = FALSE;

/*-------------------------------------------------------------------*/
/*  Sound Output Variables                                           */
/*-------------------------------------------------------------------*/
HWAVEOUT	g_hwo = NULL;
HANDLE		g_hSem = NULL;
WAVEFORMATEX g_wfx;
int			g_nCurrent;

#define SOUND_BUFS		8
WAVEHDR		g_Hdr[SOUND_BUFS];
LPBYTE		g_pbBuff;

int g_nSamplesPerSync = 0;
int g_nSampleRate = 0;

BOOL g_bUseSound = TRUE;

/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

LRESULT CALLBACK MainWndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
int LoadSRAM();
int SaveSRAM();
void RegLoad();
void RegSave();
void ShowMessage( char *pszMsg, ... );

/* Palette data */
#if 0
WORD NesPalette[ 64 ];

DWORD NesPalette24[64] = { /* Matthew Conte's Palette */
	0x808080, 0x0000bb, 0x3700bf, 0x8400a6,
	0xbb006a, 0xb7001e, 0xb30000, 0x912600,
	0x7b2b00, 0x003e00, 0x00480d, 0x003c22,
	0x002f66, 0x000000, 0x050505, 0x050505,

	0xc8c8c8, 0x0059ff, 0x443cff, 0xb733cc,
	0xff33aa, 0xff375e, 0xff371a, 0xd54b00,
	0xc46200, 0x3c7b00, 0x1e8415, 0x009566,
	0x0084c4, 0x111111, 0x090909, 0x090909,

	0xffffff, 0x0095ff, 0x6f84ff, 0xd56fff,
	0xff77cc, 0xff6f99, 0xff7b59, 0xff915f,
	0xffa233, 0xa6bf00, 0x51d96a, 0x4dd5ae,
	0x00d9ff, 0x666666, 0x0d0d0d, 0x0d0d0d,

	0xffffff, 0x84bfff, 0xbbbbff, 0xd0bbff,
	0xffbfea, 0xffbfcc, 0xffc4b7, 0xffccae,
	0xffd9a2, 0xcce199, 0xaeeeb7, 0xaaf7ee,
	0xb3eeff, 0xdddddd, 0x111111, 0x111111
};
#else
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
#endif


/*===================================================================*/
/*                                                                   */
/*									   emulation thread					                     */
/*                                                                   */
/*===================================================================*/
DWORD WINAPI InfoNesThreadProc(LPVOID pParam)
{
	InfoNES_Main();
	ExitThread(0);
	return 0;
}

/*===================================================================*/
/*                                                                   */
/*								 sound	helper functions			                     */
/*                                                                   */
/*===================================================================*/
static void CALLBACK WaveOutCallback(HANDLE hwo, UINT uMsg,
								  DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (uMsg == WOM_DONE)
		ReleaseSemaphore((HANDLE)dwUser, 1, NULL);
}

BOOL waveout_open()
{
	if (g_hSem)
		CloseHandle(g_hSem);

	g_hSem = CreateSemaphore(NULL, SOUND_BUFS, SOUND_BUFS, NULL);

	// open waveout device
	g_wfx.wFormatTag = WAVE_FORMAT_PCM;
	g_wfx.nChannels = 1;
	g_wfx.nSamplesPerSec = g_nSampleRate;
	g_wfx.wBitsPerSample = 8; 
	g_wfx.nBlockAlign = g_wfx.nChannels * g_wfx.wBitsPerSample / 8; 
	g_wfx.nAvgBytesPerSec = g_wfx.nSamplesPerSec * g_wfx.nBlockAlign;
	g_wfx.cbSize = sizeof(WAVEFORMATEX);
	if (waveOutOpen(&g_hwo, WAVE_MAPPER, &g_wfx,
					(DWORD) WaveOutCallback, (DWORD)g_hSem, CALLBACK_FUNCTION)
		!= MMSYSERR_NOERROR)
		return FALSE; // no sound;

	// prepare bufffer
	g_pbBuff = new BYTE[g_nSamplesPerSync * SOUND_BUFS];
	
	for (int i = 0; i < SOUND_BUFS; i++) {
		memset(&g_Hdr[i], 0, sizeof(WAVEFORMATEX));
		g_Hdr[i].lpData = (LPSTR)(g_pbBuff + g_nSamplesPerSync * i);
		g_Hdr[i].dwBufferLength = g_nSamplesPerSync;
		waveOutPrepareHeader(g_hwo, &g_Hdr[i], sizeof(WAVEHDR));
	}
	g_nCurrent = 0;
	return TRUE;
}

void waveout_close()
{
	if (g_hwo)
	{
		waveOutClose(g_hwo);

		for (int i = 0; i < SOUND_BUFS; i++)
			waveOutUnprepareHeader(g_hwo, &g_Hdr[i], sizeof WAVEHDR);
		g_hwo = NULL;
	}

	if (g_pbBuff) 
	{
		delete [] g_pbBuff;
		g_pbBuff = NULL;
	}

	if (g_hSem) {
		CloseHandle(g_hSem);
		g_hSem = NULL;
	}
}

/*===================================================================*/
/*                                                                   */
/*									   emulation start/stop helper fuctions					 */
/*                                                                   */
/*===================================================================*/

BOOL Start()
{
	if (InfoNES_Load(g_szRomName) != 0){
		MessageBox(g_hwnd, TEXT("未対応のファイルです"), c_szTitle, MB_ICONEXCLAMATION);
		return FALSE;
	}
	LoadSRAM();
	RegLoad();

	g_bThreadRun = TRUE;
	g_hThread = CreateThread(NULL, 0, InfoNesThreadProc, NULL, 0, &g_dwThreadID);
	return TRUE;
}

void Pause()
{
	if (g_bThreadRun == TRUE){
		g_bThreadRun = FALSE;
		SuspendThread(g_hThread);
		GXSuspend();
		GXCloseInput();
	}
}

void Resume()
{
	GXOpenInput();
	GXResume();

	g_bThreadRun = TRUE;
	ResumeThread(g_hThread);
}

void Stop()
{
	if (g_hThread) {	
		GXOpenInput();
		GXResume();

		g_bExitThread = TRUE;
		ResumeThread(g_hThread);
		WaitForSingleObject(g_hThread, INFINITE);
		CloseHandle(g_hThread);
		g_hThread = NULL;
		g_bExitThread = FALSE;
	}
	SaveSRAM();
	RegSave();
}

/*===================================================================*/
/*                                                                   */
/*										  registory save / load												 */
/*                                                                   */
/*===================================================================*/
#define REG_KEY_INFONES _T("Software\\InfoNES2003")
const TCHAR szRegName[3][32] = {
	_T("FrameSkip"),
	_T("Sound"),
	_T("Clip")
};

#define REG_KEY_INFONES_KEY _T("Software\\InfoNES2003\\Key")
const TCHAR szRegNameKey[9][32] = {
	_T("A"),
	_T("B"),
	_T("Select"),
	_T("Start"),
	_T("Up"),
	_T("Down"),
	_T("Left"),
	_T("Right"),
	_T("Exit")
};

void RegLoad()
{
  HKEY hKey;
  DWORD dwType, dwData, dwSize;
  dwSize = sizeof(DWORD);

  if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_INFONES, 0, 
		   KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
    if (RegQueryValueEx(hKey, szRegName[0], 0, &dwType, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
      FrameSkip = LOWORD(dwData);
    if (RegQueryValueEx(hKey, szRegName[1], 0, &dwType, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
      g_bUseSound = (BOOL)dwData;
    if (RegQueryValueEx(hKey, szRegName[2], 0, &dwType, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
      PPU_UpDown_Clip = (BYTE)dwData;
    RegCloseKey(hKey);
  }

  if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_INFONES_KEY, 0, 
		   KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
    dwSize = sizeof(DWORD);
    for (int i = 0; i < 9; i++) {
      if (RegQueryValueEx(hKey, szRegNameKey[i], 0, &dwType, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
	g_nKeyConfig[i] = (BOOL)dwData;
    }
    RegCloseKey(hKey);
  }
}

void RegSave()
{
  HKEY hKey;
  DWORD dwDisposition, dwData;
  if (RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_INFONES, 0, _T(""), REG_OPTION_NON_VOLATILE, 
		     KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)	{

    dwData = (DWORD)FrameSkip;
    RegSetValueEx(hKey, szRegName[0], 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD));
    dwData = g_bUseSound;
    RegSetValueEx(hKey, szRegName[1], 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD));
    dwData = (DWORD)PPU_UpDown_Clip;
    RegSetValueEx(hKey, szRegName[2], 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD));

    RegCloseKey(hKey);
  }

  if (RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_INFONES_KEY, 0, _T(""), REG_OPTION_NON_VOLATILE, 
		     KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)	{
    
    for (int i = 0; i < 9; i++) {
      dwData = g_nKeyConfig[i];
      RegSetValueEx(hKey, szRegNameKey[i], 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD));
    }
    RegCloseKey(hKey);
  }
}

/*===================================================================*/
/*                                                                   */
/*						     window helper fuctions														 */
/*                                                                   */
/*===================================================================*/
HWND CreateMenuBar(HWND hwnd)
{
	SHMENUBARINFO mbi;

	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize     = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hwnd;
	mbi.nToolBarId = IDM_MENU;
	mbi.hInstRes   = g_hInst;
	mbi.dwFlags = SHCMBF_HIDESIPBUTTON;
	mbi.nBmpId     = 0;
	mbi.cBmpImages = 0;

	if (!SHCreateMenuBar(&mbi))
		return NULL;

	return mbi.hwndMB;
}

BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
	g_hInst = hInst;

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)MainWndproc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = 0;
	wc.hInstance = g_hInst;
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = c_szAppName;
	if (!RegisterClass(&wc))
		return FALSE;

	g_hwnd = CreateWindow(c_szAppName,
					c_szTitle,
					WS_VISIBLE,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					NULL, NULL, hInst, NULL);
	if (!g_hwnd)
		return FALSE;

	RECT rc;
	GetWindowRect(g_hwnd, &rc);
	rc.bottom -= MENU_HEIGHT;
	if (g_hwndCB)
		MoveWindow(g_hwnd, rc.left, rc.top, rc.right, rc.bottom, FALSE);

	ShowWindow(g_hwnd, nCmdShow);
	UpdateWindow(g_hwnd);

	return TRUE;
}

BOOL InitInfoNES()
{
	OPENFILENAME ofn;
	TCHAR szFileName[MAX_PATH];

	/* ファイルダイアログを開く */
	if (!strlen(g_szRomName)) {
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = TEXT("NES files (*.nes)\0*.nes\0All files (*.*)\0*.*\0");
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.hwndOwner = g_hwnd;
		ofn.Flags = OFN_EXPLORER;
		if (!GetOpenFileName(&ofn))
			return FALSE;
		wcstombs(g_szRomName, szFileName, MAX_PATH); /* UnicodeからANSIへ */
	}

	/* GAPI初期化 */
	if(!GXOpenDisplay(g_hwnd, GX_FULLSCREEN)){
		MessageBox(g_hwnd, TEXT("GXOpenDisplayエラー"), c_szTitle, MB_ICONEXCLAMATION);
		return FALSE;
	}
	g_gxdp = GXGetDisplayProperties();
	if(g_gxdp.cxWidth != 240 || g_gxdp.cyHeight != 320){
		MessageBox(g_hwnd, TEXT("使用できない画面モードです"), c_szTitle, MB_ICONEXCLAMATION);
		return FALSE;
	}
	if(g_gxdp.cBPP != 16 || !(g_gxdp.ffFormat & kfDirect565)){
		MessageBox(g_hwnd, TEXT("使用できない画面モードです"), c_szTitle, MB_ICONEXCLAMATION);
		return FALSE;
	}
	if (!GXOpenInput()){
		MessageBox(g_hwnd, TEXT("GXOpenInputエラー"), c_szTitle, MB_ICONEXCLAMATION);
		return FALSE;
	}

	// get default key configuration
	GXKeyList gxkl;
	gxkl = GXGetDefaultKeys(GX_NORMALKEYS);
	g_nKeyConfig[0] = gxkl.vkA;		// A
	g_nKeyConfig[1] = gxkl.vkB;		// B
	g_nKeyConfig[2] = gxkl.vkStart;	// Select
	g_nKeyConfig[3] = gxkl.vkC;		// Start
	g_nKeyConfig[4] = gxkl.vkUp;	// Up
	g_nKeyConfig[5] = gxkl.vkDown;	// Down
	g_nKeyConfig[6] = gxkl.vkLeft;	// Left
	g_nKeyConfig[7] = gxkl.vkRight;	// Right
	g_nKeyConfig[8] = 0;			// Exit (Pause)

#if 0
	/* パレットのセット */
	int i;
	BYTE r, g, b;
	DWORD temp;

	for(i = 0; i < 64; i++){
		temp = NesPalette24[i];
		r = (BYTE)(temp >> 16);
		g = (BYTE)(((WORD)(temp)) >> 8);
		b = (BYTE)(temp);

		NesPalette[i] = (((WORD)r << 8) & 0xf800) | 
		(((WORD)g << 3) & 0x0fe0) | 
		((WORD)b >> 3); /* 5-6-5へ */
	}
#endif

	// read registory
	RegLoad();

	return TRUE;
}

int press_key;
BOOL CALLBACK KeyPressDlg(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_COMMAND:
			if (LOWORD(wParam) == IDCANCEL) {
				press_key = 0;
				EndDialog(hwndDlg, IDCANCEL);
			}
			return TRUE;
		case WM_KEYDOWN:
		{
			if (wParam == 0x5B) {
				for (int i = 0xC1; i < 0xC6; i++) {
					if (GetAsyncKeyState(i)) {
						press_key = i;
						EndDialog(hwndDlg, IDOK);
						return TRUE;
					}
				}
			} else if (wParam == 0xE5) {
				for (int i = 0x25; i < 0x29; i++) {
					if (GetAsyncKeyState(i)) {
						press_key = i;
						EndDialog(hwndDlg, IDOK);
						return TRUE;
					}
				}
			}
		}
#if 0
		case WM_KEYUP:
			press_key = wParam;
			EndDialog(hwndDlg, IDOK);
			return TRUE;
#endif
		default: return FALSE;
	}
}

int StartKeyDiaglog(HWND hwndParent)
{
	GXOpenInput();
	DialogBox(g_hInst, (LPCTSTR)IDD_KEY_PRESS, hwndParent, KeyPressDlg);
	GXCloseInput();
	return press_key;
}

BOOL CALLBACK KeyConfigDlg(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int key[9];

	switch (uMsg) {
		case WM_INITDIALOG:
		{
			SHINITDLGINFO shidi;
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
			shidi.hDlg = hwndDlg;
			SHInitDialog(&shidi);

			for (int i = 0; i < 9; i++) {
				TCHAR sz[32];
				wsprintf(sz, _T("%s :\t0x%X"), szRegNameKey[i], g_nKeyConfig[i]);
				SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC_KEY_A + i), sz);
				key[i] = g_nKeyConfig[i];
			}
			break;
		}
		case WM_COMMAND:
		{
			WORD wID = LOWORD(wParam);
			if (wID == IDOK) {
				for (int i = 0; i < 9; i++)
					g_nKeyConfig[i] = key[i];
				EndDialog(hwndDlg, LOWORD(wParam));
				return TRUE;
			}
			else if (wID == IDCANCEL) {
				EndDialog(hwndDlg, LOWORD(wParam));
				return TRUE;
			}
			else if (wID >= IDC_A && wID <= IDC_EXIT) {
				int n = StartKeyDiaglog(hwndDlg);
				if (n) {
					TCHAR sz[32];
					int nIndex = wID - IDC_A;
					wsprintf(sz, _T("%s :\t0x%X"), szRegNameKey[nIndex], n);
					SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC_KEY_A + nIndex), sz);
					key[nIndex] = n;
				}
				return TRUE;
			}
			else if (wID >= IDC_DEL_A && wID <= IDC_DEL_EXIT) {
				TCHAR sz[32]; 
				int nIndex = wID - IDC_DEL_A;
				wsprintf(sz, _T("%s :\t0x%X"), szRegNameKey[nIndex], 0);
				SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC_KEY_A + nIndex), sz);
				key[nIndex] = 0;
				return TRUE;
			}
			break;
		}
	}
	return FALSE;
}

/*===================================================================*/
/*                                                                   */
/*                WinMain() : Application main                       */
/*                                                                   */
/*===================================================================*/

/* Application main */
int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPWSTR lpCmdLine,
				   int nCmdShow)
{
	MSG msg;
	HWND hwndPrev = NULL;

	/* すでに実行されているかチェック */
	hwndPrev = FindWindow(c_szAppName, c_szTitle);
	if (hwndPrev){
		SetForegroundWindow(hwndPrev);
		return 0;
	}

	if (_tcslen(lpCmdLine))
		wcstombs(g_szRomName, lpCmdLine, MAX_PATH);

	if (!InitInstance(hInstance, nCmdShow))
		return 0;
	
	if (!InitInfoNES())
		goto error;

	/* エミュレーションの開始 */
	if (!Start())
		goto error;

	/* メッセージループ */
	while(GetMessage(&msg, NULL, 0, 0) == TRUE){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;

error:
	GXCloseDisplay();
	GXCloseInput();
	return 0;
}

/*===================================================================*/
/*                                                                   */
/*            MainWndproc() : ウィンドウプロシージャ関数             */
/*                                                                   */
/*===================================================================*/

LRESULT CALLBACK MainWndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  switch(msg){
  case WM_COMMAND:
    switch(LOWORD(wp)){
    case IDM_MAIN_END:
      Stop();
      PostMessage(hwnd, WM_CLOSE, 0, 0);
      break;
    case IDM_MAIN_RESUME:
      Resume();
      break;
    case IDM_MAIN_RESET:
      Stop();
      Start();
      break;
    case IDM_MAIN_SOUND:
      g_bUseSound = !g_bUseSound;
			if (g_bUseSound)
				waveout_open();
      else
				waveout_close();
      Resume();
      break;
    case IDM_MAIN_CLIP:
      PPU_UpDown_Clip = !PPU_UpDown_Clip;
      Resume();
      break;
    case IDM_MAIN_KEY_CONFIG:
      DialogBox(g_hInst, (LPCTSTR)IDD_KEY_CONFIG, hwnd, KeyConfigDlg);
      Resume();
      break;
    case IDM_MAIN_FRAME_0:
    case IDM_MAIN_FRAME_1:
    case IDM_MAIN_FRAME_2:
    case IDM_MAIN_FRAME_3:
    case IDM_MAIN_FRAME_4:
    case IDM_MAIN_FRAME_5:
    case IDM_MAIN_FRAME_8:
    case IDM_MAIN_FRAME_10:
      FrameCnt = 0;
      FrameSkip = LOWORD(wp) - IDM_MAIN_FRAME_0;
      Resume();
      break;
    case IDM_MAIN_VERSION:
      ShowMessage( "%s\nA portable NES emulator\nCopyright (C) 1999-2004\nJay's Factory",
		   VERSION );
      Resume();
      break;
    case IDM_MAIN_ROM:
      if ( g_hThread ) {
				ShowMessage( "Mapper\t%d\nPRG ROM\t%dKB\nCHR ROM\t%dKB\n" \
		     "Mirroring\t%s\nSRAM\t\t%s\n4 Screen\t%s\nTrainer\t\t%s",
		     MapperNo, NesHeader.byRomSize * 16, NesHeader.byVRomSize * 8,
		     ( ROM_Mirroring ? "V" : "H" ), ( ROM_SRAM ? "Yes" : "No" ), 
		     ( ROM_FourScr ? "Yes" : "No" ), ( ROM_Trainer ? "Yes" : "No" ) );
      Resume();
      }
      break;
    }
    break;
  case WM_INITMENUPOPUP:
    CheckMenuItem(HMENU(wp), IDM_MAIN_FRAME_0, MF_UNCHECKED | MF_BYCOMMAND);
    CheckMenuItem(HMENU(wp), IDM_MAIN_FRAME_1, MF_UNCHECKED | MF_BYCOMMAND);
    CheckMenuItem(HMENU(wp), IDM_MAIN_FRAME_2, MF_UNCHECKED | MF_BYCOMMAND);
    CheckMenuItem(HMENU(wp), IDM_MAIN_FRAME_3, MF_UNCHECKED | MF_BYCOMMAND);
    CheckMenuItem(HMENU(wp), IDM_MAIN_FRAME_4, MF_UNCHECKED | MF_BYCOMMAND);
    CheckMenuItem(HMENU(wp), IDM_MAIN_FRAME_5, MF_UNCHECKED | MF_BYCOMMAND);
    CheckMenuItem(HMENU(wp), IDM_MAIN_FRAME_8, MF_UNCHECKED | MF_BYCOMMAND);
    CheckMenuItem(HMENU(wp), IDM_MAIN_FRAME_10, MF_UNCHECKED | MF_BYCOMMAND);
    CheckMenuItem(HMENU(wp), IDM_MAIN_FRAME_0 + FrameSkip, MF_CHECKED);
    CheckMenuItem(HMENU(wp), IDM_MAIN_SOUND, g_bUseSound ? MF_CHECKED  | MF_BYCOMMAND:
		  MF_UNCHECKED | MF_BYCOMMAND);
    CheckMenuItem(HMENU(wp), IDM_MAIN_CLIP, PPU_UpDown_Clip ? MF_CHECKED  | MF_BYCOMMAND:
		  MF_UNCHECKED | MF_BYCOMMAND);
    Pause();
    break;
  case WM_CREATE:
    g_hwndCB = CreateMenuBar(hwnd);
    break;
  case WM_LBUTTONDOWN:
    Pause();
    break;
  case WM_DESTROY:
    GXCloseDisplay();
    GXCloseInput();
    RegSave();
    PostQuitMessage(0);
    break;
  case WM_KILLFOCUS:
    Pause();
    break;
  case WM_CLOSE:
    Stop();
    CommandBar_Destroy(g_hwndCB);
    DestroyWindow(hwnd);
    break;
  case WM_SUSPEND:
    Pause();
    break;
  default:
    return DefWindowProc(hwnd, msg, wp, lp);
  }
  
  return 0;
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
	g_nSRAM_SaveFlag = 0;

	// It is finished if the ROM doesn't have SRAM
	if ( !ROM_SRAM )
		return 0;

	// There is necessity to save it
	g_nSRAM_SaveFlag = 1;

	// The preparation of the SRAM file name
	strcpy( g_szSaveName, g_szRomName );
	strcpy( strrchr( g_szSaveName, '.' ) + 1, SRAM_FILE_EXT );

	/*-------------------------------------------------------------------*/
	/*  Read a SRAM data                                                 */
	/*-------------------------------------------------------------------*/

	// Open SRAM file
	fp = fopen( g_szSaveName, "rb" );
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

	if ( !g_nSRAM_SaveFlag )
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
	fp = fopen( g_szSaveName, "wb" );
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

  /* Nothing to do here */
	return g_bExitThread ? -1 : 0;
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
 *      Points to the starting address of the copied block's destination
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

	memcpy( dest, src, count );
	return dest;
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemorySet() : memset                          */
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

	memset( dest, c, count);  
	return dest;
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
	unsigned short *pusLine, *pusDest, *pusSrc;
	int x, y, xp, yp;

	pusSrc = WorkFrame + 8;
	xp = g_gxdp.cbxPitch >> 1;
	yp = g_gxdp.cbyPitch >> 1;
	pusLine = (unsigned short *)GXBeginDraw();

	if (!pusLine)
		return;

	pusLine += (yp * SCREEN_TOP);

	for(y = 0; y < NES_DISP_HEIGHT; y++){
		pusDest = pusLine;
		for(x = 0; x < NES_DISP_WIDTH - 16; x++){
#if 0
			*pusDest = *pusSrc;
#else
			/* Translate here into 5-6-5 format in GAPI 1.2 */
			*pusDest = ((*pusSrc & 0x7fe0)<<1)|(*pusSrc&0x001f);
#endif
			pusDest += xp;
			pusSrc++;
		}
		pusSrc += 16;
		pusLine += yp;
	}
	
	GXEndDraw();
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

	/* Transfer joypad state */
	*pdwPad1 =	( GetAsyncKeyState(g_nKeyConfig[0]) < 0 ) | /* A */
				((GetAsyncKeyState(g_nKeyConfig[1]) < 0 ) << 1 ) | /* B */
				((GetAsyncKeyState(g_nKeyConfig[2]) < 0 ) << 2 ) | /* Select */
				((GetAsyncKeyState(g_nKeyConfig[3]) < 0 ) << 3 ) | /* Start */
				((GetAsyncKeyState(g_nKeyConfig[4]) < 0 ) << 4 ) | /* Up */
				((GetAsyncKeyState(g_nKeyConfig[5]) < 0 ) << 5 ) | /* Down */
				((GetAsyncKeyState(g_nKeyConfig[6]) < 0 ) << 6 ) | /* Left */
				((GetAsyncKeyState(g_nKeyConfig[7]) < 0 ) << 7 ); /* Right */

	if (GetAsyncKeyState(g_nKeyConfig[8]) & 0x8000)
		PostMessage(g_hwnd, WM_SUSPEND, 0, 0);

	*pdwPad2   = 0;
	*pdwSystem = g_bExitThread ? PAD_SYS_QUIT : 0;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundInit() : Sound Emulation Initialize           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundInit( void ) 
{
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen( int samples_per_sync, int sample_rate ) 
{
	g_nSamplesPerSync = samples_per_sync;
	g_nSampleRate = sample_rate;

	if (g_bUseSound)
		waveout_open();

	return 1;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundClose( void ) 
{
	waveout_close();
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SoundOutput() : Sound Output 5 Waves           */           
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundOutput( int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5 ) 
{
	if (!g_hwo || !g_pbBuff)
		return;

	// copy buffer
	for (int i = 0; i < samples; i++)
		g_Hdr[g_nCurrent].lpData[i] = (wave1[i] + wave2[i] + wave3[i] + wave4[i]) / 5;

	g_Hdr[g_nCurrent].dwBufferLength = 
		g_Hdr[g_nCurrent].dwBytesRecorded = samples;

	// write
	waveOutWrite(g_hwo, &g_Hdr[g_nCurrent], sizeof(WAVEHDR));
	
	g_nCurrent = (g_nCurrent + 1) % SOUND_BUFS;
	WaitForSingleObject(g_hSem, INFINITE);
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_Wait() : Wait Emulation if required            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Wait()
{
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_MessageBox() : Print System Message            */
/*                                                                   */
/*===================================================================*/
void InfoNES_MessageBox( char *pszMsg, ... )
{

  char  pszErr[ 1024 ];
  TCHAR pszOut[ 1024 ];
  va_list args;

  va_start( args, pszMsg );
  vsprintf( pszErr, pszMsg, args );  pszErr[ 1023 ] = '\0';
  va_end( args );

  mbstowcs( pszOut, pszErr, 1024 );
  MessageBox( g_hwnd, pszOut, c_szTitle, MB_OK | MB_ICONSTOP );
}

/*===================================================================*/
/*                                                                   */
/*              ShowMessage() : Print System Information             */
/*                                                                   */
/*===================================================================*/
void ShowMessage( char *pszMsg, ... )
{
  char  pszErr[ 1024 ];
  TCHAR pszOut[ 1024 ];
  va_list args;

  va_start( args, pszMsg );
  vsprintf( pszErr, pszMsg, args );  pszErr[ 1023 ] = '\0';
  va_end( args );

  mbstowcs( pszOut, pszErr, 1024 );
  MessageBox( g_hwnd, pszOut, c_szTitle, MB_OK | MB_ICONINFORMATION );
}
