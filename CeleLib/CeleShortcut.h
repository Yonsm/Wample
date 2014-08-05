


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleUtil 2.0.202
// Copyright (C) Yonsm 2009-2010, All Rights Reserved.
#pragma once
#include "UniBase.h"

#ifndef WINCE
#include <ShlObj.h>
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleShortcut class
class CeleShortcut
{
public:
	ISTATIC BOOL CreateShortcut(PCTSTR ptzShortcut, PCTSTR ptzTarget, PCTSTR ptzParam = NULL, PCTSTR ptzIcon = NULL, INT iIcon = 0)
	{
#ifdef WINCE
		TCHAR tzTarget[MAX_PATH];
		UINT i = TStrFormat(tzTarget, TEXT("\"%s\""), ptzTarget);
		if (ptzParam)
		{
			i += TStrFormat(tzTarget + i, TEXT(" %s"), ptzParam);
		}
		if (ptzIcon)
		{
			i += TStrFormat(tzTarget + i, TEXT("?%s,%d"), ptzIcon, iIcon);
		}
		UFileDelete(ptzShortcut);
		return SHCreateShortcut((PTSTR) ptzShortcut, tzTarget);
#else
		IShellLink *pLink;
		CoInitialize(NULL);
		HRESULT hResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (PVOID *) &pLink);
		if (SUCCEEDED(hResult))
		{
			IPersistFile *pFile;
			hResult = pLink->QueryInterface(IID_IPersistFile, (PVOID *) &pFile);
			if (SUCCEEDED(hResult))
			{
				hResult = pLink->SetPath(ptzTarget);
				if (ptzParam) hResult = pLink->SetArguments(ptzParam);
				if (ptzIcon) hResult = pLink->SetIconLocation(ptzIcon, iIcon);

				//hResult = pLink->SetShowCmd(nShowCmd);
				//hResult = pLink->SetWorkingDirectory(ptzTarget);

				WCHAR wzShortcut[MAX_PATH];
				TStrToWStr(wzShortcut, ptzShortcut, MAX_PATH);
				hResult = pFile->Save(wzShortcut, FALSE);
				pFile->Release();
			}
			pLink->Release();
		}
		CoUninitialize();
		return SUCCEEDED(hResult);
#endif
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CeleAutoRun class
class CeleAutoRun
{
public:
	ISTATIC BOOL Get(PTSTR ptzShortcut = NULL)
	{
		TCHAR tzShortcut[MAX_PATH];
		if (ptzShortcut == NULL)
		{
			ptzShortcut = tzShortcut;
		}
		SHGetSpecialFolderPath(NULL, ptzShortcut, CSIDL_STARTUP, TRUE);
		UPathMake(ptzShortcut, g_ptzAppName ? g_ptzAppName : TEXT("CeleAutoRun"));
		TStrCat(ptzShortcut,  TEXT(".lnk"));
		return UFileExist(ptzShortcut);
	}

	ISTATIC BOOL Create(PTSTR ptzShortcut, PCTSTR ptzParam = TEXT("AutoRun"))
	{
		TCHAR tzPath[MAX_PATH];
		UDirGetAppPath(tzPath);
		return CeleShortcut::CreateShortcut(ptzShortcut, tzPath, ptzParam);
	}

	ISTATIC BOOL Create(BOOL bCreate, PCTSTR ptzParam = TEXT("AutoRun"))
	{
		TCHAR tzShortcut[MAX_PATH];
		Get(tzShortcut);
		if (bCreate)
		{
			return Create(tzShortcut, ptzParam);
		}
		else
		{
			return UFileDelete(tzShortcut);
		}
	}

	ISTATIC BOOL Reset(PCTSTR ptzParam = TEXT("AutoRun"))
	{
		TCHAR tzShortcut[MAX_PATH];
		if (Get(tzShortcut))
		{
			return Create(tzShortcut, ptzParam);
		}
		return FALSE;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
