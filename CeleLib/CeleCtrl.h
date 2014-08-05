


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleCtrl 2.0.204
// Copyright (C) Yonsm 2008-2010, All Rights Reserved.
#pragma once
#include "UniBase.h"
#include "CeleUtil.h"

#include <CommDlg.h>

#ifdef WINCE
#include <AygShell.h>

#pragma comment(lib, "AygShell.lib")
#if (_WIN32_WCE < 0x0500)
#pragma comment(lib, "CCrtRtti.lib")
#endif
#endif

#ifndef IDS_AppName
//#define IDS_AppName		800
//#define IDS_AppDesc		801
//#define IDS_Copyright		802
//#define IDS_CmdMsg		803
//#define IDS_CmdErr		804
//#define IDS_CmdAsk		805

//#define IDS_Open			900
//#define IDS_Save			901
//#define IDS_PromptFile	902

//#include "Resource.h"
#pragma message("CeleCtrl: IDS_AppName not defined. Resource.h should be included")
#endif

#define IDC_ListView		43295
#define IDC_StatusBar		43296
#define IDC_ClientView		43297

#ifndef DT_WORD_ELLIPSIS
#define DT_WORD_ELLIPSIS	0x00040000
#endif

#ifndef SetWindowLongPtr
#define SetWindowLongPtr	SetWindowLong
#define GetWindowLongPtr	GetWindowLong
#define GWLP_WNDPROC		GWL_WNDPROC
#define GWLP_HINSTANCE		GWL_HINSTANCE
#define GWLP_HWNDPARENT		GWL_HWNDPARENT
#define GWLP_USERDATA		GWL_USERDATA
#define GWLP_ID				GWL_ID
#endif

#define SWP_SHOW			(SWP_NOSIZE | SWP_NOMOVE | SWP_NOREPOSITION | SWP_SHOWWINDOW)
#define SWP_HIDE			(SWP_NOSIZE | SWP_NOMOVE | SWP_NOREPOSITION | SWP_HIDEWINDOW)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleMenu
class CeleMenu
{
protected:
	HMENU m_hMenu;

public:
	CeleMenu(HMENU hMenu = NULL)
	{
		m_hMenu = hMenu;
	}

	HMENU operator =(HMENU hMenu)
	{
		return m_hMenu = hMenu;
	}

	operator HMENU()
	{
		return m_hMenu;
	}

	BOOL Enable(UINT uCmd, BOOL bEnable = TRUE)
	{
		return EnableMenuItem(m_hMenu, uCmd, bEnable ? MF_ENABLED : MF_GRAYED);
	}

	BOOL Check(UINT uCmd, BOOL bChecked = TRUE)
	{
		return CheckMenuItem(m_hMenu, uCmd, bChecked ? MF_CHECKED : MF_UNCHECKED);
	}

	BOOL CheckRadio(UINT uFirst, UINT uLast, UINT uCmd)
	{
		return CheckMenuRadioItem(m_hMenu, uFirst, uLast, uCmd, MF_BYCOMMAND);
	}

	BOOL IsChecked(UINT uCmd)
	{
		MENUITEMINFO mi;
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_STATE;
		return GetMenuItemInfo(m_hMenu, uCmd, FALSE, &mi) && (mi.fState & MFS_CHECKED);
	}

	BOOL IsExist(UINT uCmd)
	{
		MENUITEMINFO mi;
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = 0;
		return GetMenuItemInfo(m_hMenu, uCmd, FALSE, &mi);
	}

	UINT GetItemCount()
	{
		UINT nCount = 0;
		MENUITEMINFO mi;
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = 0;
		while (GetMenuItemInfo(m_hMenu, nCount, TRUE, &mi))
		{
			nCount++;
		}
		return nCount;
	}

	BOOL GetText(UINT uCmd, PTSTR ptzStr)
	{
		MENUITEMINFO mi;
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_TYPE;
		mi.cch = MAX_NAME;
		mi.dwTypeData = ptzStr;
		ptzStr[0] = 0;
		return GetMenuItemInfo(m_hMenu, uCmd, FALSE, &mi);
	}

	BOOL SetText(UINT uCmd, PCTSTR ptzStr)
	{
		MENUITEMINFO mi;
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_TYPE;
		mi.fType = MFT_STRING;
		mi.dwTypeData = (PTSTR) ptzStr;
		return SetMenuItemInfo(m_hMenu, uCmd, FALSE, &mi);
	}

	BOOL Popup(HWND hParent, INT iLeft, INT iTop, UINT uFlags = 0)
	{
		return TrackPopupMenuEx(m_hMenu, uFlags, iLeft, iTop, hParent, NULL);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleMenuBar
#ifdef WINCE
#ifndef IDC_LSK
#ifndef TBIF_BYINDEX
#pragma message("CeleMenuBar: IDC_LSK is not defined. You should use IDOK & IDCANCEL as soft key command.")
#endif
#define IDC_LSK			IDOK
#define IDC_RSK			IDCANCEL
#endif
#if TBIF_BYINDEX
#define _MBIF(x)		((x) | TBIF_BYINDEX)
#define _MBID(x)		(x)
#else
#define _MBIF(x)		(x)
#define _MBID(x)		(IDC_LSK + (x))
#endif
class CeleMenuBar
{
protected:
	HWND m_hMenuBar;

public:
	CeleMenuBar(HWND hMenuBar = NULL)
	{
		m_hMenuBar = hMenuBar;
	}

	operator HWND()
	{
		return m_hMenuBar;
	}

	HWND operator =(HWND hMenuBar)
	{
		return m_hMenuBar = hMenuBar;
	}

	HWND Create(HWND hParent, UINT uBarRes = 0, DWORD dwFlags = 0)
	{
		SHMENUBARINFO mb = {0};
		mb.cbSize = sizeof(SHMENUBARINFO);
		mb.hwndParent = hParent;
		mb.hInstRes = g_hInst;
		mb.nToolBarId = uBarRes;
		if (uBarRes)
		{
#ifdef TBIF_BYINDEX
			mb.dwFlags = dwFlags | SHCMBF_HMENU;
#else
			mb.dwFlags = dwFlags;
#endif
		}
		else
		{
			mb.dwFlags = dwFlags | SHCMBF_EMPTYBAR;
		}
		SHCreateMenuBar(&mb);
		_Verify(m_hMenuBar = mb.hwndMB);

		if (CeleUtil::IsPPC())
		{
			RECT rtWnd;
			RECT rtMenuBar;
			GetWindowRect(hParent, &rtWnd);
			GetWindowRect(m_hMenuBar, &rtMenuBar);
			MoveWindow(hParent, rtWnd.left, rtWnd.top, _RectWidth(rtWnd), _RectHeight(rtWnd) - _RectHeight(rtMenuBar), FALSE);
		}
		return m_hMenuBar;
	}

	HMENU GetMenu(BOOL bRight = TRUE)
	{
#ifdef TBIF_BYINDEX
		TBBUTTON tb;
		tb.dwData = NULL;
		SendMessage(m_hMenuBar, TB_GETBUTTON, _MBID(bRight), (LPARAM) &tb);
		return (HMENU) tb.dwData;
#else
		return (HMENU) SendMessage(m_hMenuBar, SHCMBM_GETSUBMENU, 0, _MBID(bRight));
#endif
	}

	DWORD OverrideKey(WPARAM wParam = VK_TBACK, LPARAM lParam = MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY, SHMBOF_NODEFAULT | SHMBOF_NOTIFY))
	{
		return SendMessage(m_hMenuBar, SHCMBM_OVERRIDEKEY, wParam, lParam);
	}

	BOOL GetButtonText(PTSTR ptzStr, BOOL bRight = TRUE)
	{
		TBBUTTONINFO tb;
		tb.cbSize = sizeof(TBBUTTONINFO);
		tb.dwMask = _MBIF(TBIF_TEXT);
		tb.pszText = ptzStr;
		tb.cchText = MAX_PATH;
		return SendMessage(m_hMenuBar, TB_GETBUTTONINFO, _MBID(bRight), (LPARAM) &tb);
	}

	BOOL SetButtonText(PCTSTR ptzStr, BOOL bRight = TRUE)
	{
		TBBUTTONINFO tb;
		tb.cbSize = sizeof(TBBUTTONINFO);
		tb.dwMask = _MBIF(TBIF_TEXT);
		tb.pszText = (PTSTR) ptzStr;
		return SendMessage(m_hMenuBar, TB_SETBUTTONINFO, _MBID(bRight), (LPARAM) &tb);
	}

	BYTE GetButtonState(BOOL bRight = TRUE)
	{
		TBBUTTONINFO tb;
		tb.cbSize = sizeof(TBBUTTONINFO);
		tb.dwMask = _MBIF(TBIF_STATE);
		SendMessage(m_hMenuBar, TB_GETBUTTONINFO, _MBID(bRight), (LPARAM) &tb);
		return tb.fsState;
	}

	BOOL SetButtonState(BYTE bState = TBSTATE_ENABLED, BOOL bRight = TRUE)
	{
		TBBUTTONINFO tb;
		tb.cbSize = sizeof(TBBUTTONINFO);
		tb.dwMask = _MBIF(TBIF_STATE);
		tb.fsState = bState;
		return SendMessage(m_hMenuBar, TB_SETBUTTONINFO, _MBID(bRight), (LPARAM) &tb);
	}

	BOOL EnableButton(BOOL bEnable = TRUE, BOOL bRight = TRUE)
	{
		return SetButtonState(bEnable ? TBSTATE_ENABLED : 0, bRight);
	}

	BOOL HideButton(BOOL bHide = TRUE, BOOL bRight = TRUE)
	{
		return SetButtonState(bHide ? TBSTATE_HIDDEN : 0, bRight);
	}

	BOOL PopupMenu(HMENU hMenu, BOOL bRight = TRUE)
	{
		BYTE bState = GetButtonState(bRight);
		if (bState & TBSTATE_PRESSED)
		{
			keybd_event(VK_ESCAPE, 0, KEYEVENTF_SILENT, 0);
			keybd_event(VK_ESCAPE, 0, KEYEVENTF_SILENT | KEYEVENTF_KEYUP, 0);
		}
		else
		{
			RECT rt;
			GetWindowRect(m_hMenuBar, &rt);
			SetButtonState(bState | TBSTATE_PRESSED, bRight);
			TrackPopupMenuEx(hMenu, bRight ? (TPM_RIGHTALIGN | TPM_BOTTOMALIGN) : (TPM_LEFTALIGN | TPM_BOTTOMALIGN),
				bRight ? rt.right : rt.left, rt.top, GetParent(m_hMenuBar), NULL);
		}
		return SetButtonState(bState & ~TBSTATE_PRESSED, bRight);
	}
};
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleListBox
class CeleListBox
{
protected:
	HWND m_hListBox;

public:
	CeleListBox(HWND hListBox)
	{
		m_hListBox = hListBox;
	}

	operator HWND()
	{
		return m_hListBox;
	}

	HWND operator =(HWND hListBox)
	{
		return m_hListBox = hListBox;
	}

public:
	INT InitList(HWND hWnd, UINT uCtrlID, PTSTR ptzStr, UINT iSel = 0)
	{
		for (PTSTR p = ptzStr, q = ptzStr; *q; q++)
		{
			if (*q == '|')
			{
				*q = 0;
				SendMessage(m_hListBox, LB_ADDSTRING, 0, (LPARAM) p);
				p = q + 1;
			}
		}

		return SendMessage(m_hListBox, LB_SETCURSEL, iSel, 0);
	}

	INT InitList(HWND hWnd, UINT uCtrlID, UINT uStr, UINT iSel = 0)
	{
		TCHAR tzStr[MAX_STR];
		TStrLoad(uStr, tzStr);
		return InitList(hWnd, uCtrlID, tzStr, iSel);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleListView
#ifndef LVS_EX_TRANSPARENTBKGND
#define LVS_EX_TRANSPARENTBKGND		0x00400000
#endif
#define LVS_DEFAULT					(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_TABSTOP)
#ifdef WINCE
#define LVS_EX_DEFAULT				(LVS_EX_GRADIENT | LVS_EX_NOHSCROLL | LVS_EX_FULLROWSELECT/* | LVS_EX_ONECLICKACTIVATE*/)
#else
#define LVS_EX_DEFAULT				(LVS_EX_FULLROWSELECT/* | LVS_EX_ONECLICKACTIVATE*/)
#endif
class CeleListView
{
protected:
	HWND m_hListView;
	PCTSTR m_ptzColumn;

public:
	operator HWND()
	{
		return m_hListView;
	}

	HWND operator =(HWND hListView)
	{
		return m_hListView = hListView;
	}

	HWND Create(HWND hParent, DWORD dwStyle = LVS_DEFAULT, DWORD dwExStyle = LVS_EX_DEFAULT)
	{
		m_hListView = CreateWindow(WC_LISTVIEW, NULL, dwStyle, 0, 0, 0, 0, hParent, (HMENU) IDC_ListView, g_hInst, NULL);
		SetExStyle(dwExStyle);
		return m_hListView;
	}

	HWND Create(UINT uColRes, HWND hParent, DWORD dwStyle = LVS_DEFAULT, DWORD dwExStyle = LVS_EX_DEFAULT)
	{
		return Create(TStrGet(uColRes), hParent, dwStyle, dwExStyle);
	}

	HWND Create(PCTSTR ptzColumn, HWND hParent, DWORD dwStyle = LVS_DEFAULT, DWORD dwExStyle = LVS_EX_DEFAULT)
	{
		Create(hParent, dwStyle, dwExStyle);
		SetColText(ptzColumn);
		return m_hListView;
	}

	VOID SetColText(PCTSTR ptzColumn)
	{
		TCHAR tzStr[MAX_STR];
		TStrCopy(tzStr, m_ptzColumn = ptzColumn);

		INT i = 0;
		LVCOLUMN lvc;
		lvc.mask = LVCF_TEXT;
		for (PTSTR p = tzStr, q = tzStr; *q; q++)
		{
			if (*q == '|')
			{
				*q = 0;
				lvc.pszText = TStrChr(p, ',');
				lvc.pszText = lvc.pszText ? (lvc.pszText + 1) : p;
				ListView_InsertColumn(m_hListView, i++, &lvc);
				p = q + 1;
			}
		}
	}

	VOID SetColWidth(INT iWidth)
	{
		INT i = 0;
		for (PCTSTR p = m_ptzColumn, q = m_ptzColumn; *q; q++)
		{
			if (*q == '|')
			{
				INT w = TStrToInt(p);
				while (++p < q)
				{
					if (*p == ',')
					{
						if ((p[-1] == '%'))
						{
							w = (w * iWidth) / 100;
						}
						break;
					}
				}
				ListView_SetColumnWidth(m_hListView, i++, w);
				p = q + 1;
			}
		}
	}

	BOOL Resize(INT iLeft, INT iTop, INT iWidth, INT iHeight)
	{
		SetColWidth(iWidth);
		return MoveWindow(m_hListView, iLeft, iTop, iWidth, iHeight, FALSE);
	}

	INT GetSel(INT iIndex = -1)
	{
		return ListView_GetNextItem(m_hListView, iIndex, LVIS_SELECTED);
	}

	INT GetCur()
	{
		return ListView_GetNextItem(m_hListView, -1, LVIS_FOCUSED | LVIS_SELECTED);
	}

	VOID SetCur(INT i = 0)
	{
		ListView_SetItemState(m_hListView, i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		ListView_EnsureVisible(m_hListView, i, FALSE);
	}

	VOID SetExStyle(DWORD dwExStyle = LVS_EX_DEFAULT)
	{
		ListView_SetExtendedListViewStyle(m_hListView, dwExStyle);
	}

	VOID SetExStyle(DWORD dwExStyle, DWORD dwMask)
	{
		ListView_SetExtendedListViewStyleEx(m_hListView, dwMask, dwExStyle);
	}

	UINT GetCount()
	{
		return ListView_GetItemCount(m_hListView);
	}

	VOID SetCount(UINT nCount = 0)
	{
		ListView_SetItemCount(m_hListView, nCount);
	}

	VOID SetCountEx(UINT nCount = 0, DWORD dwFlags = LVSICF_NOINVALIDATEALL)
	{
		ListView_SetItemCountEx(m_hListView, nCount, dwFlags);
	}

	BOOL Clear()
	{
		return ListView_DeleteAllItems(m_hListView);
	}

	BOOL Delete(INT iIndex)
	{
		return ListView_DeleteItem(m_hListView, iIndex);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleMsgBox
class CeleMsgBox
{
public:
	ISTATIC INT ShowMsg(PCTSTR ptzMsg, HWND hParent = GetActiveWindow(), PCTSTR ptzCaption = g_ptzAppName, UINT uType = MB_ICONINFORMATION)
	{
		return MessageBox(hParent, ptzMsg, ptzCaption, uType);
	}

	ISTATIC INT ShowErr(PCTSTR ptzMsg, HWND hParent = GetActiveWindow(), PCTSTR ptzCaption = g_ptzAppName)
	{
		return ShowMsg(ptzMsg, hParent, ptzCaption, MB_ICONERROR);
	}

	ISTATIC BOOL ShowAsk(PCTSTR ptzMsg, HWND hParent = GetActiveWindow(), PCTSTR ptzCaption = g_ptzAppName)
	{
		return ShowMsg(ptzMsg, hParent, ptzCaption, MB_ICONQUESTION | MB_YESNO) == IDYES;
	}

public:
	ISTATIC INT ShowMsg(UINT uText, HWND hParent = GetActiveWindow(), PCTSTR ptzCaption = g_ptzAppName, UINT uType = MB_ICONINFORMATION)
	{
		return ShowMsg(TStrGet(uText), hParent, ptzCaption, uType);
	}

	ISTATIC INT ShowErr(UINT uText, HWND hParent = GetActiveWindow(), PCTSTR ptzCaption = g_ptzAppName)
	{
		return ShowMsg(uText, hParent, ptzCaption, MB_ICONERROR);
	}

	ISTATIC BOOL ShowAsk(UINT uText, HWND hParent = GetActiveWindow(), PCTSTR ptzCaption = g_ptzAppName)
	{
		return ShowMsg(uText, hParent, ptzCaption, MB_ICONQUESTION | MB_YESNO) == IDYES;
	}

#ifdef IDS_CmdMsg
public:
	ISTATIC INT ShowCmdMsg(PCTSTR ptzCmd = NULL, HWND hParent = GetActiveWindow(), PCTSTR ptzMsg = _MakeIntRes(IDS_CmdMsg), UINT uType = MB_ICONINFORMATION)
	{
		if ((ptzCmd == NULL) || (ptzCmd[0] == 0))
		{
			ptzCmd = g_ptzAppName;
		}

		TCHAR tzText[MAX_STR];
		if (_IsIntRes(ptzMsg))
		{
			TStrFormat(tzText, TStrGet((UINT) ptzMsg), ptzCmd);
			ptzMsg = tzText;
		}
		return ShowMsg(ptzMsg, hParent, ptzCmd, uType);
	}

	ISTATIC INT ShowCmdErr(PCTSTR ptzCmd, HWND hParent = GetActiveWindow(), PCTSTR ptzMsg = _MakeIntRes(IDS_CmdErr))
	{
		return ShowCmdMsg(ptzCmd, hParent, ptzMsg, MB_ICONERROR);
	}

	ISTATIC BOOL ShowCmdAsk(PCTSTR ptzCmd, HWND hParent = GetActiveWindow(), PCTSTR ptzMsg = _MakeIntRes(IDS_CmdAsk))
	{
		return ShowCmdMsg(ptzCmd, hParent, ptzMsg, MB_ICONQUESTION | MB_YESNO) == IDYES;
	}

public:
	STATIC INT ShowCmdMsg(HMENU hMenu, UINT uCmd, HWND hParent = GetActiveWindow(), PCTSTR ptzMsg = _MakeIntRes(IDS_CmdMsg), UINT uType = MB_ICONINFORMATION)
	{
		CeleMenu m(hMenu);
		TCHAR tzCmd[MAX_NAME];
		if (!m.GetText(uCmd, tzCmd))
		{
			TStrLoad(uCmd, tzCmd);
		}
		return ShowCmdMsg(tzCmd, hParent, ptzMsg, uType);
	}

	ISTATIC INT ShowCmdErr(HMENU hMenu, UINT uCmd, HWND hParent = GetActiveWindow(), PCTSTR ptzMsg = _MakeIntRes(IDS_CmdErr))
	{
		return ShowCmdMsg(hMenu, uCmd, hParent, ptzMsg, MB_ICONERROR);
	}

	ISTATIC BOOL ShowCmdAsk(HMENU hMenu, UINT uCmd, HWND hParent = GetActiveWindow(), PCTSTR ptzMsg = _MakeIntRes(IDS_CmdAsk))
	{
		return ShowCmdMsg(hMenu, uCmd, hParent, ptzMsg, MB_ICONQUESTION | MB_YESNO) == IDYES;
	}

public:
	STATIC INT ShowCmdMsg(UINT uCmd, HWND hParent = GetActiveWindow(), PCTSTR ptzMsg = _MakeIntRes(IDS_CmdMsg), UINT uType = MB_ICONINFORMATION)
	{
		TCHAR tzCmd[MAX_NAME];
		CeleMenuBar mb(SHFindMenuBar(hParent));
		CeleMenu m(mb.GetMenu(TRUE));
		if (!m.GetText(uCmd, tzCmd))
		{
			m = mb.GetMenu(FALSE);
			if (!m.GetText(uCmd, tzCmd))
			{
				TStrLoad(uCmd, tzCmd);
			}
		}
		return ShowCmdMsg(tzCmd, hParent, ptzMsg, uType);
	}

	ISTATIC INT ShowCmdErr(UINT uCmd = IDS_AppName, HWND hParent = GetActiveWindow(), PCTSTR ptzMsg = _MakeIntRes(IDS_CmdErr))
	{
		return ShowCmdMsg(uCmd, hParent, ptzMsg, MB_ICONERROR);
	}

	ISTATIC BOOL ShowCmdAsk(UINT uCmd = IDS_AppName, HWND hParent = GetActiveWindow(), PCTSTR ptzMsg = _MakeIntRes(IDS_CmdAsk))
	{
		return ShowCmdMsg(uCmd, hParent, ptzMsg, MB_ICONQUESTION | MB_YESNO) == IDYES;
	}
#endif
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleDialog
#ifdef WINCE
#define DS_DEFAULT (DS_CENTER | WS_POPUP | WS_BORDER | WS_VISIBLE | DS_SETFOREGROUND)
#define DS_EX_DEFAULT (WS_EX_CAPTIONOKBTN)
#else
#define DS_DEFAULT (DS_CENTER | WS_POPUP | WS_BORDER | WS_VISIBLE | DS_SETFOREGROUND | WS_SYSMENU | WS_CAPTION)
#define DS_EX_DEFAULT (0)
#endif
struct CeleTempDlg
{
	DLGTEMPLATE dtDlg;
	WCHAR wzMenu[1];
	WCHAR wzClass[1];
	WCHAR wzCaption[MAX_NAME];
	WORD wFontSize;
	WCHAR wzFontName[1];

	FINLINE CeleTempDlg()
	{
	}

	FINLINE CeleTempDlg(PCTSTR ptzCaption, UINT nWidth = 0, UINT nHeight = 0, DWORD dwStyle = DS_DEFAULT, DWORD dwExStyle = DS_EX_DEFAULT)
	{
		_Zero(this);
		dtDlg.cx = nWidth;
		dtDlg.cy = nHeight;
		dtDlg.style = dwStyle;
		dtDlg.dwExtendedStyle = dwExStyle;
		TStrToWStr(wzCaption, ptzCaption, MAX_NAME);
	}

	FINLINE INT_PTR Show(HWND hParent, DLGPROC pProc, PVOID pvParam = NULL)
	{
		return DialogBoxIndirectParam(g_hInst, &dtDlg, hParent, pProc, (LPARAM) pvParam);
	}
};

class CeleDialog
{
public:
	ISTATIC INT_PTR DlgBox(HWND hParent, DLGPROC pProc, PVOID pvParam = NULL,
		PCTSTR ptzCaption = g_ptzAppName, UINT nWidth = 0, UINT nHeight = 0, DWORD dwStyle = DS_DEFAULT, DWORD dwExStyle = DS_EX_DEFAULT)
	{
		CeleTempDlg dlg(ptzCaption, nWidth, nHeight, dwStyle, dwExStyle);
		return dlg.Show(hParent, pProc, pvParam);
	}

#ifdef WINCE
	ISTATIC BOOL InitDlg(HWND hWnd, DWORD dwFlags = SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_DONEBUTTON)
	{
		SHINITDLGINFO di;
		di.hDlg = hWnd;
		di.dwMask = SHIDIM_FLAGS;
		di.dwFlags = dwFlags;
		return SHInitDialog(&di);
	}

	ISTATIC HWND InitDlgBar(HWND hWnd, UINT uMenuBar, BOOL bOverBack = FALSE, BYTE bRSK= TBSTATE_ENABLED, BYTE bLSK = TBSTATE_ENABLED, DWORD dwFlags = SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_DONEBUTTON)
	{
		CeleMenuBar mb;
		mb.Create(hWnd, uMenuBar);

		if (bOverBack)
		{
			mb.OverrideKey(VK_TBACK);
		}
		if (bLSK!= TBSTATE_ENABLED)
		{
			mb.SetButtonState(bLSK, FALSE);
		}
		if (bRSK != TBSTATE_ENABLED)
		{
			mb.SetButtonState(bRSK, TRUE);
		}

		InitDlg(hWnd, dwFlags);
		return mb;
	}
#endif
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleComDlg
#if (_WIN32_WCE < 0x500)
typedef struct tagOPENFILENAMEEX
{
	// Fields which map to OPENFILENAME
	DWORD			lStructSize;
	HWND			hwndOwner;
	HINSTANCE		hInstance;
	PCTSTR			lpstrFilter;
	PTSTR			lpstrCustomFilter;
	DWORD			nMaxCustFilter;
	DWORD			nFilterIndex;
	PTSTR			lpstrFile;
	DWORD			nMaxFile;
	LPTSTR			lpstrFileTitle;
	DWORD			nMaxFileTitle;
	PCTSTR			lpstrInitialDir;
	PCTSTR			lpstrTitle;
	DWORD			Flags;
	WORD			nFileOffset;
	WORD			nFileExtension;
	PCTSTR			lpstrDefExt;
	LPARAM			lCustData;
	LPOFNHOOKPROC	lpfnHook;
	PCTSTR			lpTemplateName;

	// Extended fields
	DWORD			dwSortOrder;
	DWORD			ExFlags;
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

#define OFN_COMMON (0x11000000)
class CeleComDlg
{
private:
	typedef BOOL (*PGETFILENAME)(LPOPENFILENAMEEX pOfn);
	typedef BOOL (*TGETFILENAME)(BOOL bSave, LPOPENFILENAMEEX pOfn);

public:
	STATIC BOOL BrowseFile(HWND hParent, PTSTR ptzPath, UINT uFilterRes = 0, BOOL bSave = FALSE)
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

#define _SAVEDIR
#ifdef _SAVEDIR
		// Read initialize folder
		CeleReg reg;
		TCHAR tzInitDir[MAX_PATH];
		if (reg.GetStr(TEXT("BrowseFile"), tzInitDir))
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
		if (TStrLoad(uFilterRes, tzFilter))
		{
			ofn.lpstrFilter = TStrRep(tzFilter, '|', 0);
		}

		// Parse default extension
		TCHAR tzDefExt[MAX_NAME];
		PTSTR p = TStrChr(tzFilter, 0) + 1;
		if (p = TStrChr(p, '.'))
		{
			TStrCopy(tzDefExt, p);
			if (p = TStrChr(tzDefExt, ';'))
			{
				*p = 0;
			}
			ofn.lpstrDefExt = tzDefExt;
		}

#ifdef IDS_Open
		// Get title
		TCHAR tzTitle[MAX_NAME];
		ofn.lpstrTitle = tzTitle;
		TStrLoad(bSave ? IDS_Save : IDS_Open, tzTitle);
#endif

#ifdef WINCE
		// Module name and procedure table
		enum {MP_AygShell, MP_RFileShell, MP_tGetFile, MP_CoreDll};
		const static struct {PCTSTR ptzModName; PCTSTR ptzOpenProc; PCTSTR ptzSaveProc;} c_sModProc[] =
		{
			{TEXT("AygShell"), TEXT("GetOpenFileNameEx"), TEXT("GetSaveFileNameEx")},
			{TEXT("RFileShell"), TEXT("RShellGetOpenFileName"), TEXT("RShellGetSaveFileName")},
			{TEXT("tGetFile"), TEXT("tGetFile"), TEXT("tGetFile")},
			{TEXT("CoreDll"), TEXT("GetOpenFileNameW"), TEXT("GetSaveFileNameW")},
		};

		// Try all library
		UINT i = 0;
		BOOL bResult = FALSE;
		for (; i < _NumOf(c_sModProc); i++)
		{
			HMODULE hModule = LoadLibrary(c_sModProc[i].ptzModName);
			if ((hModule == NULL) && (i == MP_RFileShell))
			{
				// Lookup RESCO Explorer install folder
				TCHAR tzModule[MAX_PATH];
				CeleReg rfs(TEXT("SOFTWARE\\Apps\\Resco Explorer"), HKEY_LOCAL_MACHINE);
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
		if ((i == MP_CoreDll) && !CeleUtil::IsPPC() && !CeleUtil::IsWM5() && !bResult)
		{
			TStrFormat(ofn.lpstrFile, TEXT("\\My Documents\\%s%s"), g_ptzAppName, ofn.lpstrDefExt);
#ifdef IDS_PromptFile
			TCHAR tzInfo[MAX_STR];
			TStrFormat(tzInfo, TStrGet(IDS_PromptFile), ofn.lpstrFile);
			bResult = (MessageBox(hParent, tzInfo, tzTitle, MB_ICONQUESTION | MB_YESNO) == IDYES);
#else
			bResult = TRUE;
#endif
		}
#else
		ofn.lStructSize = sizeof(OPENFILENAME);
		BOOL bResult = bSave ? GetSaveFileName(LPOPENFILENAME(&ofn)) : GetOpenFileName(LPOPENFILENAME(&ofn));
#endif

		if (bResult)
		{
			// Set control text
			if (_IsIntRes(ptzPath))
			{
				SetFocus(GetDlgItem(hParent, (UINT) ptzPath));
				SetDlgItemText(hParent, (UINT) ptzPath, ofn.lpstrFile);
			}

#ifdef _SAVEDIR
			// Save initialize folder
			p = TStrRChr(ofn.lpstrFile, '\\');
			if (p++)
			{
				TCHAR c = *p;
				*p = 0;
				reg.SetStr(TEXT("BrowseFile"), ofn.lpstrFile);
				*p = c;
			}
#endif
		}

		return bResult;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleCtrl
class CeleCtrl: public CeleMsgBox, public CeleDialog, public CeleComDlg
{
public:
	ISTATIC INT GetCtrlRect(HWND hWnd, HWND hCtrl, RECT& rtRect)
	{
		GetWindowRect(hCtrl, &rtRect);
		return MapWindowPoints(NULL, hWnd, (PPOINT) &rtRect, 2);
	}

	ISTATIC INT GetCtrlRect(HWND hWnd, UINT uCtrlID, RECT& rtRect)
	{
		return GetCtrlRect(hWnd, GetDlgItem(hWnd, uCtrlID), rtRect);
	}

	//#define PIX_PAD 10
	#define PIX_PAD GetSystemMetrics(SM_CXVSCROLL)
	ISTATIC UINT ReWidth(HWND hWnd, UINT uCtrlID, UINT uWidth)
	{
		RECT rt;
		HWND hCtrl = GetDlgItem(hWnd, uCtrlID);
		GetCtrlRect(hWnd, hCtrl, rt);
		uWidth -= (rt.left + PIX_PAD);
		MoveWindow(hCtrl, rt.left, rt.top, uWidth, _RectHeight(rt), TRUE);
		return uWidth;
	}

	ISTATIC UINT ReLeft(HWND hWnd, UINT uCtrlID, UINT uWidth)
	{
		RECT rt;
		HWND hCtrl = GetDlgItem(hWnd, uCtrlID);
		GetCtrlRect(hWnd, hCtrl, rt);
		uWidth -= (_RectWidth(rt) + PIX_PAD);
		MoveWindow(hCtrl, uWidth, rt.top, _RectWidth(rt), _RectHeight(rt), TRUE);
		return uWidth;
	}

	ISTATIC UINT ReSize(HWND hWnd, UINT uCtrlID, LPARAM lParam)
	{
		RECT rt;
		HWND hCtrl = GetDlgItem(hWnd, uCtrlID);
		GetCtrlRect(hWnd, hCtrl, rt);
		UINT uWidth = LOWORD(lParam) - (rt.left + PIX_PAD);
		UINT uHeight = HIWORD(lParam) - (rt.top + PIX_PAD);
		MoveWindow(hCtrl, rt.left, rt.top, uWidth, uHeight, TRUE);
		return MAKELONG(uWidth, uHeight);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleActivate
#define _HandleActivate(hWnd, uMsg, wParam, lParam)	\
	STATIC SHACTIVATEINFO s_saInfo = {sizeof(SHACTIVATEINFO)};	\
	switch (uMsg)	\
	{	\
	case WM_ACTIVATE: SHHandleWMActivate(hWnd, wParam, lParam, &s_saInfo, FALSE); break;	\
	case WM_SETTINGCHANGE: SHHandleWMSettingChange(hWnd, wParam, lParam, &s_saInfo);  break;	\
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleScroll
#ifdef WINCE
struct CeleScroll
{
	//protected:
	SIZE m_szAll;
	SIZE m_szClient;
	POINT m_ptOffset;
	SHACTIVATEINFO m_saInfo;

	CeleScroll()
	{
		m_szAll.cx = m_szAll.cy = 0;
		m_ptOffset.x = m_ptOffset.y = 0;
		m_szClient.cx = m_szClient.cy = 0;
	}

	//public:
	BOOL HandleScroll(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
			//case WM_CREATE:
		case WM_INITDIALOG:
			POINT ptMin, ptMax;
			ptMax.x = ptMax.y = 0;
			ptMin.x = ptMin.y = 0x7FFFFFFF;
			for (HWND hCtrl = GetWindow(hWnd, GW_CHILD); hCtrl; hCtrl = GetWindow(hCtrl, GW_HWNDNEXT))
			{
				RECT rt;
				GetWindowRect(hCtrl, &rt);
				MapWindowPoints(NULL, hWnd, (PPOINT) &rt, 2);
				if (rt.left < ptMin.x) ptMin.x = rt.left;
				if (rt.top < ptMin.y) ptMin.y = rt.top;
				if (rt.right > ptMax.x) ptMax.x = rt.right;
				if (rt.bottom > ptMax.y) ptMax.y = rt.bottom;
			}
			m_szAll.cx = ptMax.x + ptMin.x - GetSystemMetrics(SM_CXVSCROLL);
			m_szAll.cy = ptMax.y + ptMin.y;
			UpdateScroll(hWnd);

			UMemSet(&m_saInfo, 0, sizeof(m_saInfo));
			m_saInfo.cbSize = sizeof(SHACTIVATEINFO);
			return TRUE;

		case WM_ACTIVATE:
			SHHandleWMActivate(hWnd, wParam, lParam, &m_saInfo, FALSE);
			break;

		case WM_SETTINGCHANGE:
			SHHandleWMSettingChange(hWnd, wParam, lParam, &m_saInfo);
			break;

		case WM_HSCROLL:
			ScrollWindow(hWnd, SB_HORZ, LOWORD(wParam), (INT&) m_ptOffset.x, m_szAll.cx, m_szClient.cx);
			break;

		case WM_VSCROLL:
			ScrollWindow(hWnd, SB_VERT, LOWORD(wParam), (INT&) m_ptOffset.y, m_szAll.cy, m_szClient.cy);
			break;

		case WM_SIZE:
			if (m_szAll.cx)
			{
				m_szClient.cx = LOWORD(lParam);
				m_szClient.cy = HIWORD(lParam);
				UpdateScroll(hWnd);
			}
			break;

		case WM_KEYUP:
			if (wParam == VK_LEFT || wParam == VK_RIGHT)
			{
				ScrollWindow(hWnd, SB_HORZ, (wParam == VK_LEFT ? SB_PAGELEFT : SB_PAGERIGHT), (INT&) m_ptOffset.x, m_szAll.cx, m_szClient.cx);
			}
			break;
		}
		return FALSE;
	}

	BOOL HandleScrollEx(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (CeleUtil::IsPPC())
		{
			return HandleScroll(hWnd, uMsg, wParam, lParam);
		}
		else if (uMsg == WM_INITDIALOG)
		{
			SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_VSCROLL);
			return TRUE;
		}
		return FALSE;
	}

	//protected:
	INT UpdateScroll(HWND hWnd)
	{
		ScrollWindowEx(hWnd, m_ptOffset.x, m_ptOffset.y, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE | SW_SCROLLCHILDREN);
		m_ptOffset.x = m_ptOffset.y = 0;

		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMin = 0;
		si.nMax = m_szAll.cx - 1;
		si.nPage = m_szClient.cx;
		si.nPos = m_ptOffset.x;
		si.nTrackPos = 0;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

		//si.cbSize = sizeof(SCROLLINFO);
		//si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		//si.nMin = 0;
		//si.nTrackPos = 0;
		si.nMax = m_szAll.cy - 1;
		si.nPage = m_szClient.cy;
		si.nPos = m_ptOffset.y;
		return SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	}

	INT ScrollWindow(HWND hWnd, INT nType, INT nScrollCode, INT& cxyOffset, INT cxySizeAll, INT cxyClient)
	{
		INT cxyMax = cxySizeAll - cxyClient;
		if (cxyMax < 0)   // Can't scroll, client area is bigger
		{
			return ERROR;
		}

		INT cxySizeLine = cxySizeAll / 100;
		if (cxySizeLine < 1)
		{
			cxySizeLine = 1;
		}
		INT cxySizePage = cxySizeAll / 10;
		if (cxySizePage < 1)
		{
			cxySizePage = 1;
		}

		INT cxyScroll = 0;
		BOOL bUpdate = TRUE;

		switch (nScrollCode)
		{
		case SB_TOP:		// top or all left
			cxyScroll = cxyOffset;
			cxyOffset = 0;
			break;

		case SB_BOTTOM:		// bottom or all right
			cxyScroll = cxyOffset - cxyMax;
			cxyOffset = cxyMax;
			break;

		case SB_LINEUP:		// line up or line left
			if (cxyOffset >= cxySizeLine)
			{
				cxyScroll = cxySizeLine;
				cxyOffset -= cxySizeLine;
			}
			else
			{
				cxyScroll = cxyOffset;
				cxyOffset = 0;
			}
			break;

		case SB_LINEDOWN:	// line down or line right
			if (cxyOffset < cxyMax - cxySizeLine)
			{
				cxyScroll = -cxySizeLine;
				cxyOffset += cxySizeLine;
			}
			else
			{
				cxyScroll = cxyOffset - cxyMax;
				cxyOffset = cxyMax;
			}
			break;

		case SB_PAGEUP:		// page up or page left
			if (cxyOffset >= cxySizePage)
			{
				cxyScroll = cxySizePage;
				cxyOffset -= cxySizePage;
			}
			else
			{
				cxyScroll = cxyOffset;
				cxyOffset = 0;
			}
			break;

		case SB_PAGEDOWN:	// page down or page right
			if (cxyOffset < cxyMax - cxySizePage)
			{
				cxyScroll = -cxySizePage;
				cxyOffset += cxySizePage;
			}
			else
			{
				cxyScroll = cxyOffset - cxyMax;
				cxyOffset = cxyMax;
			}
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_TRACKPOS;
			if (GetScrollInfo(hWnd, nType, &si))
			{
				cxyScroll = cxyOffset - si.nTrackPos;
				cxyOffset = si.nTrackPos;
			}
			break;

		case SB_ENDSCROLL:
		default:
			return ERROR;
		}

		if (!cxyScroll)
		{
			return ERROR;
		}

		SetScrollPos(hWnd, nType, cxyOffset, TRUE);
		if (nType == SB_VERT)
		{
			return ScrollWindowEx(hWnd, 0, cxyScroll, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE | SW_SCROLLCHILDREN);
		}
		else
		{
			return ScrollWindowEx(hWnd, cxyScroll, 0, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE | SW_SCROLLCHILDREN);
		}
	}
};
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
