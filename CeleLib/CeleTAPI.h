

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleTapi 2.0.20
// Copyright (C) Yonsm 2009-2010, All Rights Reserved.
#include <TSP.h>
#include <TAPI.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleTapi class
class CeleTapi
{
public:
	HLINEAPP m_hLineApp;
	DWORD m_dwDevCount;
	DWORD m_dwAPIVer;
	HANDLE m_hEvent;

public:
	CeleTapi()
	{
		m_dwAPIVer = 0x00020000;
		LINEINITIALIZEEXPARAMS liep;
		liep.dwTotalSize = sizeof(liep);
		liep.dwOptions = LINEINITIALIZEEXOPTION_USEEVENT;
		lineInitializeEx(&m_hLineApp, NULL, NULL, NULL, &m_dwDevCount, &m_dwAPIVer, &liep);
		m_hEvent = liep.Handles.hEvent;
	}

	~CeleTapi()
	{
		lineShutdown(m_hLineApp);
	}

	operator HLINEAPP()
	{
		return m_hLineApp;
	}

public:
	LONG WaitForMessage(DWORD dwMsg, LINEMESSAGE& msg, DWORD dwTimeout = INFINITE)
	{
		LONG hResult;
		while ((hResult = lineGetMessage(m_hLineApp, &msg, dwTimeout)) == S_OK)
		{
			if (msg.dwMessageID == dwMsg)
			{
				break;
			}
		}
		return hResult;
	}

	LONG WaitForCallState(DWORD dwState, LINEMESSAGE& msg, DWORD dwTimeout = INFINITE)
	{
		LONG hResult;
		while ((hResult = lineGetMessage(m_hLineApp, &msg, dwTimeout)) == S_OK)
		{
			if ((msg.dwMessageID == LINE_CALLSTATE) && (msg.dwParam1 == dwState))
			{
				break;
			}
		}
		return hResult;
	}

	LONG AnswerCall(PCTSTR ptzDTMF = NULL, DWORD dwTimeOut = INFINITE)
	{
		LINEMESSAGE msg = {0};
		LONG hResult = WaitForCallState(LINECALLSTATE_OFFERING, msg, dwTimeOut);
		if (hResult == S_OK)
		{
			Sleep(1000);
			hResult = lineAnswer((HCALL) msg.hDevice, NULL, 0);
			if (SUCCEEDED(hResult) && ptzDTMF)
			{
				hResult = WaitForCallState(LINECALLSTATE_CONNECTED, msg, dwTimeOut);
				if (hResult == S_OK)
				{
					Sleep(1000);
					hResult = lineGenerateDigits((HCALL) msg.hDevice, LINEDIGITMODE_DTMF, ptzDTMF, 0);
				}
			}
		}
		return hResult;
	}

	LONG DropCall(DWORD dwTimeOut = INFINITE)
	{
		LINEMESSAGE msg = {0};
		LONG hResult = WaitForCallState(LINECALLSTATE_PROCEEDING, msg, dwTimeOut);
		if (hResult == S_OK)
		{
			Sleep(1000 * 2);
			hResult = lineDrop((HCALL) msg.hDevice, NULL, 0);
		}
		return hResult;
	}

public:
	DWORD GetLineID(PCTSTR ptzLineName = CELLTSP_LINENAME_STRING)
	{
		DWORD dwLineID = -1;

		LINEDEVCAPS* pCaps = (LINEDEVCAPS*) malloc(1024);
		pCaps->dwTotalSize = 1024;
		for (DWORD i = 0; i < m_dwDevCount; i++)
		{
			LINEEXTENSIONID LineExtensionID;
			if (lineNegotiateAPIVersion(m_hLineApp, i, 0x00020000, 0x00020000, &m_dwAPIVer, &LineExtensionID) == 0)
			{
				LONG lResult = lineGetDevCaps(m_hLineApp, i, m_dwAPIVer, 0, pCaps);
				if (pCaps->dwTotalSize < pCaps->dwNeededSize)
				{
					pCaps = (LINEDEVCAPS*) realloc(pCaps, pCaps->dwNeededSize);
					pCaps->dwTotalSize = pCaps->dwNeededSize;
					lResult = lineGetDevCaps(m_hLineApp, i, m_dwAPIVer, 0, pCaps);
				}

				if ((lResult == 0) && (_tcscmp((PTSTR) ((PBYTE) pCaps + pCaps->dwLineNameOffset), ptzLineName) == 0))
				{
					dwLineID = i;
				}
			}
		}

		free(pCaps);
		return dwLineID;
	}

	LONG GetLineAddress(DWORD dwLineID, PTSTR ptzAdress)
	{
		LINEADDRESSCAPS* pCaps = (LINEADDRESSCAPS*) malloc(2048);
		pCaps->dwTotalSize = 2048;
		pCaps->dwNeededSize = 0;

		const DWORD dwAddressID = 0;
		LONG hResult = lineGetAddressCaps(m_hLineApp, dwLineID, dwAddressID, m_dwAPIVer, 0, pCaps);
		if (pCaps->dwTotalSize < pCaps->dwNeededSize)
		{
			pCaps = (LINEADDRESSCAPS*) realloc(pCaps, pCaps->dwNeededSize);
			pCaps->dwTotalSize = pCaps->dwNeededSize;
			hResult = lineGetAddressCaps(m_hLineApp, dwLineID, dwAddressID, m_dwAPIVer, 0, pCaps);
		}

		ptzAdress[0] = 0;
		if (hResult == S_OK)
		{
			if (pCaps->dwAddressSize)
			{
				memcpy(ptzAdress, (PTSTR) (((PBYTE) pCaps) + pCaps->dwAddressOffset), pCaps->dwAddressSize);
			}
			else
			{
				hResult = S_FALSE;
			}
		}
		free(pCaps);
		return hResult;
	}

	inline BOOL GetPhoneNumber(PTSTR ptzNumber)
	{
		return GetLineAddress(GetLineID(), ptzNumber);
	}

	STATIC UINT CopyPhoneNumber(PTSTR ptzDst, PCTSTR ptzSrc, PTSTR ptzCountryCode = TEXT("86"))
	{
		PTSTR ptzStart = ptzDst;
		if (*ptzSrc == '+')
		{
			ptzSrc++;
		}
		else
		{
			UINT nCountryCode = TStrLen(ptzCountryCode);
			if ((*ptzSrc == '0') && (TStrEqual(ptzSrc + 1, ptzCountryCode) == nCountryCode))
			{
				ptzSrc++;
			}
			else if (TStrEqual(ptzSrc, ptzCountryCode) != nCountryCode)
			{
				while (*ptzCountryCode) *ptzDst++ = *ptzCountryCode++;
			}
		}
		for (; *ptzSrc; ptzSrc++)
		{
			if (TChrIsNum(*ptzSrc) || (*ptzSrc == ',') || (*ptzSrc == '*') || (*ptzSrc == '#') || (*ptzSrc == '!'))
			{
				*ptzDst++ = *ptzSrc;
			}
		}
		*ptzDst = 0;
		return UINT(ptzDst - ptzStart);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleTAPILine class
class CeleTAPILine: public CeleTapi
{
public:
	HLINE m_hLine;

public:
	CeleTAPILine(DWORD dwLineID, DWORD dwPrivileges = LINECALLPRIVILEGE_MONITOR | LINECALLPRIVILEGE_OWNER, DWORD dwMediaModes = LINEMEDIAMODE_INTERACTIVEVOICE)
	{
		lineOpen(m_hLineApp, dwLineID, &m_hLine, m_dwAPIVer, 0, (DWORD) this, dwPrivileges, dwMediaModes, NULL);
	}

	CeleTAPILine(PCTSTR ptzLineName = CELLTSP_LINENAME_STRING, DWORD dwPrivileges = LINECALLPRIVILEGE_MONITOR | LINECALLPRIVILEGE_OWNER, DWORD dwMediaModes = LINEMEDIAMODE_INTERACTIVEVOICE)
	{
		lineOpen(m_hLineApp, GetLineID(ptzLineName), &m_hLine, m_dwAPIVer, 0, (DWORD) this, dwPrivileges, dwMediaModes, NULL);
	}

	~CeleTAPILine()
	{
		if (m_hLine) lineClose(m_hLine);
	}

	operator HLINE()
	{
		return m_hLine;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////