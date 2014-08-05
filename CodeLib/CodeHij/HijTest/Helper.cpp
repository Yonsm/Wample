


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Header
#include "Define.h"
#include "UniBase.h"
#include "Reg.h"
#include "MiniCls.h"
#include "Helper.h"
#include <CommDlg.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get system parameters
DWORD CHelper::GetSysSpec()
{
	STATIC DWORD s_dwSysSpec = 0;

	if (s_dwSysSpec == 0)
	{
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&vi);
		s_dwSysSpec = MAKEWORD(vi.dwMajorVersion, vi.dwMinorVersion);

		TCHAR tzStr[MAX_NAME];
		SystemParametersInfo(SPI_GETPLATFORMTYPE, MAX_NAME, tzStr, 0);
		if (!UStrCmpI(tzStr, TEXT("PocketPC")))
		{
			s_dwSysSpec |= SF_PocketPC;
		}
	}

	return s_dwSysSpec;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize dialog
HWND CHelper::InitDlgBar(HWND hWnd, BOOL bOverBack, BYTE bCancelState, BYTE bOKState, UINT uMenuBar)
{
	CMenuBar cMenuBar;
	cMenuBar.Create(hWnd, uMenuBar);

	if (bOverBack)
	{
		cMenuBar.OverrideKey(VK_TBACK);
	}
	if (bOKState != TBSTATE_ENABLED)
	{
		cMenuBar.SetButtonState(IDOK, bOKState);
	}
	if (bCancelState != TBSTATE_ENABLED)
	{
		cMenuBar.SetButtonState(IDCANCEL, bCancelState);
	}

	InitDialog(hWnd);
	return cMenuBar;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize list box
INT CHelper::InitListBox(HWND hList, UINT uID, UINT uStrRes, UINT iSel)
{
	TCHAR tzStr[MAX_STR];
	for (PTSTR p = _GetStr(uStrRes), q = tzStr; *q; q++)
	{
		if (*q == '|')
		{
			*q = 0;
			SendDlgItemMessage(hList, uID, LB_ADDSTRING, 0, (LPARAM) p);
			p = q + 1;
		}
	}

	return SendDlgItemMessage(hList, uID, LB_SETCURSEL, iSel, 0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get owner name
PTSTR CHelper::GetOwnerName(PTSTR ptzName)
{
	CReg reg(TEXT("ControlPanel\\Owner"));
	if (!reg.GetStr(TEXT("Name"), ptzName))
	{
		reg.GetStr(TEXT("Owner"), ptzName, MAX_NAME, STR_Author);
	}
	return ptzName;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize dialog
INT CHelper::MsgBox(HWND hParent, UINT uCmd, PCTSTR ptzText, UINT uType)
{
	CMenuBar cMenuBar;
	cMenuBar.Attach(hParent);

	CMenu cMenu;
	TCHAR tzCmd[MAX_NAME];
	cMenu = cMenuBar.GetSubMenu(IDCANCEL);
	if (!cMenu.GetText(uCmd, tzCmd))
	{
		cMenu = cMenuBar.GetSubMenu(IDOK);
		if (!cMenu.GetText(uCmd, tzCmd))
		{
			_LoadStr(IDS_AppName, tzCmd);
		}
	}

	TCHAR tzText[MAX_STR];
	if (_IsIntRes(ptzText))
	{
		TCHAR tzStr[MAX_STR];
		UStrPrint(tzText, _GetStr((UINT) ptzText), tzCmd);
		ptzText = tzText;
	}

	return MessageBox(hParent, ptzText, tzCmd, uType);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Declaration
#if (_WIN32_WCE < 0x500)
typedef struct tagOPENFILENAMEEX
{
	// Fields which map to OPENFILENAME
	DWORD        lStructSize;
	HWND         hwndOwner;
	HINSTANCE    hInstance;
	LPCTSTR      lpstrFilter;
	LPTSTR       lpstrCustomFilter;
	DWORD        nMaxCustFilter;
	DWORD        nFilterIndex;
	LPTSTR       lpstrFile;
	DWORD        nMaxFile;
	LPTSTR       lpstrFileTitle;
	DWORD        nMaxFileTitle;
	LPCTSTR      lpstrInitialDir;
	LPCTSTR      lpstrTitle;
	DWORD        Flags;
	WORD         nFileOffset;
	WORD         nFileExtension;
	LPCTSTR      lpstrDefExt;
	LPARAM       lCustData;
	LPOFNHOOKPROC lpfnHook;
	LPCTSTR      lpTemplateName;

	// Extended fields
	DWORD       dwSortOrder;
	DWORD       ExFlags;
}
OPENFILENAMEEX, *LPOPENFILENAMEEX;

// Sort order
typedef enum tagOFN_SORTORDER
{
	OFN_SORTORDER_AUTO,
	OFN_SORTORDER_DATE,
	OFN_SORTORDER_NAME,
	OFN_SORTORDER_SIZE,
	OFN_SORTORDER_ASCENDING = 0x00008000

}
OFN_SORTORDER;

// Extended Flags
typedef enum tagOFN_EXFLAG
{
	OFN_EXFLAG_DETAILSVIEW			= 0x00000001,
	OFN_EXFLAG_THUMBNAILVIEW		= 0x00000002,
	OFN_EXFLAG_LOCKDIRECTORY		= 0x00000100,
	OFN_EXFLAG_NOFILECREATE			= 0x00000200,
	OFN_EXFLAG_HIDEDRMPROTECTED		= 0x00010000,
	OFN_EXFLAG_HIDEDRMFORWARDLOCKED	= 0x00020000
}
OFN_EXFLAG;
#endif

typedef BOOL (*PGETFILENAME)(LPOPENFILENAMEEX pOfn);
typedef BOOL (*TGETFILENAME)(BOOL bSave, LPOPENFILENAMEEX pOfn);

// Module name and procedure table
enum {MP_RFileShell, MP_AygShell, MP_tGetFile, MP_CoreDll};
const struct {PCTSTR ptzModName; PCTSTR ptzOpenProc; PCTSTR ptzSaveProc;} c_sModProc[] =
{
	{TEXT("RFileShell"), TEXT("RShellGetOpenFileName"), TEXT("RShellGetSaveFileName")},
	{TEXT("AygShell"), TEXT("GetOpenFileNameEx"), TEXT("GetSaveFileNameEx")},
	{TEXT("tGetFile"), TEXT("tGetFile"), TEXT("tGetFile")},
	{TEXT("CoreDll"), TEXT("GetOpenFileNameW"), TEXT("GetSaveFileNameW")},
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Browse for file name
#define OFN_COMMON (0x11000000/* | OFN_HIDEREADONLY*/)
BOOL CHelper::GetFileName(HWND hParent, PTSTR ptzPath, UINT uFilterRes, BOOL bSave)
{
	// Fill structure
	OPENFILENAMEEX ofn = {0};
	ofn.hwndOwner = hParent;
	ofn.hInstance = g_hInst;
	ofn.lpstrFile = ptzPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = bSave ? (OFN_COMMON | OFN_OVERWRITEPROMPT) : (OFN_COMMON | OFN_PATHMUSTEXIST);

	ofn.ExFlags = OFN_EXFLAG_THUMBNAILVIEW;
	ofn.dwSortOrder = OFN_SORTORDER_NAME | OFN_SORTORDER_ASCENDING;

	// Get initialize folder from control
	TCHAR tzPath[MAX_PATH];
	if (_IsIntRes(ptzPath))
	{
		ofn.lpstrFile = tzPath;
		GetDlgItemText(hParent, (UINT) ptzPath, tzPath, MAX_PATH);
	}

#ifdef _SAVEDIR
	// Read initialize folder
	CReg reg;
	TCHAR tzInitDir[MAX_PATH];
	if (reg.GetStr(TEXT("InitDir"), tzInitDir))
	{
		ofn.lpstrInitialDir = tzInitDir;
	}
	else
	{
		ofn.lpstrInitialDir = TEXT("\\");
	}
#else
	ofn.lpstrInitialDir = TEXT("\\");
	if (ofn.lpstrFile[0] == 0)
	{
		ofn.lpstrFile[0] = '\\';
		ofn.lpstrFile[1] = 0;
	}
#endif

	// Load extension filter
	TCHAR tzFilter[MAX_PATH];
	_LoadStr(uFilterRes, tzFilter);
	ofn.lpstrFilter = UStrRep(tzFilter, '|', 0);

	// Parse default extension
	TCHAR tzDefExt[MAX_NAME];
	PTSTR p = UStrChr(tzFilter, 0) + 1;
	if (p = UStrChr(p, '.'))
	{
		UStrCopy(tzDefExt, p);
		if (p = UStrChr(tzDefExt, ';')) *p = 0;
		ofn.lpstrDefExt = tzDefExt;
	}

	// Get title
	TCHAR tzTitle[MAX_NAME];
	ofn.lpstrTitle = _LoadStr(bSave ? IDS_Save : IDS_Open, tzTitle);

	// Try all library
	UINT i = 0;
	BOOL bResult = FALSE;
	for (; i < _NumOf(c_sModProc); i++)
	{
		HMODULE hModule = LoadLibrary(c_sModProc[i].ptzModName);
		if ((hModule == NULL) && (i == MP_RFileShell))
		{
			// Lookup Resco Explorer install folder
			TCHAR tzModule[MAX_PATH];
			CReg rfs(TEXT("SOFTWARE\\Apps\\Resco Explorer"), HKEY_LOCAL_MACHINE);
			if (rfs.GetStr(TEXT("InstallDir"), tzModule))
			{
				UPathMake(tzModule, c_sModProc[i].ptzModName);
				hModule = LoadLibrary(tzModule);
			}
		}
		if (hModule)
		{
			PGETFILENAME pGetFileName = (PGETFILENAME) GetProcAddress(hModule, bSave ? c_sModProc[i].ptzSaveProc : c_sModProc[i].ptzOpenProc);
			if (pGetFileName)
			{
				ofn.lStructSize = (i == MP_AygShell) ? sizeof(OPENFILENAMEEX) : sizeof(OPENFILENAME);
				bResult = (i == MP_tGetFile) ? ((TGETFILENAME) pGetFileName)(bSave, &ofn) : pGetFileName(&ofn);
				FreeLibrary(hModule);
				break;
			}
			FreeLibrary(hModule);
		}
	}

	// Prompt for library
	if ((i == MP_CoreDll) && !IsPPC() && !IsWM5() && !bResult)
	{
		TCHAR tzStr[MAX_PATH];
		TCHAR tzInfo[MAX_STR];
		UStrPrint(ofn.lpstrFile, TEXT("\\My Documents\\%s%s"), STR_AppName, ofn.lpstrDefExt);
		UStrPrint(tzInfo, _GetStr(IDS_GetFileName), ofn.lpstrFile);
		bResult = (MessageBox(hParent, tzInfo, tzTitle, MB_ICONQUESTION | MB_YESNO) == IDYES);
	}

	if (bResult)
	{
		// Set control text
		if (_IsIntRes(ptzPath))
		{
			SetFocus(GetDlgItem(hParent, (UINT) ptzPath));
			SetDlgItemText(hParent, (UINT) ptzPath, tzPath);
		}

#ifdef _SAVEDIR
		// Save initialize folder
		p = UStrRChr(ofn.lpstrFile, '\\');
		if (p++)
		{
			TCHAR c = *p;
			*p = 0;
			reg.SetStr(TEXT("InitDir"), ofn.lpstrFile);
			*p = c;
		}
#endif
	}

	return bResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
