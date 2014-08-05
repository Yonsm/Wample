


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Header
#include "StdAfx.h"
#include "AppWnd.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
CAppWnd::CAppWnd()
{
	// Register window class
	WNDCLASS wc = {0};
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WNDPROC(m_tWnd.Init(this, &CAppWnd::WndProc));
	wc.hInstance = g_hInst;
	wc.hbrBackground = HBRUSH(COLOR_INACTIVECAPTION);
	wc.lpszClassName = STR_AppName;
#ifndef WINCE
	wc.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_AppIcon));
	wc.hCursor = LoadCursor(NULL, IDC_HAND);
#endif
	RegisterClass(&wc);

#ifdef WINCE
	// Create window
	CreateWindowEx(0, STR_AppName, g_ptzAppName, WS_VISIBLE, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, g_hInst, 0);
#else
	CreateWindowEx(0, STR_AppName, g_ptzAppName, WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 248, 350, NULL, NULL, g_hInst, 0);
#endif

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
CAppWnd::~CAppWnd()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Show main window
WPARAM CAppWnd::Run(PTSTR ptzCmdLine, INT iCmdShow)
{
	ShowWindow(m_tWnd, iCmdShow);
	UpdateWindow(m_tWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Window callback function
LRESULT CALLBACK CAppWnd::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		OnCreate();
		break;

	case WM_ACTIVATE:
		break;

	case WM_PAINT:
		OnPaint();
		break;

	case WM_SIZE:
		OnSize(lParam);
		break;

	case WM_COMMAND:
		OnCommand(LOWORD(wParam));
		break;

	case WM_KEYDOWN:
		OnKeyDown(wParam);
		break;

	case WM_KEYUP:
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

#ifdef WINCE
	_HandleActivate(m_tWnd, uMsg, wParam, lParam);
#endif
	return DefWindowProc(m_tWnd, uMsg, wParam, lParam);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create handler
VOID CAppWnd::OnCreate()
{
#ifdef WINCE
	CeleMenuBar mb;
	mb.Create(m_tWnd, IDR_DlgMenu);
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Paint handler
VOID CAppWnd::OnPaint(BOOL bGray)
{
	PAINTSTRUCT ps;
	BeginPaint(m_tWnd, &ps);

	EndPaint(m_tWnd, &ps);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sizing handler
VOID CAppWnd::OnSize(LPARAM lParam)
{

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command handler
VOID CAppWnd::OnCommand(UINT uCmd)
{
	switch (uCmd)
	{
#ifdef WINCE
	case IDC_LSK:
		OnKeyDown(VK_F1);
		break;

	case IDC_RSK:
		OnKeyDown(VK_F2);
		break;
#endif

	case IDCANCEL:
		break;

	//case IDM_About:
	//	CeleAbout::Show(m_tWnd);
	//	break;

	//case IDM_Hide:
	//	ShowWindow(m_tWnd, SW_HIDE);
	//	break;

 	case IDOK:
 	//case IDM_Exit:
 		DestroyWindow(m_tWnd);
 		break;

	default:
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
VOID CAppWnd::OnKeyDown(UINT vKey)
{
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
