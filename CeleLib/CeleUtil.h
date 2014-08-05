


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleUtil 2.0.204
// Copyright (C) Yonsm 2007-201`, All Rights Reserved.
#pragma once
#include "UniBase.h"
#include "CeleReg.h"

#ifdef WINCE
#include <AygShell.h>
#pragma comment(lib, "AygShell.lib")
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System specification
#define SYS_MajorVer	0x000000FF
#define SYS_MinorVer	0x0000FF00

#define SYS_TypeMask	0x00FF0000
#define SYS_Windows		0x00000000
#define SYS_PocketPC	0x00010000
#define SYS_Smartphone	0x00020000

#define SYS_WM5			0x00200000
#define SYS_WM6			0x00400000

#define SYS_UserMask	0xFF000000

#define ST_SP2003		SYS_Smartphone
#define ST_PPC2003		SYS_PocketPC
#define ST_SP5			(SYS_WM5 | SYS_Smartphone)
#define ST_PPC5			(SYS_WM5 | SYS_PocketPC)
#define ST_SP6			(SYS_WM5 | SYS_WM6 | SYS_Smartphone)
#define ST_PPC6			(SYS_WM5 | SYS_WM6 | SYS_PocketPC)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GDI Escapes for ExtEscape
#define QUERYESCSUPPORT    8

// The following are unique to CE
#define GETVFRAMEPHYSICAL	6144
#define GETVFRAMELEN		6145
#define DBGDRIVERSTAT		6146
#define SETPOWERMANAGEMENT	6147
#define GETPOWERMANAGEMENT	6148

typedef enum _VIDEO_POWER_STATE
{
	VideoPowerOn = 1,
	VideoPowerStandBy,
	VideoPowerSuspend,
	VideoPowerOff
}
VIDEO_POWER_STATE, *PVIDEO_POWER_STATE;


typedef struct _VIDEO_POWER_MANAGEMENT
{
	ULONG Length;
	ULONG DPMSVersion;
	ULONG PowerState;
}
VIDEO_POWER_MANAGEMENT, *PVIDEO_POWER_MANAGEMENT;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleUtil
class CeleUtil
{
private:
	STATIC DWORD m_dwSysSpec;

private:
	ISTATIC DWORD InitSysSpec()
	{
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&vi);

		DWORD dwSysSpec = MAKEWORD(vi.dwMajorVersion, vi.dwMinorVersion);

#ifdef WINCE
		if (vi.dwMajorVersion >= 5)
		{
			dwSysSpec |= SYS_WM5;
			if (vi.dwMinorVersion >= 2)
			{
				dwSysSpec |= SYS_WM6;
			}
		}
		TCHAR tzStr[MAX_NAME];
		SystemParametersInfo(SPI_GETPLATFORMTYPE, MAX_NAME, tzStr, 0);
		if (!TStrCmpI(tzStr, TEXT("PocketPC")))
		{
			dwSysSpec |= SYS_PocketPC;
		}
		else if (!TStrCmpI(tzStr, TEXT("Smartphone")))
		{
			dwSysSpec |= SYS_Smartphone;
		}
#endif

		return dwSysSpec;
	}

public:
	ISTATIC DWORD GetSysSpec()
	{
		return m_dwSysSpec;
	}

	ISTATIC BYTE GetMajorVer()
	{
		return (BYTE) (GetSysSpec() & SYS_MajorVer);
	}

	ISTATIC BYTE GetMinorVer()
	{
		return (BYTE) ((GetSysSpec() & SYS_MinorVer) >> 8);
	}

	ISTATIC BYTE GetSysType()
	{
		return (BYTE) ((GetSysSpec() & SYS_TypeMask) >> 16);
	}

	ISTATIC BOOL IsWM5()
	{
#if (_WIN32_WCE >= 0x500)
		return TRUE;
#else
		return GetSysSpec() & SYS_WM5;
#endif
	}

	ISTATIC BOOL IsWM6()
	{
		return GetSysSpec() & SYS_WM6;
	}

	ISTATIC BOOL IsPPC()
	{
		return GetSysSpec() & SYS_PocketPC;
	}

public:
	ISTATIC UINT GetOwnerName(PTSTR ptzName)
	{
		CeleReg reg(TEXT("ControlPanel\\Owner"));
		UINT nRet = reg.GetStr(TEXT("Name"), ptzName);
		if (nRet == 0)
		{
			nRet = reg.GetStr(TEXT("Owner"), ptzName/*, MAX_NAME, STR_Author*/);
		}
		return nRet;
	}

	ISTATIC DWORD GetFileAttr(PCTSTR ptzPath)
	{
		WIN32_FILE_ATTRIBUTE_DATA fa;
		fa.dwFileAttributes = 0;
		GetFileAttributesEx(ptzPath, GetFileExInfoStandard, &fa);
		return fa.dwFileAttributes;
	}

	ISTATIC BOOL RemoveFileAttr(PCTSTR ptzPath, DWORD dwRemove = FILE_ATTRIBUTE_READONLY)
	{
		DWORD dwAttr = GetFileAttr(ptzPath);
		return (dwAttr & dwRemove) ? SetFileAttributes(ptzPath, dwAttr & ~dwRemove) : FALSE;
	}

	ISTATIC INT_PTR ShellOpen(PCTSTR ptzPath, PCTSTR ptzParam = NULL, HWND hWnd = GetActiveWindow(), DWORD dwMask = 0)
	{
		SHELLEXECUTEINFO se ={0};
		se.cbSize = sizeof(se);
		//se.lpVerb = TEXT("Open");
		se.lpFile = ptzPath;
		se.hwnd = hWnd;
		se.lpParameters = ptzParam;
		se.nShow = SW_SHOWNORMAL;
		se.fMask = dwMask;
		BOOL bResult = ShellExecuteEx(&se);
		return (dwMask & SEE_MASK_NOCLOSEPROCESS) ? (INT_PTR) se.hProcess : bResult;
	}

	ISTATIC BOOL ExecCtrlPanel(INT iIndex, INT iPage = 0)
	{
		TCHAR tzParam[MAX_PATH];
		TStrFormat(tzParam, TEXT("cplmain.cpl, %d, %d"), iIndex, iPage);
		return BOOL(ShellOpen(TEXT("ctlpnl.exe"), tzParam));
	}

	ISTATIC BOOL SaveBitmap(PCTSTR ptzPath, PBITMAPINFO pbmInfo, PVOID pvBits)
	{
		HANDLE hFile = UFileOpen(ptzPath, UFILE_WRITE);
		if (hFile)
		{
			BITMAPFILEHEADER bf;
			UINT nInfoSize = sizeof(pbmInfo->bmiHeader) + pbmInfo->bmiHeader.biClrUsed * sizeof(pbmInfo->bmiColors);
			bf.bfType = 'MB';
			bf.bfOffBits = sizeof(bf) + nInfoSize;
			bf.bfSize = bf.bfOffBits + pbmInfo->bmiHeader.biSizeImage;
			bf.bfReserved1 = 0;
			bf.bfReserved2 = 0;
			UFileWrite(hFile, &bf, sizeof(bf));
			UFileWrite(hFile, pbmInfo, bf.bfOffBits - sizeof(bf));
			UFileWrite(hFile, pvBits, pbmInfo->bmiHeader.biSizeImage);
			UFileClose(hFile);
			return TRUE;
		}
		return FALSE;
	}

public:
	ISTATIC BOOL ToggleScreen(VIDEO_POWER_STATE eState = VideoPowerOff)
	{
#ifdef WINCE
		HDC hDC = GetDC(NULL);
		INT iESC = SETPOWERMANAGEMENT;
		if (ExtEscape(hDC, QUERYESCSUPPORT, sizeof(INT), (PCSTR) &iESC, 0, NULL) == FALSE)
		{
			VIDEO_POWER_MANAGEMENT vpm;
			vpm.Length = sizeof(VIDEO_POWER_MANAGEMENT);
			vpm.DPMSVersion = 0x0001;
			vpm.PowerState = VideoPowerOff;
			ExtEscape(hDC, SETPOWERMANAGEMENT, vpm.Length, (PCSTR) &vpm, 0, NULL);
			ReleaseDC(NULL, hDC);
			return TRUE;
		}
#endif
		return FALSE;
	}

	ISTATIC BOOL RotateScreen(BOOL bFourDirection = FALSE)
	{
#ifdef WINCE
		DEVMODE dm = {0};
		dm.dmSize = sizeof(DEVMODE);
		dm.dmFields = DM_DISPLAYORIENTATION;
		if (ChangeDisplaySettingsEx(NULL, &dm, NULL, CDS_TEST, NULL) == DISP_CHANGE_SUCCESSFUL)
		{
			if (bFourDirection)
			{
				switch (dm.dmDisplayOrientation)
				{
				case DMDO_0: dm.dmDisplayOrientation = DMDO_90; break;
				case DMDO_90: dm.dmDisplayOrientation = DMDO_180; break;
				case DMDO_180: dm.dmDisplayOrientation = DMDO_270; break;
				case DMDO_270: dm.dmDisplayOrientation = DMDO_0; break;
				}
			}
			else
			{
				dm.dmDisplayOrientation = !dm.dmDisplayOrientation;
			}
		
			return ChangeDisplaySettingsEx(NULL,&dm,NULL,CDS_RESET,NULL) == DISP_CHANGE_SUCCESSFUL;
		}
#endif
		return FALSE;
	}

	ISTATIC VOID FullScreen(HWND hWnd, BOOL bFullScreen = TRUE, BOOL bShowSip = FALSE)
	{
#ifdef WINCE
		// Handle window
		SHFullScreen(hWnd, bFullScreen ? (SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON) : (SHFS_SHOWTASKBAR | SHFS_SHOWSIPBUTTON)); 

		// Handle menu bar
		static INT s_iMBHeight = 26;
		HWND hMenuBar = SHFindMenuBar(hWnd);
		if (bFullScreen)
		{
			RECT rt;
			GetWindowRect(hMenuBar, &rt);
			INT iMBHeight = _RectHeight(rt);
			if (iMBHeight) s_iMBHeight = iMBHeight;

			SetWindowPos(hMenuBar, NULL, -1, -1, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else
		{
			SetWindowPos(hMenuBar, NULL, 0, GetSystemMetrics(SM_CYSCREEN) - s_iMBHeight, GetSystemMetrics(SM_CXSCREEN), s_iMBHeight, SWP_NOZORDER | SWP_NOACTIVATE);
		}

		RECT rt;
		if (bFullScreen)
		{
			SetRect(&rt, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		}
		else
		{
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rt, FALSE);
			rt.bottom -= s_iMBHeight;
		}
		MoveWindow(hWnd, rt.left, rt.top, _RectWidth(rt), _RectHeight(rt), FALSE);

		// Handle SIP
		SHSipPreference(hWnd, bShowSip ? SIP_UP : SIP_FORCEDOWN);
#endif
	}
};

__declspec(selectany) DWORD CeleUtil::m_dwSysSpec = CeleUtil::InitSysSpec();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
