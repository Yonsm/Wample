


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleAuto 2.0.200
// Copyright (C) Yonsm 2008-2009, All Rights Reserved.
#pragma once
#include <Windows.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleAutoBase
template<typename M> class CeleAutoBase
{
protected:
	M m;

public:
	template<typename T> operator T()
	{
		return (T) m;
	}

	template<typename T> M operator +(T t)
	{
		return m + t;
	}

	template<typename T> M operator -(T t)
	{
		return m - t;
	}

	template<typename T> M operator ++()
	{
		return m++;
	}

	template<typename T> M operator --()
	{
		return m--;
	}

	BOOL operator ==(M t)
	{
		return m == t;
	}

	M operator =(M t)
	{
		return m = t;
	}

	BOOL operator !()
	{
		return !m;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleAutoMem
class CeleAutoMem: public CeleAutoBase<PBYTE>
{
public:
	PBYTE operator =(PVOID pvMem)
	{
		return m = (PBYTE) pvMem;
	}

	CeleAutoMem(PVOID pvMem = NULL)
	{
		m = (PBYTE) pvMem;
	}

	CeleAutoMem(UINT uSize)
	{
		m = (PBYTE) malloc(uSize);
	}

	~CeleAutoMem()
	{
		if (m)
		{
			free(m);
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleAutoHandle
class CeleAutoHandle: public CeleAutoBase<HANDLE>
{
public:
	HANDLE operator =(HANDLE hHaldle)
	{
		return m = hHaldle;
	}

	CeleAutoHandle(HANDLE hHaldle = NULL)
	{
		m = hHaldle;
	}

	~CeleAutoHandle()
	{
		if (m)
		{
			CloseHandle(m);
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleAutoPtr
template<class M> class CeleAutoPtr
{
protected:
	M* m;

public:
	M* operator =(M* t)
	{
		return m = t;
	}

	CeleAutoPtr(M* t = NULL)
	{
		m = t;
	}

	~CeleAutoPtr()
	{
		if (m)
		{
			m->Release();
		}
	}

	template<class T> operator T*()
	{
		T* p;
		m->QueryInterface(__uuidof(T), (PVOID*) &p);
		return p;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleAutoCursor
class CeleAutoCursor: public CeleAutoBase<HCURSOR>
{
public:
	CeleAutoCursor(HCURSOR hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)))
	{
		m = SetCursor(hCursor);
	}

	~CeleAutoCursor()
	{
		SetCursor(m);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleAutoFont
class CeleAutoFont
{
private:
	HDC m_hDC;
	HFONT m_hNewFont;
	HFONT m_hOldFont;

public:
	CeleAutoFont(HDC hDC, HFONT hFont)
	{
		m_hNewFont = hFont;
		m_hOldFont = (HFONT) SelectObject(hDC, m_hNewFont);
	}

	CeleAutoFont(HDC hDC, LOGFONT lFont)
	{
		m_hNewFont = CreateFontIndirect(&lFont);
		m_hOldFont = (HFONT) SelectObject(hDC, m_hNewFont);
	}

	CeleAutoFont(HDC hDC, INT nPoint = 8, INT nWeight = 0, PCTSTR ptzName = TEXT("MS Shell Dlg"))
	{
		LOGFONT lf = {0};
		lf.lfHeight = -PointToHeight(hDC, IsCJK() ? (nPoint + 1) : nPoint);
		lf.lfWeight = nWeight;
		lstrcpy(lf.lfFaceName, ptzName);
		m_hNewFont = CreateFontIndirect(&lf);
		m_hOldFont = (HFONT) SelectObject(hDC, m_hNewFont);
	}

	~CeleAutoFont()
	{
		SelectObject(m_hDC, m_hOldFont);
		DeleteObject(m_hNewFont);
	}

public:
	static INT PointToHeight(HDC hDC, UINT nPoint = 8)
	{
		return (INT) ((nPoint * (DOUBLE) GetDeviceCaps(hDC, LOGPIXELSY) / 72.0) + 0.5);
	}

	static BOOL IsCJK()
	{
		switch (PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID())))
		{
		case LANG_CHINESE:
		case LANG_KOREAN:
		case LANG_JAPANESE:
			return TRUE;
		}
		return FALSE;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
