


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Header
#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include <AygShell.h>

#define IDC_ListView 13254
extern HINSTANCE g_hInst;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CMenu class
class CMenu
{
protected:
	HMENU m_hMenu;

public:
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
		mi.cbSize= sizeof(MENUITEMINFO);
		mi.fMask = MIIM_STATE;
		return GetMenuItemInfo(m_hMenu, uCmd, FALSE, &mi) && (mi.fState & MFS_CHECKED);
	}

	BOOL GetText(UINT uCmd, PTSTR ptzStr)
	{
		MENUITEMINFO mi;
		mi.cbSize= sizeof(MENUITEMINFO);
		mi.fMask = MIIM_TYPE;
		mi.cch = MAX_NAME;
		mi.dwTypeData = ptzStr;
		return GetMenuItemInfo(m_hMenu, uCmd, FALSE, &mi);
	}

	BOOL Popup(HWND hParent, INT iLeft, INT iTop, UINT uFlags = 0)
	{
		return TrackPopupMenuEx(m_hMenu, uFlags, iLeft, iTop, hParent, NULL);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CMenuBar class
class CMenuBar
{
protected:
	HWND m_hMenuBar;

public:
	operator HWND()
	{
		return m_hMenuBar;
	}

	operator HMENU()
	{
		return (HMENU) SendMessage(m_hMenuBar, SHCMBM_GETMENU, 0, 0);
	}

	HWND operator =(HWND hMenuBar)
	{
		return m_hMenuBar = hMenuBar;
	}

	HWND Attach(HWND hParent)
	{
		return m_hMenuBar = SHFindMenuBar(hParent);
	}

	HWND Create(HWND hParent, UINT uBarRes)
	{
		SHMENUBARINFO mb = {0};
		mb.cbSize = sizeof(SHMENUBARINFO);
		mb.hwndParent = hParent;
		mb.nToolBarId = uBarRes;
		mb.hInstRes = g_hInst;
		SHCreateMenuBar(&mb);
		return m_hMenuBar = mb.hwndMB;
	}

	HMENU GetSubMenu(UINT uID)
	{
		return (HMENU) SendMessage(m_hMenuBar, SHCMBM_GETSUBMENU, 0, uID);
	}

	DWORD OverrideKey(WPARAM wParam = VK_TBACK)
	{
		return SendMessage(m_hMenuBar, SHCMBM_OVERRIDEKEY, wParam, MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY, SHMBOF_NODEFAULT | SHMBOF_NOTIFY));
	}

	BOOL SetButtonText(UINT uCmd, PTSTR ptzStr)
	{
		TBBUTTONINFO tb;
		tb.cbSize = sizeof(TBBUTTONINFO);
		tb.dwMask = TBIF_TEXT;
		tb.pszText = ptzStr;
		return SendMessage(m_hMenuBar, TB_SETBUTTONINFO, uCmd, (LPARAM) &tb);
	}

	BOOL SetButtonState(UINT uCmd, BYTE bState = TBSTATE_ENABLED)
	{
		TBBUTTONINFO tb;
		tb.cbSize = sizeof(TBBUTTONINFO);
		tb.dwMask = TBIF_STATE;
		tb.fsState = bState;
		return SendMessage(m_hMenuBar, TB_SETBUTTONINFO, uCmd, (LPARAM) &tb);
	}

	BOOL EnableButton(UINT uCmd, BOOL bEnable = TRUE)
	{
#ifdef WIN32_PLATFORM_WFSP
		return SetButtonState(uCmd, bEnable ? TBSTATE_ENABLED : 0);
#else
		return SendMessage(m_hMenuBar, TB_ENABLEBUTTON, uCmd, bEnable);
#endif
	}

	BOOL HideButton(UINT uCmd, BOOL bHide = TRUE)
	{
#ifdef WIN32_PLATFORM_WFSP
		return SetButtonState(uCmd, bHide ? TBSTATE_HIDDEN : 0);
#else
		return SendMessage(m_hMenuBar, TB_HIDEBUTTON, uCmd, bHide);
#endif
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CListView class
#ifndef LVS_EX_TRANSPARENTBKGND
#define LVS_EX_TRANSPARENTBKGND 0x00400000
#endif
#define LVS_DEFAULT		(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS)
#define LVS_EX_DEFAULT	(LVS_EX_GRADIENT | LVS_EX_NOHSCROLL | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE)
class CListView
{
protected:
	UINT m_uColRes;
	HWND m_hListView;

public:
	operator HWND()
	{
		return m_hListView;
	}

	HWND operator =(HWND hListView)
	{
		return m_hListView = hListView;
	}

	HWND Create(HWND hParent, DWORD dwStyle, DWORD dwExStyle)
	{
		m_hListView = CreateWindow(WC_LISTVIEW, NULL, dwStyle, 0, 0, 0, 0, hParent, (HMENU) IDC_ListView, g_hInst, NULL);
		SetExStyle(dwExStyle);
		return m_hListView;
	}

	HWND Create(HWND hParent, UINT uColRes, DWORD dwStyle = LVS_DEFAULT, DWORD dwExStyle = LVS_EX_DEFAULT)
	{
		Create(hParent, dwStyle, dwExStyle);
		SetColText(uColRes);
		return m_hListView;
	}

	VOID SetColText(UINT uColRes)
	{
		INT i = 0;
		LVCOLUMN lvc;
		TCHAR tzStr[MAX_STR];
		lvc.mask = LVCF_TEXT;
		for (PTSTR p = tzStr, q = _GetStr(uColRes); *q; q++)
		{
			if (*q == '|')
			{
				*q = 0;
				lvc.pszText = UStrChr(p, ',');
				lvc.pszText = lvc.pszText ? (lvc.pszText + 1) : p;
				ListView_InsertColumn(m_hListView, i++, &lvc);
				p = q + 1;
			}
		}
		m_uColRes = uColRes;
	}

	VOID SetColWidth(INT iWidth)
	{
		INT i = 0;
		TCHAR tzStr[MAX_STR];
		for (PTSTR p = tzStr, q = _GetStr(m_uColRes); *q; q++)
		{
			if (*q == '|')
			{
				*q = 0;
				INT w = UStrToInt(p);
				p = UStrChr(p, ',');			
				if (p && (p[-1] == '%'))
				{
					w = (w * iWidth) / 100;
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
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
