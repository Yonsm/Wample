
// Header
#include "StdAfx.h"
#include "AppWnd.h"

// Global variable
HWND g_hWnd = NULL;


// Entry
INT APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PTSTR ptzCmdLine, INT iCmdShow)
{
	// Active previous instance
	HWND hWnd = FindWindow(STR_AppName, NULL);
	if (hWnd)
	{
		ShowWindow(hWnd, SW_SHOW);
		SetForegroundWindow(HWND(UINT_PTR(hWnd) | 0x00000001));
		return 0;
	}

	// Init library
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	InitCommonControls();
#ifdef WINCE
	SHInitExtraControls();
#endif

	// Set basic information
	g_hInst = hInstance;
	g_ptzAppName = TStrGet(IDS_AppName);

	// Run the main window
	CAppWnd aw;
	aw.Run(ptzCmdLine, iCmdShow);

	// Free library
	CoUninitialize();

	return 0;
}
