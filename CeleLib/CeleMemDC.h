


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleMemFile 2.0.200
// Copyright (C) Yonsm 2008-2009, All Rights Reserved.
#pragma once
#include "UniBase.h"

//#define _AutoRotate
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory DC
class CeleMemDC
{
protected:
	HDC m_hMemDC;
	HDC m_hSrcDC;
	RECT m_rtSrcDC;
	HBITMAP m_hBitmap;
	HBITMAP m_hOldBitmap;

public:
	CeleMemDC(HDC hSrcDC, RECT rtSrcDC, BOOL bCopyFrom = FALSE, HBITMAP hBitmap = NULL)
	{
		m_hSrcDC = hSrcDC;
		m_rtSrcDC = rtSrcDC;
		m_hMemDC = CreateCompatibleDC(hSrcDC);
		m_hBitmap = hBitmap ? hBitmap : CreateCompatibleBitmap(hSrcDC, _RectWidth(m_rtSrcDC), _RectHeight(m_rtSrcDC));
		m_hOldBitmap = (HBITMAP) SelectObject(m_hMemDC, m_hBitmap);
		if (bCopyFrom)
		{
			BitBlt(m_hMemDC, 0, 0, GetWidth(), GetHeight(), m_hSrcDC, m_rtSrcDC.left,  m_rtSrcDC.top, SRCCOPY);
		}
	}

	~CeleMemDC()
	{
		if (m_hSrcDC)
		{
			BitBlt(m_hSrcDC, m_rtSrcDC.left,  m_rtSrcDC.top, GetWidth(), GetHeight(), m_hMemDC, 0, 0, SRCCOPY);
		}
		SelectObject(m_hMemDC, m_hOldBitmap);
		DeleteObject(m_hBitmap);
		DeleteDC(m_hMemDC);
	}

	operator HDC()
	{
		return m_hMemDC;
	}

	operator HBITMAP()
	{
		return m_hBitmap;
	}

	INT GetWidth()
	{
		return _RectWidth(m_rtSrcDC);
	}

	INT GetHeight()
	{
		return _RectHeight(m_rtSrcDC);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory DC with DIB support
template<UINT TByteCount = 3> class CeleDibDC: public CeleMemDC
{
protected:
	PBYTE m_pbBits;

public:
	CeleDibDC(HDC hSrcDC, RECT rtSrc, BOOL bCopyFrom = FALSE):
	  CeleMemDC(hSrcDC, rtSrc, bCopyFrom, CreateBitmap(hSrcDC, _RectWidth(rtSrc), _RectHeight(rtSrc)))
	{
		_Assert((TByteCount == 3) || (TByteCount == 4));
	}

	HBITMAP CreateBitmap(HDC hDC, UINT nWidth, UINT nHeight)
	{
		BITMAPINFOHEADER bih;
		bih.biSize = sizeof(bih);
		bih.biWidth = nWidth;
		bih.biHeight = nHeight;
		bih.biPlanes = 1;
		bih.biBitCount = (TByteCount * 8);
		bih.biCompression = /*(TByteCount < 3) ? BI_BITFIELDS : */BI_RGB;
		bih.biSizeImage = _DibSize(bih.biWidth, bih.biBitCount, bih.biHeight);
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;
		return CreateDIBSection(hDC, (BITMAPINFO*) &bih, DIB_RGB_COLORS, (PVOID*) &m_pbBits, NULL, 0);
	}

	operator PBYTE()
	{
		return m_pbBits;
	}

	VOID Gray()
	{
		PBYTE q = m_pbBits;
		INT yDstInc = _DibStride(GetWidth(), (TByteCount * 8)) - GetWidth() * TByteCount;
		for (INT y = GetHeight(); y > 0; y--, q += yDstInc)
		{
			for (INT x = GetWidth(); x > 0; x--, q += TByteCount)
			{
				q[0] = q[1] = q[2] = (q[0] + q[1] + q[2]) / 3;
			}
		}
	}

	VOID Paint(INT x, INT y, PBITMAPINFOHEADER pImage, PBYTE pbData = NULL)
	{
		INT nSrcByteCount = pImage->biBitCount / 8;
		INT nDstStride = _DibStride(GetWidth(), (TByteCount * 8));
		PBYTE p = pbData ? pbData : ((PBYTE) pImage + pImage->biSize);
		PBYTE q;
		INT nDrawWidth, nDrawHeight;
		INT xDstInc, yDstInc, ySrcInc;

#ifdef _AutoRotate
		if (((GetWidth() > GetHeight()) && (pImage->biWidth < pImage->biHeight)) ||
			((GetWidth() < GetHeight()) && (pImage->biWidth > pImage->biHeight)))
		{
			y = GetHeight() - y - pImage->biWidth;
			q = m_pbBits + nDstStride * y;

			xDstInc = -nDstStride;
			nDrawWidth = pImage->biWidth;
			if (x < 0)
			{
				p -= x * nSrcByteCount;
				nDrawHeight = pImage->biHeight + x;
			}
			else
			{
				q += x * TByteCount;
				nDrawHeight = (pImage->biHeight > GetWidth() - x) ? (GetWidth() - x) : pImage->biHeight;
			}

			q += nDstStride * pImage->biWidth;
			yDstInc = nDstStride * pImage->biWidth + TByteCount;
			ySrcInc = _DibStride(pImage->biWidth, pImage->biBitCount) - pImage->biWidth * nSrcByteCount;
		}
		else
#endif
		{
			y = GetHeight() - y - pImage->biHeight;
			q = m_pbBits + nDstStride * y;

			xDstInc = TByteCount;
			nDrawHeight = pImage->biHeight;
			if (x < 0)
			{
				p -= x * nSrcByteCount;
				nDrawWidth = pImage->biWidth + x;
			}
			else
			{
				q += x * TByteCount;
				nDrawWidth = (pImage->biWidth > GetWidth() - x) ? (GetWidth() - x) : pImage->biWidth;
			}

			yDstInc = nDstStride - nDrawWidth * TByteCount;
			ySrcInc = _DibStride(pImage->biWidth, pImage->biBitCount) - nDrawWidth * nSrcByteCount;
		}

		if (pImage->biBitCount == 32)
		{
			for (INT y = nDrawHeight; y > 0; y--, q += yDstInc, p += ySrcInc)
			{
				for (INT x = nDrawWidth; x > 0; x--, q += xDstInc, p += nSrcByteCount)
				{
					q[0] = (p[0] * p[3] + q[0] * (255 - p[3])) / 256;
					q[1] = (p[1] * p[3] + q[1] * (255 - p[3])) / 256;
					q[2] = (p[2] * p[3] + q[2] * (255 - p[3])) / 256;
				}
			}
		}
		else //if (pImage->biBitCount == 24)
		{
			_Assert(pImage->biBitCount == 24);
			for (INT y = nDrawHeight; y > 0; y--, q += yDstInc, p += ySrcInc)
			{
				for (INT x = nDrawWidth; x > 0; x--, q += xDstInc, p += nSrcByteCount)
				{
					q[0] = p[0];
					q[1] = p[1];
					q[2] = p[2];
				}
			}
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
