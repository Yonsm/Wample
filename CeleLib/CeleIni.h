


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleIni 2.0.200 For Windows 9x/NT
// Copyright (C) Yonsm 2007-2009, All Rights Reserved.
#pragma once
#include <Windows.h>

#ifdef STR_AppName
#define INI_AppKey STR_AppName
#else
#define INI_AppKey TEXT("CeleWare")
#endif

#define INI_AppSubKey(x) INI_AppKey TEXT("\\") TEXT(x)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleIni class
class CeleIni
{
protected:
	PCTSTR m_ptzKey;
	TCHAR m_tzPath[MAX_PATH];

public:
	CeleIni(PCTSTR ptzKey = INI_AppKey)
	{
		m_ptzKey = ptzKey;
		lstrcpy(m_tzPath + GetModuleFileName(NULL, m_tzPath, MAX_PATH) - 4, TEXT(".ini"));
	}

	CeleIni(PCTSTR ptzPath, PCTSTR ptzKey)
	{
		m_ptzKey = ptzKey;
		lstrcpy(m_tzPath, ptzPath);
	}

	PCTSTR operator =(PCTSTR ptzKey)
	{
		return m_ptzKey = ptzKey;
	}

	operator PCTSTR() const
	{
		return m_tzPath;
	}

public:
	INT GetInt(PCTSTR ptzName, INT iDef = 0)
	{
		return GetPrivateProfileInt(m_ptzKey, ptzName, iDef, m_tzPath);
	}

	BOOL SetInt(PCTSTR ptzName, INT iVal = 0)
	{
		TCHAR tzStr[16];
		wsprintf(tzStr, TEXT("%d"), iVal);
		return WritePrivateProfileString(m_ptzKey, ptzName, tzStr, m_tzPath);
	}

	UINT GetStr(PCTSTR ptzName, PTSTR ptzStr, UINT uLen = MAX_PATH, PCTSTR ptzDef = NULL)
	{
		return GetPrivateProfileString(m_ptzKey, ptzName, ptzDef, ptzStr, uLen, m_tzPath);
	}

	BOOL SetStr(PCTSTR ptzName, PCTSTR ptzStr = TEXT(""))
	{
		return WritePrivateProfileString(m_ptzKey, ptzName, ptzStr, m_tzPath);
	}

	BOOL GetVal(PCTSTR ptzName, PVOID pvData, UINT uSize)
	{
		return GetPrivateProfileStruct(m_ptzKey, ptzName, pvData, uSize, m_tzPath);
	}

	BOOL SetVal(PCTSTR ptzName, CONST VOID* pvData, UINT uSize)
	{
		return WritePrivateProfileStruct(m_ptzKey, ptzName, (PVOID) pvData, uSize, m_tzPath);
	}

public:
	BOOL DelVal(PCTSTR ptzName)
	{
		return WritePrivateProfileString(m_ptzKey, ptzName, NULL, m_tzPath);
	}

	BOOL DelKey()
	{
		return WritePrivateProfileString(m_ptzKey, NULL, NULL, m_tzPath);
	}

	UINT EnumVal(PTSTR ptzBuf, UINT uLen)
	{
		return GetPrivateProfileSection(m_ptzKey, ptzBuf, uLen, m_tzPath);
	}

	UINT EnumKey(PTSTR ptzBuf, UINT uLen)
	{
		return GetPrivateProfileSectionNames(ptzBuf, uLen, m_tzPath);
	}

public:
	BOOL SetSection(PCTSTR ptzStr)
	{
		return WritePrivateProfileSection(m_ptzKey, ptzStr, m_tzPath);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
