


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleReg 2.0.200
// Copyright (C) Yonsm 2008-2009, All Rights Reserved.
#pragma once
#include <Windows.h>

#ifdef WINCE
#define _RegDeleteKey RegDeleteKey
#else
#include <ShLwApi.h>
#pragma comment(lib,"ShLwApi.lib")
#define _RegDeleteKey SHDeleteKey
#endif

#ifdef STR_AppName
#define REG_AppKey TEXT("Software\\") STR_AppName
#else
#define REG_AppKey TEXT("Software\\CeleWare")
#endif

#define REG_AppSubKey(x) REG_AppKey TEXT("\\") TEXT(x)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleReg class
class CeleReg
{
protected:
	HKEY m_hKey;

public:
	CeleReg(PCTSTR ptzKey = REG_AppKey, HKEY hKey = HKEY_CURRENT_USER, REGSAM regSam = KEY_ALL_ACCESS)
	{
		m_hKey = NULL;
		RegCreateKeyEx(hKey, ptzKey, 0, NULL, 0, regSam, NULL, &m_hKey, NULL);
	}

	CeleReg(HKEY hKey, PCTSTR ptzKey, REGSAM regSam = KEY_READ)
	{
		m_hKey = NULL;
		RegOpenKeyEx(hKey, ptzKey, 0, regSam, &m_hKey);
	}

	~CeleReg()
	{
		if (m_hKey)
		{
			RegCloseKey(m_hKey);
		}
	}

	operator HKEY()
	{
		return m_hKey;
	}

public:
	INT GetInt(PCTSTR ptzName, INT iDef = 0)
	{
		UINT uSize = sizeof(INT);
		GetVal(ptzName, &iDef, &uSize);
		return iDef;
	}

	BOOL SetInt(PCTSTR ptzName, INT iVal = 0)
	{
		return SetVal(ptzName, &iVal, sizeof(INT), REG_DWORD);
	}

	UINT GetStr(PCTSTR ptzName, PTSTR ptzStr, UINT uLen = MAX_PATH, PCTSTR ptzDef = NULL)
	{
		ptzStr[0] = 0;
		uLen = uLen * sizeof(TCHAR);
		if (GetVal(ptzName, ptzStr, &uLen))
		{
			return (uLen / sizeof(TCHAR)) - 1;
		}
		else if (ptzDef)
		{
			PCTSTR p = ptzDef;
			while (*ptzStr++ = *p++);
			return (UINT) (p - ptzDef);
		}
		else
		{
			return 0;
		}
	}

	BOOL SetStr(PCTSTR ptzName, PCTSTR ptzStr = TEXT(""))
	{
		UINT uLen = 1;
		while (ptzStr[uLen++]);
		return SetVal(ptzName, ptzStr, uLen * sizeof(TCHAR), REG_SZ);
	}

	BOOL GetVal(PCTSTR ptzName, PVOID pvData, UINT uSize, PDWORD pdwType = NULL)
	{
		return GetVal(ptzName, pvData, &uSize, pdwType);
	}

	BOOL GetVal(PCTSTR ptzName, PVOID pvData, PUINT puSize, PDWORD pdwType = NULL)
	{
		return RegQueryValueEx(m_hKey, ptzName, NULL, pdwType, (PBYTE) pvData, (PDWORD) puSize) == S_OK;
	}

	BOOL SetVal(PCTSTR ptzName, CONST VOID* pvData, UINT uSize, DWORD dwType = REG_BINARY)
	{
		return RegSetValueEx(m_hKey, ptzName, 0, dwType, (PBYTE) pvData, uSize) == S_OK;
	}

public:
	BOOL DelVal(PCTSTR ptzName)
	{
		return RegDeleteValue(m_hKey, ptzName) == S_OK;
	}

	static BOOL DelKey(PCTSTR ptzKey = REG_AppKey, HKEY hKey = HKEY_CURRENT_USER)
	{
		return _RegDeleteKey(hKey, ptzKey) == S_OK;
	}

	BOOL EnumVal(DWORD dwIndex, PTSTR ptzName, PBYTE pbVal, PUINT puSize, PDWORD pdwType = NULL)
	{
		DWORD dwSize = MAX_PATH;
		return RegEnumValue(m_hKey, dwIndex, ptzName, &dwSize, NULL, pdwType, pbVal, (PDWORD) puSize) == S_OK;
	}

	BOOL EnumKey(DWORD dwIndex, PTSTR ptzKeyName)
	{
		FILETIME ftTime;
		DWORD dwSize = MAX_PATH;
		return RegEnumKeyEx(m_hKey, dwIndex, ptzKeyName, &dwSize, NULL, NULL, 0, &ftTime) == S_OK;
	}

public:
	BOOL ExistVal(PCTSTR ptzName)
	{
		UINT uSize;
		return GetVal(ptzName, NULL, &uSize);
	}

	static BOOL ExistKey(PCTSTR ptzKey = REG_AppKey, HKEY hKey = HKEY_CURRENT_USER)
	{
		if (RegOpenKeyEx(hKey, ptzKey, 0, KEY_READ, &hKey) == S_OK)
		{
			RegCloseKey(hKey);
			return TRUE;
		}
		return FALSE;
	}

	UINT GetSize(PCTSTR ptzName)
	{
		UINT uSize = 0;
		GetVal(ptzName, NULL, &uSize);
		return uSize;
	}

	BOOL GetInfo(PDWORD pcValues = NULL, PDWORD pcSubKeys = NULL, PDWORD pcbMaxValueName = NULL, PDWORD pcbMaxValue = NULL, PDWORD pcbMaxSubKey = NULL)
	{
		return RegQueryInfoKey(m_hKey, NULL, NULL, NULL, pcSubKeys, pcbMaxSubKey, NULL, pcValues, pcbMaxValueName, pcbMaxValue, NULL, NULL) == S_OK;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
