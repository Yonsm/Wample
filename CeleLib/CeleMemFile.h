


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleMemFile 2.0.200
// Copyright (C) Yonsm 2008-2009, All Rights Reserved.
#pragma once
#include <Windows.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleMemFile
#ifdef WINCE
#define _CreateFileForMapping CreateFileForMapping
#else
#define _CreateFileForMapping CreateFile
#endif
class CeleMemFile
{
protected:
	HANDLE m_hFile;
	HANDLE m_hMap;
	PBYTE m_pbFile;
	DWORD m_dwSize;

public:
	BOOL operator !() {return !m_pbFile;}
	operator DWORD() CONST {return m_dwSize;}
	operator PVOID() CONST {return m_pbFile;}
	operator PBYTE() CONST {return m_pbFile;}
	operator PCHAR() CONST {return (PCHAR) m_pbFile;}
	operator PWCHAR() CONST {return (PWCHAR) m_pbFile;}

public:
	CeleMemFile(PCTSTR ptzPath, BOOL bRead = TRUE, DWORD dwSize = 0)
	{
		m_dwSize = 0;
		m_pbFile = NULL;
		m_hMap = NULL;
		m_hFile = _CreateFileForMapping(ptzPath,
			(bRead ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE)), FILE_SHARE_READ, NULL,
			((bRead || (dwSize == 0)) ? OPEN_EXISTING : OPEN_ALWAYS), FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			m_dwSize = dwSize ? dwSize : GetFileSize(m_hFile, NULL);
			if (m_dwSize)
			{
				m_hMap = CreateFileMapping(m_hFile, NULL, bRead ? PAGE_READONLY : PAGE_READWRITE, 0, m_dwSize, NULL);
				if (m_hMap)
				{
					m_pbFile = (PBYTE) MapViewOfFile(m_hMap, bRead ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS, 0, 0, m_dwSize);
				}
			}
		}
	}

	~CeleMemFile()
	{
		if (m_pbFile) UnmapViewOfFile(m_pbFile);
		if (m_hMap) CloseHandle(m_hMap);
		if (m_hFile != INVALID_HANDLE_VALUE) CloseHandle(m_hFile);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
