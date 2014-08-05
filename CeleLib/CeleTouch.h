

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleTouch 2.0.200
// Copyright (C) Yonsm 2009, All Rights Reserved.
#pragma once
#include <Windows.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleTouch class
class CeleTouch
{
protected:
	INT m_xOffset;
	LPARAM m_lMousePos;
	enum {Touch_None, Touch_Down, Touch_Touch} m_eTouchState;

	const static INT TOUCH_MoveDelta = 4;
	const static INT TOUCH_BackDelta = 64;
	const static INT TOUCH_Threshold = 12;

public:
	CeleTouch()
	{
		m_xOffset = 0;
		m_eTouchState = Touch_None;
	}

	BOOL OnMouseDown(HWND hWnd, LPARAM lParam)
	{
		SetCapture(hWnd);
		m_lMousePos = lParam;
		m_eTouchState = Touch_Down;
		return FALSE;
	}

	BOOL OnMouseMove(HWND hWnd, LPARAM lParam)
	{
		if (m_eTouchState != Touch_None)
		{
			INT xDelta = (SHORT) (lParam) - (SHORT) (m_lMousePos);
			if (xDelta > TOUCH_MoveDelta || xDelta < -TOUCH_MoveDelta)
			{
				if (m_eTouchState == Touch_Down)
				{
					if (xDelta > TOUCH_Threshold || xDelta < -TOUCH_Threshold)
					{
						m_eTouchState = Touch_Touch;
					}
				}
				if (m_eTouchState == Touch_Touch)
				{
					m_lMousePos = lParam;
					m_xOffset += xDelta;
					OnPaint(hWnd);
					return TRUE;
				}
			}
		}
		return FALSE;
	}

	BOOL OnMouseUp(HWND hWnd, LPARAM lParam, INT xMin = 0, INT xMax = 0)
	{
		ReleaseCapture();
		if (m_eTouchState != Touch_Touch)
		{
			m_eTouchState = Touch_None;
			return FALSE;
		}

		m_xOffset += (SHORT) (lParam) - (SHORT) (m_lMousePos);
		if ((xMax > 0) && (xMin == -xMax))	// Trick: Support PageUp & PageDown
		{
			if (m_xOffset > xMax)
			{
				SendMessage(hWnd, WM_KEYDOWN, VK_PRIOR, 0);
			}
			else if (m_xOffset < xMin)
			{
				SendMessage(hWnd, WM_KEYDOWN, VK_NEXT, 0);
			}
			else
			{
				if (m_xOffset > 0)
				{
					while ((m_xOffset -= TOUCH_BackDelta) > 0)
					{
						OnPaint(hWnd);
					}
				}
				else
				{
					while ((m_xOffset += TOUCH_BackDelta) < 0)
					{
						OnPaint(hWnd);
					}
				}
			}
			m_xOffset = 0;
		}
		else
		{
			if (m_xOffset > xMax)
			{
				while ((m_xOffset -= TOUCH_BackDelta) > xMax)
				{
					OnPaint(hWnd);
				}
				m_xOffset = xMax;
			}
			else if (m_xOffset < xMin)
			{
				while ((m_xOffset += TOUCH_BackDelta) < xMin)
				{
					OnPaint(hWnd);
				}
				m_xOffset = xMin;
			}
		}

		OnPaint(hWnd);
		m_eTouchState = Touch_None;
		return TRUE;
	}

	virtual VOID OnPaint(HWND hWnd)
	{
		InvalidateRect(hWnd, NULL, TRUE);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
