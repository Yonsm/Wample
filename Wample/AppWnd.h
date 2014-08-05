


// Header
#pragma once


// CAppWnd class
class CAppWnd
{
public:
	CAppWnd();
	~CAppWnd();

	WPARAM Run(PTSTR ptzCmdLine, INT iCmdShow);

private:
	LRESULT CALLBACK WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	VOID OnCreate();
	VOID OnPaint(BOOL bGray = FALSE);
	VOID OnSize(LPARAM lParam);
	VOID OnCommand(UINT uCmd);
	VOID OnKeyDown(UINT vKey);

private:
	CeleThunk<HWND> m_tWnd;
};
