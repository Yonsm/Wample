


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleAbout 2.0.204
// Copyright (C) Yonsm 2008-2010, All Rights Reserved.
#pragma once
#include "UniBase.h"
#include "CeleAuto.h"

#ifdef WINCE
#include "CeleCtrl.h"
#include <DeviceResolutionAware.h>
#define _ScaleX DRA::SCALEX
#define _ScaleY DRA::SCALEY
#define _AutoFont(d)
#else
#include "CeleUtil.h"
#define _ScaleX
#define _ScaleY
#define _AutoFont(h) CeleAutoFont af(h)
#endif

#ifndef STR_VersionStamp
#pragma message("CeleAbout: STR_VersionStamp not defined. AppDef.h should be included")
#define STR_BuildStamp		TEXT("2010")
#define STR_VersionStamp	TEXT("1.0.0")
#define STR_Web				TEXT("WWW.Yonsm.NET")
#define STR_Mail			TEXT("Yonsm@msn.com")
#define STR_WebCmd			TEXT("http://WWW.Yonsm.NET")
#define STR_MailCmd			TEXT("mailto:Yonsm@msn.com")
#define STR_AppDesc			TEXT("CeleWare Application")
#define STR_Copyright		TEXT("Copyright (C) 2010 Yonsm.NET. All Rights Reserved.")
#endif

#ifndef STR_AboutLink
#define STR_AboutLink		{STR_Mail, STR_MailCmd}, {STR_Web, STR_WebCmd}
#endif

#ifndef IDI_AppIcon
#define IDI_AppIcon			100
#pragma message("CeleAbout: IDI_AppIcon not defined. Resource.h should be included")
#endif

#ifdef WINCE
#ifndef IDR_DlgMenu
#define IDR_DlgMenu			0
#pragma message("CeleAbout: IDR_DlgMenu not defined. You will get an about box with an empty menu bar.")
#endif
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleAbout
#if !defined(WINCE) || (_WIN32_WCE < 0x0500)
#define _COLOR_OfficeXP
#endif
#ifdef _COLOR_OfficeXP
#define COLOR_LineGroup1	0x00FF9966
#define COLOR_LineGroup2	0x00FF9966
#define COLOR_SolidRect1	0x00FF9966
#define COLOR_SolidRect2	0x00CC3333
#define COLOR_LogoRect1		0x00CC3333
#define COLOR_LogoRect2		0x00DDFFFF
#define COLOR_VersionText	0x00BBBBBB
#define COLOR_CopyrightText	0x00000000
#define COLOR_AppNameText	0x00111111
#else
#define COLOR_LineGroup1	GetSysColor(COLOR_ACTIVECAPTION)
#define COLOR_LineGroup2	GetSysColor(COLOR_ACTIVECAPTION)
#define COLOR_SolidRect1	GetSysColor(COLOR_ACTIVECAPTION)
#define COLOR_SolidRect2	GetSysColor(COLOR_MENUTEXT)
#define COLOR_LogoRect1		GetSysColor(COLOR_MENUTEXT)
#define COLOR_LogoRect2		GetSysColor(COLOR_INFOBK)
#define COLOR_VersionText	GetSysColor(COLOR_GRAYTEXT)
#define COLOR_CopyrightText	GetSysColor(COLOR_WINDOWTEXT)
#define COLOR_AppNameText	GetSysColor(COLOR_WINDOWTEXT)
#endif
#ifdef IDS_Copyright
#define ABOUT_AppDesc		TStrGet(IDS_AppDesc)
#define ABOUT_Copyright		TStrGet(IDS_Copyright)
#else
#define ABOUT_AppDesc		STR_AppDesc
#define ABOUT_Copyright		STR_Copyright
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleAbout
class CeleAbout
{
public:
	ISTATIC INT_PTR Show(HWND hParent = GetActiveWindow())
	{
		struct CeleAboutDlg
		{
			DLGTEMPLATE dtDlg;
			WCHAR wzMenu[1];
			WCHAR wzClass[1];
			WCHAR wzCaption[MAX_NAME];
			WORD wFontSize;
			WCHAR wzFontName[1];
		};
		CeleAboutDlg ad = {0};
		ad.dtDlg.cx = 200;
		ad.dtDlg.cy = 123;
		ad.dtDlg.style = WS_POPUP | WS_VISIBLE | WS_BORDER | DS_CENTER | DS_SETFOREGROUND;
		ad.dtDlg.dwExtendedStyle = hParent ? (WS_EX_TOOLWINDOW | WS_EX_TOPMOST) : 0;
		TStrToWStr(ad.wzCaption, g_ptzAppName, MAX_NAME);
#ifdef IDR_DlgMenu
		return DialogBoxIndirectParam(g_hInst, &ad.dtDlg, hParent, AboutDlgProc, (LPARAM) IDR_DlgMenu);
#else
		return DialogBoxIndirectParam(g_hInst, &ad.dtDlg, hParent, AboutDlgProc, 0);
#endif
	}

	STATIC BOOL CALLBACK AboutDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:
#ifdef WINCE
			if (lParam == IDR_DlgMenu)
			{
				CeleMenuBar mb;
				mb.Create(hWnd, IDR_DlgMenu, SHCMBF_HIDESIPBUTTON);
				mb.HideButton();
				mb.OverrideKey();
				CeleCtrl::InitDlg(hWnd);
			}
#else
			if (GetParent(hWnd) == NULL)
			{
#pragma warning(push)
#pragma warning(disable:4244)
				SetClassLongPtr(hWnd, GCLP_HICON, (LONG_PTR) LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_AppIcon)));
#pragma warning(pop)
			}
#endif
			break;

#ifdef WINCE
		case WM_HOTKEY:
			if ((HIWORD(lParam) == VK_ESCAPE) && (LOWORD(lParam) == MOD_KEYUP))
			{
				EndDialog(hWnd, 0);
			}
			break;
#endif

		case WM_ERASEBKGND:
			return OnPaint(hWnd, (HDC) wParam);

		case WM_LBUTTONUP:
			if (LinkFromPoint(lParam) == m_uCurLink)
			{
				OnKeyDown(hWnd, VK_RETURN);
			}
			break;

		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
			UINT uCurLink;
			uCurLink = LinkFromPoint(lParam);
			if (m_uCurLink != uCurLink)
			{
				m_uCurLink = uCurLink;
				DrawHyperLink(hWnd);
			}
#ifndef WINCE
			if ((uMsg == WM_LBUTTONDOWN) && (uCurLink == -1))
			{
				RECT rt;
				POINT ptRef = GetRefPoint(hWnd, rt);
				if ((LOWORD(lParam) > ptRef.x - _ScaleX(24)) &&
					(LOWORD(lParam) < ptRef.x + _ScaleX(24)) &&
					(HIWORD(lParam) > ptRef.y - _ScaleX(24)) &&
					(HIWORD(lParam) < ptRef.y + _ScaleX(24)))
				{
					ReleaseCapture();
					SetCursor(LoadCursor(NULL, IDC_SIZEALL));
					PostMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
				}
				else
				{
					EndDialog(hWnd, 0);
				}
			}
#endif
			break;

		case WM_KEYDOWN:
			OnKeyDown(hWnd, wParam);
			break;

		case WM_KEYUP:
			if (wParam == VK_ESCAPE)
			{
				EndDialog(hWnd, VK_ESCAPE);
			}
			break;

		case WM_GETDLGCODE:
			if (PMSG pMsg = (PMSG) lParam)
			{
				if ((pMsg->message == WM_KEYDOWN) &&
					((pMsg->wParam != VK_RETURN) || (m_uCurLink < m_cLinkCount)))
				{
					SetWindowLong(hWnd, DWL_MSGRESULT, DLGC_WANTALLKEYS);
					return DLGC_WANTALLKEYS;
				}
			}
			break;

		case WM_COMMAND:
			if ((LOWORD(wParam) == IDOK)
#ifdef IDC_LSK
				|| (LOWORD(wParam) == IDC_LSK)
#endif
				|| (LOWORD(wParam) == IDCANCEL))
			{
				return EndDialog(hWnd, wParam);
			}
			break;
		}

		return FALSE;
	}

private:
	ISTATIC POINT GetRefPoint(HWND hWnd, RECT& rtDlg)
	{
		POINT pt;
		GetClientRect(hWnd, &rtDlg);
		pt.x = rtDlg.right / (5 - (rtDlg.right >= _ScaleX(200))),
		pt.y = rtDlg.bottom / (5 - (rtDlg.bottom >= _ScaleY(200)));
		return pt;
	}

	STATIC BOOL OnPaint(HWND hWnd, HDC hDC)
	{
		_AutoFont(hDC);

		// Calculate reference point
		RECT rtDlg;
		POINT ptRef = GetRefPoint(hWnd, rtDlg);
		FillRect(hDC, &rtDlg, (HBRUSH) GetStockObject(WHITE_BRUSH));

		// Group line 1
		DrawLineGroup(hDC, 0, 0, ptRef.x, ptRef.y, COLOR_LineGroup1, _ScaleY(2));

		// Background rect
		RECT rt;
		SetRect(rt, 0, ptRef.y, rtDlg.right, ptRef.y + _ScaleY(36));
		DrawSolidRect(hDC, rt, COLOR_SolidRect1);
		//DrawGlassRect(hDC, rt, 0x00D1AE7A, 0x00B98835, 0x00975F00, 0x00C6A46A);
		rt.top = rt.bottom;
		rt.bottom += _ScaleY(4);
		DrawSolidRect(hDC, rt, COLOR_SolidRect2);

		// Group line 2
		DrawLineGroup(hDC, ptRef.x - _ScaleX(28), rt.bottom + 1, rtDlg.right, rt.bottom + _ScaleY(14), COLOR_LineGroup2, _ScaleY(2));

		// Version text
		SetBkMode(hDC, TRANSPARENT);
		SetTextColor(hDC, COLOR_VersionText);
		if (rtDlg.bottom >= _ScaleY((200)))
		{
			SetRect(rt, ptRef.x - _ScaleX(28), rt.bottom + _ScaleY(16), rtDlg.right - _ScaleX(2), rt.bottom + _ScaleY(32));
			DrawText(hDC, STR_VersionStamp, -1, &rt, DT_SINGLELINE | DT_VCENTER);
			DrawText(hDC, STR_BuildStamp, -1, &rt, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
			//DrawLineGroup(hDC, ptRef.x - _ScaleX(28), rt1.bottom + _ScaleY(32), rtDlg.right, rt1.bottom + _ScaleY(33), 0x00000000);
		}
		else
		{
			SetRect(rt, ptRef.x + _ScaleX(30), ptRef.y, rtDlg.right + _ScaleX(2), ptRef.y + _ScaleY(18));
			DrawText(hDC, STR_VersionStamp, -1, &rt, DT_SINGLELINE | DT_VCENTER);
			rt.top = rt.bottom;
			rt.bottom += _ScaleY(18);
			DrawText(hDC, STR_BuildStamp, -1, &rt, DT_SINGLELINE | DT_VCENTER);
		}

		// Hyper link text
		DrawHyperLink(hDC, ptRef.x - _ScaleX(28), rtDlg.bottom - _ScaleY(36) - GetSystemMetrics(SM_CYSMICON) * m_cLinkCount);

		// Copyright text
		SetTextColor(hDC, COLOR_CopyrightText);
		SetRect(rt, ptRef.x - _ScaleX(28), rtDlg.bottom - _ScaleY(17), rtDlg.right, rtDlg.bottom - _ScaleY(1));
		DrawText(hDC, ABOUT_Copyright, -1, &rt, DT_SINGLELINE | DT_VCENTER);
		rt.bottom = rt.top;
		rt.top -= _ScaleY(16);
		DrawText(hDC, ABOUT_AppDesc, -1, &rt, DT_SINGLELINE | DT_VCENTER);

		// Frame rect
		SetRect(rt, ptRef.x - _ScaleX(28), ptRef.y - _ScaleY(28), ptRef.x + _ScaleX(28), ptRef.y + _ScaleY(28));
		DrawSolidRect(hDC, rt, COLOR_LogoRect1);
		InflateRect(&rt, _ScaleX(-4), _ScaleY(-4));
		DrawSolidRect(hDC, rt, COLOR_LogoRect2);

		// Logo icon
		DrawIconEx(hDC, ptRef.x - _ScaleX(16), ptRef.y - _ScaleY(16), LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_AppIcon)),
			GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0, NULL, DI_NORMAL);

		// Name text
		if (TRUE)
		{
			CeleAutoFont af(hDC, 15, FW_BOLD);
			SetRect(rt, ptRef.x + _ScaleX(30), ptRef.y - _ScaleY(30), rtDlg.right, ptRef.y);
			Draw3DText(hDC, g_ptzAppName, rt, COLOR_AppNameText);
		}
		return TRUE;
	}

	STATIC BOOL OnKeyDown(HWND hWnd, WPARAM nVirtKey)
	{
		switch (nVirtKey)
		{
		case VK_UP:
			if (m_uCurLink == 0)
			{
				return FALSE;
			}
			else if (m_uCurLink >= m_cLinkCount)
			{
				m_uCurLink = m_cLinkCount - 1;
			}
			else
			{
				m_uCurLink--;
			}
			break;

		case VK_TAB:
		case VK_DOWN:
			if (m_uCurLink == m_cLinkCount - 1)
			{
				return FALSE;
			}
			else if (m_uCurLink >= m_cLinkCount)
			{
				m_uCurLink = 0;
			}
			else
			{
				m_uCurLink++;
			}
			break;

		case VK_RETURN:
			if ((m_uCurLink < m_cLinkCount) && m_cLinks[m_uCurLink].ptzOpen)
			{
				return BOOL(CeleUtil::ShellOpen(m_cLinks[m_uCurLink].ptzOpen, m_cLinks[m_uCurLink].ptzParam));
			}

		default:
			return FALSE;
		}

		DrawHyperLink(hWnd);
		return TRUE;
	}

public:
	ISTATIC VOID SetRect(RECT& rtRect, INT iLeft, INT iTop, INT iRight, INT iBottom)
	{
		rtRect.left = iLeft;
		rtRect.top = iTop;
		rtRect.right = iRight;
		rtRect.bottom = iBottom;
	}

	ISTATIC VOID Draw3DText(HDC hDC, PCTSTR ptzText, RECT& rtRect, COLORREF crColor)
	{
		SetTextColor(hDC, 0x00AAAAAA);
		DrawText(hDC, ptzText, -1, &rtRect, DT_SINGLELINE | DT_VCENTER);

		OffsetRect(&rtRect, -2, -2);
		SetTextColor(hDC, crColor);
		DrawText(hDC, ptzText, -1, &rtRect, DT_SINGLELINE | DT_VCENTER);
	}

	ISTATIC VOID DrawLineGroup(HDC hDC, INT iLeft, INT iTop, INT iRight, INT iBottom, COLORREF crColor, UINT nDelta = 2)
	{
		HPEN hPen = (HPEN) SelectObject(hDC, ::CreatePen(PS_SOLID, 0, crColor));
		for (INT j = iTop; j < iBottom; j += nDelta)
		{
			MoveToEx(hDC, iLeft, j, NULL);
			LineTo(hDC, iRight, j);
			if (nDelta > 2)
			{
				MoveToEx(hDC, iLeft, j + 1, NULL);
				LineTo(hDC, iRight, j + 1);
			}
		}
		DeleteObject(SelectObject(hDC, hPen));
	}

	ISTATIC VOID DrawSolidRect(HDC hDC, CONST RECT& rtRect, COLORREF crColor)
	{
		SetBkColor(hDC, crColor);
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rtRect, NULL, 0, NULL);
	}

	ISTATIC VOID DrawGradientRect(HDC hDC, CONST RECT& rtRect, COLORREF crStart, COLORREF crEnd, ULONG ulMode = GRADIENT_FILL_RECT_V)
	{
		TRIVERTEX vt[2];
		vt[0].x = rtRect.left;
		vt[0].y = rtRect.top;
		vt[0].Red = GetRValue(crStart) << 8;
		vt[0].Green = GetGValue(crStart) << 8;
		vt[0].Blue = GetBValue(crStart) << 8;
		vt[0].Alpha = ((BYTE) (crStart >> 24)) << 8;

		vt[1].x = rtRect.right;
		vt[1].y = rtRect.bottom;
		vt[1].Red = GetRValue(crEnd) << 8;
		vt[1].Green = GetGValue(crEnd) << 8;
		vt[1].Blue = GetBValue(crEnd) << 8;
		vt[1].Alpha = ((BYTE) (crStart >> 24)) << 8;

		GRADIENT_RECT rt[1] = {{0, 1}};
		GradientFill(hDC, vt, _NumOf(vt), rt, _NumOf(rt), ulMode);
	}

	ISTATIC VOID DrawGlassRect(HDC hDC, /*CONST */RECT& rtRect, COLORREF crStart1, COLORREF crEnd1, COLORREF crStart2, COLORREF crEnd2)
	{
		INT iHalfHeight = _RectHeight(rtRect) / 2;
		rtRect.bottom -= iHalfHeight;
		DrawGradientRect(hDC, rtRect, crStart1, crEnd1);
		rtRect.top = rtRect.bottom;
		rtRect.bottom += iHalfHeight;
		DrawGradientRect(hDC, rtRect, crStart2, crEnd2);
		rtRect.top -= iHalfHeight;
	}

private:
	ISTATIC VOID DrawHyperLink(HDC hDC, INT iLeft, INT iTop)
	{
		UINT h = GetSystemMetrics(SM_CYSMICON);
		for (UINT i = 0; i < m_cLinkCount; i++)
		{
			m_cLinks[i].rtBound.left = iLeft;
			m_cLinks[i].rtBound.top = iTop + (i * h);
			m_cLinks[i].rtBound.right = 1024;
			m_cLinks[i].rtBound.bottom = m_cLinks[i].rtBound.top + h;
			SetTextColor(hDC, (i == m_uCurLink) ? 0x000000FF : 0x00FF0000);
			DrawText(hDC, m_cLinks[i].ptzText, -1, &m_cLinks[i].rtBound, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
			DrawText(hDC, m_cLinks[i].ptzText, -1, &m_cLinks[i].rtBound, DT_SINGLELINE | DT_VCENTER);
		}
	}

	ISTATIC VOID DrawHyperLink(HWND hWnd)
	{
		if (m_cLinkCount)
		{
			if (HDC hDC = GetDC(hWnd))
			{
				_AutoFont(hDC);
				//SetBkMode(hDC, TRANSPARENT);
				DrawHyperLink(hDC, m_cLinks[0].rtBound.left, m_cLinks[0].rtBound.top);
				ReleaseDC(hWnd, hDC);
			}
		}
	}

	ISTATIC UINT LinkFromPoint(LPARAM lParam)
	{
		POINT pt = {LOWORD(lParam), HIWORD(lParam)};
		for (UINT i = 0; i < m_cLinkCount; i++)
		{
			if (PtInRect(&m_cLinks[i].rtBound, pt))
			{
				return i;
			}
		}
		return -1;
	}

public:
	struct ABOUTLINK
	{
		PCTSTR ptzText;
		PCTSTR ptzOpen;
		PCTSTR ptzParam;
		RECT rtBound;
	};

	STATIC UINT m_uCurLink;
	STATIC ABOUTLINK m_cLinks[];
	STATIC CONST UINT m_cLinkCount;
};

// Static variable
__declspec(selectany) UINT CeleAbout::m_uCurLink = -1;
__declspec(selectany) CeleAbout::ABOUTLINK CeleAbout::m_cLinks[] = {STR_AboutLink};
__declspec(selectany) CONST UINT CeleAbout::m_cLinkCount = _NumOf(m_cLinks);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
