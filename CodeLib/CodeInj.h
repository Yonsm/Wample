


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Header
#pragma once
#include "UniBase.h"
#include <TlHelp32.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Some declarations from Platform Builder
typedef struct _CALLBACKINFO
{
	HANDLE hProc;
	FARPROC pFunc;
	PVOID pvArg0;
}
CALLBACKINFO, *PCALLBACKINFO;

extern "C" BOOL SetKMode(BOOL fMode);
extern "C" DWORD SetProcPermissions(DWORD dwPermissions) ;
extern "C" PVOID MapPtrToProcess(PVOID pvAddress, HANDLE hProcess);
extern "C" DWORD PerformCallBack4(PCALLBACKINFO pci, DWORD dw1, DWORD dw2, DWORD dw3);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CCodeInj class
class CCodeInj
{
public:
	STATIC HMODULE RemoteLoadLibrary(HANDLE hProcess, PCTSTR ptzPath)
	{
		BOOL bMode = SetKMode(TRUE);
		DWORD dwPerm = SetProcPermissions(0xFFFFFFFF);

		CALLBACKINFO ci;
		ci.hProc= hProcess;
		ci.pFunc = (FARPROC) MapPtrToProcess(GetProcAddress(GetModuleHandle(TEXT("COREDLL")), TEXT("LoadLibraryW")), hProcess);;
		ci.pvArg0 = MapPtrToProcess((PVOID) ptzPath, GetCurrentProcess());
		HMODULE hModule = (HMODULE) PerformCallBack4(&ci, 0, 0, 0);

		SetKMode(bMode);
		SetProcPermissions(dwPerm);
		return hModule;
	}

	STATIC HMODULE RemoteLoadLibrary(PCTSTR ptzProcess, PCTSTR ptzPath)
	{
		HANDLE hProcess = OpenProcess(ptzProcess);
		if (hProcess)
		{
			HMODULE hModule = RemoteLoadLibrary(hProcess, ptzPath);
			CloseHandle(hProcess);
			return hModule;
		}

		return NULL;
	}

public:
	STATIC DWORD GetProcess(PCTSTR ptzProcess)
	{
		DWORD dwProces = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnap != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 pe;
			pe.dwSize = sizeof(pe);
			for (BOOL bRet = Process32First(hSnap, &pe); bRet; bRet = Process32Next(hSnap, &pe))
			{
				if (UStrCmpI(pe.szExeFile, ptzProcess) == 0)
				{
					dwProces = pe.th32ProcessID;
					break;
				}
			}

			CloseToolhelp32Snapshot(hSnap);
		}
		return dwProces;
	}

	STATIC HANDLE OpenProcess(PCTSTR ptzProcess)
	{
		DWORD dwProcess = GetProcess(ptzProcess);
		if (dwProcess)
		{
			HANDLE hProcess = ::OpenProcess(0, FALSE, dwProcess);
			return hProcess ? hProcess : (HANDLE) dwProcess;
		}
		return NULL;
	}

	STATIC BOOL KillProcess(PCTSTR ptzProcess)
	{
		HANDLE hProcess = OpenProcess(ptzProcess);
		if (hProcess)
		{
			BOOL bRet = TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
			return bRet;
		}
		return FALSE;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
