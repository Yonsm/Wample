


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Header
#include "Define.h"
#include "UniBase.h"
#include "Helper.h"
#include "MiniCls.h"
#include "Resource.h"

#include <AygShell.h>
#pragma comment(lib, "AygShell.lib")

HINSTANCE g_hInst = NULL;

// CodeHij interface
#define UM_CodeHij (WM_APP + 272)
extern "C" BOOL WINAPI HijStart(HWND hNotify);
extern "C" BOOL WINAPI HijStop();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MainDlgProc dialog
INT_PTR CALLBACK MainDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	STATIC CMenuBar s_mb;
	static BOOL s_bStarted = FALSE;
	STATIC SHACTIVATEINFO s_sa = {sizeof(SHACTIVATEINFO)};

	switch (uMsg)
	{
	case UM_CodeHij:
		UINT uLen;
		uLen = SendDlgItemMessage(hWnd, IDC_Log, WM_GETTEXTLENGTH, 0, 0);
		SendDlgItemMessage(hWnd, IDC_Log, EM_SETSEL, uLen, uLen);
		SendDlgItemMessage(hWnd, IDC_Log, EM_REPLACESEL, 0, lParam);
		break;

	case WM_INITDIALOG:
		s_mb = CHelper::InitDlgBar(hWnd, TRUE, TBSTATE_ENABLED, TBSTATE_ENABLED, IDR_Menu);
		return TRUE;

	case WM_SIZE:
		CHelper::ReSize(hWnd, IDC_Log, lParam);
		break;

	case WM_ACTIVATE:
		SHHandleWMActivate(hWnd, wParam, lParam, &s_sa, FALSE);
		break;

	case WM_SETTINGCHANGE:
		SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sa);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_About:
			MessageBox(hWnd, STR_VersionStamp TEXT("\r\n\r\n") STR_BuildStamp TEXT("\r\n\r\n") STR_Copyright TEXT("\r\n\r\n") STR_Web, STR_AppName, MB_ICONINFORMATION);
			break;

		case IDM_Clear:
			SetDlgItemText(hWnd, IDC_Log, NULL);
			break;

		case IDOK:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				TCHAR tzStr[MAX_PATH];
				if (s_bStarted)
				{
					if (HijStop())
					{
						s_bStarted = FALSE;
						s_mb.SetButtonText(IDOK, _GetStr(IDS_Start));
					}
					else
					{
						MessageBox(hWnd, _GetStr(IDS_StopError), STR_AppName, MB_ICONERROR);
					}
				}
				else
				{
					s_bStarted = HijStart(hWnd);
					if (s_bStarted)
					{
						s_mb.SetButtonText(IDOK, _GetStr(IDS_Stop));
					}
					else
					{
						MessageBox(hWnd, _GetStr(IDS_StartError), STR_AppName, MB_ICONERROR);
					}
				}
				break;
			}

		case IDCANCEL:
		case IDM_Exit:
			if (s_bStarted)
			{
				SendMessage(hWnd, WM_COMMAND, IDOK, 0);
			}
			EndDialog(hWnd, S_OK);
			break;
		}
		break;
	}

	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EXE Entry
INT APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PTSTR ptzCmdLine, INT iCmdShow)
{
	g_hInst = hInstance;
	SHInitExtraControls();
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_Main), NULL, MainDlgProc);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
