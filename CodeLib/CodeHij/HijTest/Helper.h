


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Header
#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include <AygShell.h>
#include "Resource.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System flags
#define SF_PocketPC			0x10000000
#define SF_MajorVersion		0x000000FF
#define SF_MinorVersion		0x0000FF00
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CHelper class
class CHelper
{
public:
	STATIC DWORD GetSysSpec();

	STATIC HWND InitDlgBar(HWND hWnd, BOOL bOverBack = FALSE, BYTE bCancelState = TBSTATE_ENABLED, BYTE bOKState = TBSTATE_ENABLED, UINT uMenuBar = IDS_OK);

	STATIC INT InitListBox(HWND hList, UINT uID, UINT uStrRes, UINT iSel = 0);

	STATIC PTSTR GetOwnerName(PTSTR ptzName);

	STATIC BOOL GetFileName(HWND hParent, PTSTR ptzPath, UINT uFilterRes, BOOL bSave = FALSE);

	STATIC INT MsgBox(HWND hParent, UINT uCmd = IDS_AppName, PCTSTR ptzText = _MakeIntRes(IDS_CmdMsg), UINT uType = MB_ICONINFORMATION);

public:
	STATIC BOOL AskBox(HWND hParent, UINT uCmd = IDS_AppName, PCTSTR ptzText = _MakeIntRes(IDS_CmdAsk))
	{
		return MsgBox(hParent, uCmd, ptzText, MB_ICONQUESTION | MB_YESNO) == IDYES;
	}

	STATIC INT ErrBox(HWND hParent, UINT uCmd = IDS_AppName, PCTSTR ptzText = _MakeIntRes(IDS_CmdErr))
	{
		return MsgBox(hParent, uCmd, ptzText, MB_ICONERROR);
	}

	STATIC BOOL IsWM5()
	{
		return ((BYTE) GetSysSpec()) >= 5;
	}

	STATIC BOOL IsPPC()
	{
		return GetSysSpec() & SF_PocketPC;
	}

	STATIC DWORD GetFileAttr(PCTSTR ptzPath)
	{
		WIN32_FILE_ATTRIBUTE_DATA fa;
		fa.dwFileAttributes = 0;
		GetFileAttributesEx(ptzPath, GetFileExInfoStandard, &fa);
		return fa.dwFileAttributes;
	}

	STATIC BOOL ShellOpen(HWND hWnd, PCTSTR ptzPath, PCTSTR ptzParam = NULL)
	{
		SHELLEXECUTEINFO se ={sizeof(SHELLEXECUTEINFO)};
		se.lpVerb = TEXT("Open");
		se.lpFile = ptzPath;
		se.hwnd = hWnd;
		se.lpParameters = ptzParam;
		se.nShow = SW_SHOWNORMAL;
		return ShellExecuteEx(&se);
	}

	STATIC INT GetCtrlRect(HWND hWnd, HWND hCtrl, RECT& rtRect)
	{
		GetWindowRect(hCtrl, &rtRect);
		return MapWindowPoints(NULL, hWnd, (PPOINT) &rtRect, 2);
	}

	STATIC INT GetCtrlRect(HWND hWnd, UINT uCtrlID, RECT& rtRect)
	{
		return GetCtrlRect(hWnd, GetDlgItem(hWnd, uCtrlID), rtRect);
	}

	#define PIX_PAD 0
	//#define PIX_PAD (GetSystemMetrics(SM_CXVSCROLL) + 4)
	STATIC UINT ReWidth(HWND hWnd, UINT uCtrlID, UINT uWidth)
	{
		RECT rt;
		HWND hCtrl = GetDlgItem(hWnd, uCtrlID);
		GetCtrlRect(hWnd, hCtrl, rt);
		uWidth -= (rt.left + PIX_PAD);
		MoveWindow(hCtrl, rt.left, rt.top, uWidth, _RectHeight(rt), TRUE);
		return uWidth;
	}

	STATIC UINT ReLeft(HWND hWnd, UINT uCtrlID, UINT uWidth)
	{
		RECT rt;
		HWND hCtrl = GetDlgItem(hWnd, uCtrlID);
		GetCtrlRect(hWnd, hCtrl, rt);
		uWidth -= (_RectWidth(rt) + PIX_PAD);
		MoveWindow(hCtrl, uWidth, rt.top, _RectWidth(rt), _RectHeight(rt), TRUE);
		return uWidth;
	}

	STATIC UINT ReSize(HWND hWnd, UINT uCtrlID, LPARAM lParam)
	{
		RECT rt;
		HWND hCtrl = GetDlgItem(hWnd, uCtrlID);
		GetCtrlRect(hWnd, hCtrl, rt);
		UINT uWidth = LOWORD(lParam) - (rt.left + PIX_PAD);
		UINT uHeight = HIWORD(lParam) - (rt.top + PIX_PAD);
		MoveWindow(hCtrl, rt.left, rt.top, uWidth, uHeight, TRUE);
		return MAKELONG(uWidth, uHeight);
	}

	STATIC INT_PTR DlgBox(HWND hWnd, DLGPROC dpProc, PVOID pvParam = NULL)
	{
		extern HINSTANCE g_hInst;
		STATIC struct
		{
			DLGTEMPLATE dtDlg;
			WCHAR wzMenu[1];
			WCHAR wzClass[1];
			WCHAR wzCaption[12];
			WORD wFontSize;
			WCHAR wzFontName[1];
			WORD wPad[4];
		}
		dlg = 
		{
			{DS_CENTER | WS_POPUP | WS_BORDER | WS_VISIBLE | DS_SETFOREGROUND, 0, 0, 0, 0, 100, 100},
			0, 0, L"CeleCommand", 0, 0, 0
		};
		return DialogBoxIndirectParam(g_hInst, (LPDLGTEMPLATE) &dlg, hWnd, dpProc, (LPARAM) pvParam);
	}

	STATIC BOOL InitDialog(HWND hWnd, DWORD dwFlags = SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_DONEBUTTON)
	{
		SHINITDLGINFO di;
		di.hDlg = hWnd;
		di.dwMask = SHIDIM_FLAGS;
		di.dwFlags = dwFlags;
		return SHInitDialog(&di);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
