


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Header
#pragma once
#include <Windows.h>
#include "HijMisc.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Holds information for one system API, which is hooked by us
typedef struct _APIINFO
{
	BOOL bUsed;
	BOOL bSwapped;
	INT iOrgApiSetID;
	PCINFO pOrgApiSet;
	PCINFO pNewApiSet;
}
APIINFO, *PAPIINFO;

// Process information
typedef struct _PROCINFO
{
	DWORD dwID;
	DWORD dwTrust;
	HMODULE hCodeHij;			// CodeHij module handle in each process
	PTSTR ptzExeFile;			// Points into array tzExeFile
}
PROCINFO, *PPROCINFO;

// Share data in mapping share memory
typedef struct _SHAREDATA
{
	PROCINFO piProcs[32];		// CE has up to 32 running processes

	TCHAR tzCodeHij[_MAX_PATH];
	TCHAR tzLMemDebug[16];
	DWORD dwCount;

	PFNVOID pLoadCodeHij;
	PFNVOID pLoadLibraryEx;
	PFNVOID pGetProcModList;
	PFNVOID pGetRomFileInfo;
	PFNVOID pDebugNotify;
	PFNVOID pCeGetCurrentTrust;
	PFNVOID pGetProcessIndexFromID;

	PFNVOID* ppHookWin32Methods;
	PFNVOID* ppHookExtraMethods;

	DWORD dwNumCallsToStub;
	DWORD dwLastStack;

	TCHAR tzExeFile[32][MAX_PATH];

	UINT m_uStubSize;
	BYTE m_bStub[1];
}
SHAREDATA, *PSHAREDATA;


// Main data structure, only one instance in share data section
struct HijEngine
{
//public:
	// On process attach or detach. Must be called in DllMain
	BOOL Attach(HMODULE hModule);
	BOOL Detach();

	// Start or stop hijack engine
	BOOL Start();
	BOOL Stop();

	// Hook one API, return the old API address, must be called after Start()
	PFNVOID Hook(INT iApiSetID, INT iIndex, PFNVOID pfnNew);

//private:
	BOOL InitShareData();
	BOOL InitProcessList();
	BOOL InjectAllProcess();
	BOOL HookCoreDll(PVOID pvOld, PVOID pvNew);
	BOOL CreateDupApi(INT iApiSetID, INT iMinNumMethods = 0);
	BOOL CreateDupMethodTable(INT iOrgNum, INT iNewNum, PFNVOID* pMethods, PDWORD pdwSigns, PFNVOID*& ppRetHookMethods, PDWORD& pdwRetHookSigns);

	HMODULE LoadToProcess(HANDLE hProcess, DWORD& dwTrust);
	BOOL UnloadFromProcess(HANDLE hProcess, DWORD dwTrust, HMODULE hModule);

	BOOL FreeMemInKernelProcess(PVOID pvMem);
	PVOID AllocMemInKernelProcess(UINT uSize);
	PVOID MapProcessAddress(PVOID pvAddress, PPROCESS pProcess);
	DWORD CallForward(HANDLE hProcess, FARPROC pFunction, DWORD dw0 = 0, DWORD dw1 = 0, DWORD dw2 = 0, DWORD dw3 = 0, DWORD dw4 = 0, DWORD dw5 = 0, DWORD dw6 = 0, DWORD dw7 = 0);
	DWORD CallCoreDllInProcess(HANDLE hProcess, PCTSTR ptzMethod, DWORD dwParam1 = 0, DWORD dwParam2 = 0, DWORD dwParam3 = 0, DWORD dwParam4 = 0);
	DWORD CallCoreDllInKernelProcess(PCTSTR ptzMethod, DWORD dwParam1 = 0, DWORD dwParam2 = 0, DWORD dwParam3 = 0, DWORD dwParam4 = 0);
	BOOL CreateRemoteThread(HANDLE hProcess, FARPROC pThreadProc, DWORD dwParam, PBYTE pbData, INT iDataSize, INT iTimeout, PDWORD pdwRet);

//private:
	// Pointer to an array of system APIs
	PCINFO* m_pSysApiSets;

	// Pointers to the kernel Win32 method table and extra methods table
	PFNVOID* m_ppWin32Methods;
	PFNVOID* m_ppExtraMethods;
	PFNVOID* m_ppHookWin32Methods;
	PFNVOID* m_ppHookExtraMethods;
	PDWORD m_pdwHookWin32Signs;
	PDWORD m_pdwHookExtraSigns;

	// Other members
	BOOL m_bCoreDllHooked;

	// Mapping memory to store share data
	PSHAREDATA m_pShare;

	// All system APIs table
	APIINFO m_Api[32];
};
