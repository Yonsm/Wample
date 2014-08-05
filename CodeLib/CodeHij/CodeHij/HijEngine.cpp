


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Header
#include "HijEngine.h"
#include <TlHelp32.h>
#pragma comment(lib, "ToolHelp.lib")

#define _Log
#define _Assert(x)	__noop
#define _NumOf(a)	(sizeof(a) / sizeof(a[0]))

// Module handle for CodeHij, Used in the first process
HMODULE g_hModule = NULL;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions placed in "STUBS" section will be copied at run time to a shared memory. 
// Therefore, these functions must not call directly any other functions outside "STUBS" section.
// The only way to communicate with the outside world is using a pointer to SHAREDATA and API calls using IMPLICIT_CALL.
// The pointer to SHAREDATA will be injected at run time into the stub code by replacing a placeholder STUB_PLACEHOLDER_SHARED_MEM with the actual address.
// To ensure that the compiler does not generate hidden calls to check for buffer security, specify /GS- compiler option.
#define STUB_PLACEHOLDER_SHARED_MEM 0x12345601

#pragma code_seg("STUBS$1")
VOID StubStartMarker(){} //used to measure the size of stubs routines
#pragma code_seg()

#pragma code_seg("STUBS$2")

// Redefine GetCurrentProcess from kfuncs.h
#define HANDLE_CURPROC ((HANDLE) (SH_CURPROC + SYS_HANDLE_BASE))

// StubLoadCodeHijInCE5 invokes LoadLibraryExW API to load our DLL into the current process on CE5.
// StubLoadCodeHijInCE5 will return immediately if the DLL is already loaded.

// CE5 made LoadLibrary a 2 step process: 
// SC_LoadLibraryExW loads one or more DLLs into process address space and returns the number of DllMains, which have to be called, in dwCount.
// The caller should allocate an array of DLLMAININFO[dwCount] and pass it to SC_GetProcModList.
// Then the caller should iterate through the array and invoke DllMain for all modules.
// All of that is normally done by COREDLL under critical section.
// We do not have access to that section, but we don't need it since we will be invoked by StubGetRomFileInfo or StubCeGetCurrentTrust, which will be called by DllMain of COREDLL.
// We do not really need this DLLMAININFO since we already know where our DllMain is (and we are not linked to any static library except for COREDLL)
// but kernel will generate debug asserts if we do not call.
BOOL StubLoadCodeHijInCE5(PSHAREDATA pData)
{
	DWORD dwProcIndex = ((DWORD (*)(HANDLE)) pData->pGetProcessIndexFromID)(HANDLE_CURPROC);
	if ((dwProcIndex >= _NumOf(pData->piProcs)) || (pData->piProcs[dwProcIndex].hCodeHij))
	{
		// Bad index - should not happen
		// Already loaded
		return TRUE;
	}

	HMODULE hModule = ((HMODULE (WINAPI *)(PCWSTR, DWORD, PDWORD)) pData->pLoadLibraryEx) (pData->tzCodeHij, 0, &pData->dwCount);
	if (!hModule)
	{
#ifdef DEBUG
		// Uncomment for debugging
		((DWORD (*)()) IMPLICIT_CALL(SH_WIN32, 89))(); //GetLastError
		DebugBreak();
#endif
		return FALSE;
	}

	typedef struct _DLLMAININFO
	{
		HMODULE m_hModule;
		FARPROC m_pDllMain;
		PVOID m_pBaseAddress;
		DWORD m_dwSect14rva;
		DWORD m_dwSect14size;
	}
	DLLMAININFO, *PDLLMAININFO;

	DWORD dwCount = pData->dwCount;
	if (!dwCount)
	{
		return TRUE;
	}

	PDLLMAININFO pInfo = (PDLLMAININFO) _alloca(sizeof(DLLMAININFO) * dwCount);
	if (((BOOL (WINAPI *)(PDLLMAININFO, DWORD)) pData->pGetProcModList)(pInfo, dwCount))
	{
		// Since our DLL does not depend on anything (except for COREDLL), dwCount should be 1
		for (DWORD i = 0; i < dwCount; i++)
		{
			// Our DllMain does not fail, so don't bother checking the return value.
			((BOOL (WINAPI *)(HANDLE, DWORD, PVOID))pInfo[i].m_pDllMain)(hModule, DLL_PROCESS_ATTACH, 0);
		}
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// StubLoadCodeHijInCE4 invokes LoadLibraryExW API to load our DLL into the current process on CE 4.x and earlier.
// StubLoadCodeHijInCE4 will return immediately if the DLL is already loaded.
BOOL StubLoadCodeHijInCE4(PSHAREDATA pData)
{
	DWORD dwProcIndex = ((DWORD (*)(HANDLE)) pData->pGetProcessIndexFromID)(HANDLE_CURPROC);
	if ((dwProcIndex >= _NumOf(pData->piProcs)) || (pData->piProcs[dwProcIndex].hCodeHij))
	{
		// Bad index - should not happen
		// Already loaded
		return TRUE;
	}

	HMODULE hModule = ((HMODULE (WINAPI *)(PCWSTR, HANDLE, DWORD)) pData->pLoadLibraryEx)(pData->tzCodeHij, 0, 0);
	if (!hModule)
	{
#ifdef DEBUG
		// Uncomment for debugging
		((DWORD (*)()) IMPLICIT_CALL(SH_WIN32, 89))(); //GetLastError
		DebugBreak();
#endif
		return FALSE;
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// An interceptor for undocumented function GetRomFileInfo,
// which is called by COREDLL when a new process started to retrieve pointers to 2 system API method tables: Win32Methods and ExtraMethods. 
// We must always intercept GetRomFileInfo to substitute these 2 pointers and also to load the CodeHij into the new process.
// DWORD dwType - type of information. Value 3 is used to retrieve a pointer to Win32Methods, 4 - ExtraMethods. Other values are used to enumerate ROM files.
// LPWIN32_FIND_DATA pFindData - pointer to a structure to fill
// DWORD dwSize - size of structure pointed by pFindData
BOOL StubGetRomFileInfo(DWORD dwType, LPWIN32_FIND_DATA pFindData, DWORD dwSize)
{
	PSHAREDATA pData = (PSHAREDATA) STUB_PLACEHOLDER_SHARED_MEM;
	if (dwType == 3 || dwType == 4)
	{
		// Calls from COREDLL have to be intercepted by our hook
		if (((BOOL (*)(PSHAREDATA)) pData->pLoadCodeHij)(pData))
		{
			*((PVOID*) pFindData) = (dwType == 3) ? pData->ppHookWin32Methods : pData->ppHookExtraMethods;
			return TRUE;
		}
	}

	// All other calls have to be forwarded to the original routine ASAP
	return ((BOOL (*)(DWORD, LPWIN32_FIND_DATA, DWORD)) pData->pGetRomFileInfo)(dwType, pFindData, dwSize);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hook CeGetCurrentTrust
DWORD StubCeGetCurrentTrust()
{
	PSHAREDATA pData = (PSHAREDATA) STUB_PLACEHOLDER_SHARED_MEM;
	pData->dwNumCallsToStub++;
	((BOOL (*)(PSHAREDATA)) pData->pLoadCodeHij)(pData);
	return ((DWORD (*)())pData->pCeGetCurrentTrust)();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
#pragma code_seg()

#pragma code_seg("STUBS$3")
VOID StubEndMarker(){} // Used to measure the size of stubs routines
#pragma code_seg()

inline PBYTE GetRealAddress(PVOID pAddress)
{
#ifdef _M_IX86
	// If we get an address of a local function (like StubStartMarker), we may get an address of a JUMP instruction to the real function.
	if (((PBYTE) pAddress)[0] == 0xE9)
	{
		//Get the target of the jump
		pAddress = (PBYTE) pAddress + 5 + *((PDWORD)((PBYTE) pAddress + 1));
	}
#endif

	return (PBYTE) pAddress;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// On process attach
BOOL HijEngine::Attach(HMODULE hModule)
{
	g_hModule = hModule;
	DWORD dwProcID = GetCurrentProcessId();
	DWORD dwProcIndex = GetProcessIndexFromID((HANDLE) dwProcID);
	PPROCINFO pProcess = m_pShare ? &m_pShare->piProcs[dwProcIndex] : NULL;
	if (!pProcess)
	{
		return FALSE;
	}

	// Fill new process info
	pProcess->dwID = dwProcID;
	pProcess->hCodeHij = hModule;
	pProcess->dwTrust = CeGetCurrentTrust();
	GetModuleFileName(NULL, pProcess->ptzExeFile, _MAX_PATH);

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// On process detach
BOOL HijEngine::Detach()
{
	DWORD dwIndex = GetProcessIndexFromID((HANDLE) GetCurrentProcessId());
	PPROCINFO pProcess = m_pShare ? &m_pShare->piProcs[dwIndex] : NULL;
	if (!pProcess)
	{
		return FALSE;
	}

	pProcess->hCodeHij = NULL;
	pProcess->dwTrust = 0;
	pProcess->dwID = 0;
	*pProcess->ptzExeFile = 0;

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start hijack engine
BOOL HijEngine::Start()
{
	// Initliaze this
	memset(this, 0, sizeof(HijEngine));

	// Initialize share data
	if (!InitShareData())
	{
		return FALSE;
	}

	PFNVOID pStubCeGetCurrentTrust = (PFNVOID) (m_pShare->m_bStub + (GetRealAddress(StubCeGetCurrentTrust) - GetRealAddress(StubStartMarker)));
	PFNVOID pStubGetRomFileInfo = (PFNVOID) (m_pShare->m_bStub + (GetRealAddress(StubGetRomFileInfo) - GetRealAddress(StubStartMarker)));

#ifdef DEBUG
	// Test that our stub does not crash. If it does, add /GS- compiler option.
	DWORD dwTrust = ((DWORD (*)()) pStubCeGetCurrentTrust)();
	((BOOL (*)(DWORD, LPWIN32_FIND_DATA, DWORD)) pStubGetRomFileInfo)(3, (LPWIN32_FIND_DATA) &dwTrust, (DWORD) &dwTrust);
#endif

	// Load CodeHij into all running processes
	InjectAllProcess();

	// Do the same as COREDLL itself is doing when each new process is loaded: get direct pointers to a table of Win32 methods and extra methods.
	BOOL bKernelMode;
	GetRomFileInfo(3, (LPWIN32_FIND_DATA) &m_ppWin32Methods, (DWORD) &bKernelMode);
	GetRomFileInfo(4, (LPWIN32_FIND_DATA) &m_ppExtraMethods, NULL);

	// Substitute the system API by our own.
	// Later custom interceptors may hook other APIs as well,  but we always need to intercept the SH_WIN32 because:
	// 1. we must intercept W32_GetRomFileInfo
	// 2. pointer to SH_WIN32 is cached by COREDLL and we need to replace this cached pointer.
	CreateDupApi(SH_WIN32);

	// Also create a copy of special "Extra methods" table.
	// This table is kept in the kernel and a pointer to is is cached by COREDLL. COREDLL uses it as a back door to some system services.
	// We, obviously, need to substitute this pointer as well.
	CreateDupMethodTable(256, 256, (PFNVOID*) m_ppExtraMethods, NULL, m_ppHookExtraMethods, m_pdwHookExtraSigns);

	m_pShare->ppHookWin32Methods = m_ppHookWin32Methods;
	m_pShare->ppHookExtraMethods = m_ppHookExtraMethods;

	// Replace pointers to SH_WIN32 table and "Extra Methods" table, which are cached by COREDLL, by our copies.
	if (!HookCoreDll(m_ppWin32Methods, m_ppHookWin32Methods) || !HookCoreDll(m_ppExtraMethods, m_ppHookExtraMethods))
	{
		// Undo the hooking:
		HookCoreDll(m_ppHookWin32Methods, m_ppWin32Methods);
		HookCoreDll(m_ppHookExtraMethods, m_ppExtraMethods);
		return FALSE;
	}
	else
	{
		m_bCoreDllHooked = TRUE;
	}	

	// Hook GetRomFileInfo and CeGetCurrentTrust to intercept new process loading
	// PPC 2003 calls GetRomFileInfo, Smartphone 2005 calls only CeGetCurrentTrust for untrusted processes, so we need to intercept both
	Hook(SH_WIN32, W32_GetRomFileInfo, pStubGetRomFileInfo);
	Hook(SH_WIN32, W32_CeGetCurrentTrust, pStubCeGetCurrentTrust);

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stop hijack engine
BOOL HijEngine::Stop()
{
	if (m_bCoreDllHooked)
	{
		m_bCoreDllHooked = FALSE;
		HookCoreDll(m_ppHookWin32Methods, m_ppWin32Methods);
		HookCoreDll(m_ppHookExtraMethods, m_ppExtraMethods);
	}

	INT i;
	for (i = 0; i < _NumOf(m_Api); i++)
	{
		PAPIINFO pApi = m_Api + i;
		if (!pApi->bUsed || !pApi->bSwapped)
		{
			continue;
		}

		// Copy the original table into our (eliminating all hooks)
		PFNVOID* pOrgMethods = (PFNVOID*) MapProcessAddress(pApi->pOrgApiSet->m_ppMethods, pApi->pOrgApiSet->m_pProcessServer);

		for (INT j = 0; j < pApi->pOrgApiSet->m_wNumMethods; j++)
		{
			pApi->pNewApiSet->m_ppMethods[j] = pOrgMethods[j];
		}

		// Swap back
		m_pSysApiSets[pApi->iOrgApiSetID] = pApi->pOrgApiSet;
		pApi->bSwapped = FALSE;

		// Free duplicate tables and CINFO.
		// WARNING: There may be a chance that they are still used by a dispatcher or COREDLL on another thread.
		// So, if crashes will happen after Stop - delay or remove this memory freeing
		FreeMemInKernelProcess(pApi->pNewApiSet->m_ppMethods);
		FreeMemInKernelProcess(pApi->pNewApiSet);
		pApi->pNewApiSet = NULL;
	}

	for (INT iProcID = _NumOf(m_pShare->piProcs) - 1; iProcID > 0; iProcID--)
	{
		// Skip NK.EXE
		PPROCINFO pProcess = m_pShare->piProcs + iProcID;
		if (pProcess->dwID == 0)
		{
			continue;
		}

		HANDLE hProcess = OpenProcess(0, FALSE, pProcess->dwID);
		if (hProcess)
		{
			UnloadFromProcess(hProcess, m_pShare->piProcs[iProcID].dwTrust, m_pShare->piProcs[iProcID].hCodeHij);
			CloseHandle(hProcess);
			m_pShare->piProcs[iProcID].hCodeHij = NULL;
		}
	}

	// TODO: Here means all active API calls to hooked API must exit asap. May be we may make a lock for each hooked API.
	Sleep(500);

	UnmapViewOfFile(m_pShare);
	m_pShare = NULL;

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Intercept the specified method in the specified API.
// INT iApiSetID - the API set ID (index) to intercept
// INT iIndex - method index in the given API to intercept
// PFNVOID pfnNew - interceptor method in the CodeHij.
// RETURN: Pointer to the original method. This pointer must be stored by the caller and used by the interceptor routine to call the original method.
PFNVOID HijEngine::Hook(INT iApiSetID, INT iIndex, PFNVOID pfnNew)
{
	// First call CreateDupApi to create a copy of the original API and swap pointers, so the copy will immediately be used.
	// Note that CreateDupApi can be called many times for the same API - it will only create a copy once, the following calls will simply return the copy created earlier.
	// This allows calling Hook for several method from the same API.
	if (!CreateDupApi(iApiSetID, iIndex + 1))
	{
		return NULL;
	}

	APIINFO& rAPI = m_Api[iApiSetID];
	if (iIndex >= (INT) rAPI.pNewApiSet->m_wNumMethods)
	{
		// Can not hook method
		return NULL;
	}

	// Then prepare an address to the method table of the target API
	PFNVOID* pOrgMethods = (PFNVOID*) MapProcessAddress(rAPI.pOrgApiSet->m_ppMethods, rAPI.pOrgApiSet->m_pProcessServer);
	PFNVOID pfnOrg = (iIndex < (INT) rAPI.pOrgApiSet->m_wNumMethods) ? ((PFNVOID*) pOrgMethods)[iIndex] : NULL;

	// Finally, put our interceptor to the table of the target API:
	((PFNVOID*) rAPI.pNewApiSet->m_ppMethods)[iIndex] = pfnNew;

	// Check whether the method is duplicated in the "extra" table and hook it there.
	for (INT i = 0; i < 256; i++)
	{
		if (m_ppHookExtraMethods[i] == pfnOrg)
		{
			// Replace method in Extra table
			m_ppHookExtraMethods[i] = pfnNew;
			break;
		}
	}

	return pfnOrg;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enumerate running processes and store process information
BOOL HijEngine::InitProcessList()
{
	_Assert(m_pShare);

	memset(m_pShare->piProcs, 0, sizeof(m_pShare->piProcs));
	for (INT i = 0; i < _NumOf(m_pShare->piProcs); i++)
	{
		m_pShare->piProcs[i].ptzExeFile = m_pShare->tzExeFile[i];
	}

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);
	for (BOOL bRet = Process32First(hSnap, &pe); bRet; bRet = Process32Next(hSnap, &pe))
	{
		INT iIndex = 0;
		for (INT iKey = pe.th32AccessKey >> 1; iKey; iIndex++)
		{
			iKey >>= 1;
		}
		m_pShare->piProcs[iIndex].dwID = pe.th32ProcessID;
		memcpy(m_pShare->piProcs[iIndex].ptzExeFile, pe.szExeFile, MAX_PATH * sizeof(TCHAR));
	}

	CloseToolhelp32Snapshot(hSnap);
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Prepare stubs in kernel memory
BOOL HijEngine::InitShareData()
{
	// Get OS version
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&vi);

	// Allocate shared memory, which will be accessible from all processes.
	// We will use this memory as argument to LoadLibrary to load the CodeHij into all processes.
	// To support Smartphone 2005 use file mapping instead of m_pShare = (PSHAREDATA) AllocMemInKernelProcess(sizeof(SHAREDATA));
	UINT uStubsSize = GetRealAddress(StubEndMarker) - GetRealAddress(StubStartMarker);
	UINT uSize = sizeof(SHAREDATA) + uStubsSize;
	HANDLE hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, uSize, TEXT("CodeHijMem"));
	if (!hMapping)
	{
		return FALSE;
	}

	m_pShare = (PSHAREDATA) MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	CloseHandle(hMapping);
	if (!m_pShare)
	{
		return FALSE;
	}
	else
	{
		memset(m_pShare, 0, uSize);
		m_pShare->m_uStubSize = uStubsSize;
	}

	InitProcessList();

	// Get access to the kernel data structures.
	m_pSysApiSets = (PCINFO*)(UserKInfo[KINX_APISETS]);
	DWORD dwMask = UserKInfo[KINX_API_MASK];

	// Need to keep a call to the original LoadLibrary.
	m_pShare->pLoadLibraryEx = m_pSysApiSets[SH_WIN32]->m_ppMethods[W32_LoadLibraryEx];
	m_pShare->pGetProcModList = (vi.dwMajorVersion >= 5) ? m_pSysApiSets[SH_WIN32]->m_ppMethods[185] : NULL;
	m_pShare->pGetRomFileInfo = m_pSysApiSets[SH_WIN32]->m_ppMethods[W32_GetRomFileInfo];
	m_pShare->pDebugNotify = m_pSysApiSets[SH_WIN32]->m_ppMethods[W32_DebugNotify];
	m_pShare->pCeGetCurrentTrust = m_pSysApiSets[SH_WIN32]->m_ppMethods[W32_CeGetCurrentTrust];
	m_pShare->pGetProcessIndexFromID = m_pSysApiSets[SH_WIN32]->m_ppMethods[152];

	// Put the path of CodeHij to the shared memory:
	GetModuleFileName(g_hModule, m_pShare->tzCodeHij, _MAX_PATH);
	_tcscpy(m_pShare->tzLMemDebug, L"LMemDebug.DLL"); //LMEMDEBUG_DLL from lmemdebug.h

	// Prepare STUB
	PBYTE pbFunction = GetRealAddress(StubStartMarker);
	memcpy(m_pShare->m_bStub, pbFunction, m_pShare->m_uStubSize);
	pbFunction = m_pShare->m_bStub;

	// Replace placeholder(s) in the body of the stubs by appropriate addresses
	DWORD dwPlaceholders[] = {STUB_PLACEHOLDER_SHARED_MEM};
	DWORD dwReplacements[_NumOf(dwPlaceholders)] = {(DWORD) m_pShare};
	BOOL fFound[_NumOf(dwPlaceholders)] = {FALSE};

	for (UINT i = 0; i < m_pShare->m_uStubSize; i++)
	{
		for (UINT j = 0; j < _NumOf(dwPlaceholders); j++)
		{
			if (pbFunction[i+3] == ((PBYTE) &(dwPlaceholders[j]))[3] &&
				pbFunction[i+2] == ((PBYTE) &(dwPlaceholders[j]))[2] &&
				pbFunction[i+1] == ((PBYTE) &(dwPlaceholders[j]))[1] &&
				pbFunction[i+0] == ((PBYTE) &(dwPlaceholders[j]))[0])
			{
				fFound[j] = TRUE;
				pbFunction[i+3] = ((PBYTE) &(dwReplacements[j]))[3];
				pbFunction[i+2] = ((PBYTE) &(dwReplacements[j]))[2];
				pbFunction[i+1] = ((PBYTE) &(dwReplacements[j]))[1];
				pbFunction[i+0] = ((PBYTE) &(dwReplacements[j]))[0];
			}
		}
	}

	if (!fFound[0])
	{
		return FALSE;
	}

	PBYTE pbLoadCodeHij = (vi.dwMajorVersion) >= 5 ? GetRealAddress(StubLoadCodeHijInCE5) : GetRealAddress(StubLoadCodeHijInCE4);
	m_pShare->pLoadCodeHij = (PFNVOID) (m_pShare->m_bStub + (pbLoadCodeHij - GetRealAddress(StubStartMarker)));

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Load our CodeHij in all running processes so that these processes will be able to access our interceptor routines.
//#define STR_ExcludeExeFile TEXT("CodeHij.exe")
BOOL HijEngine::InjectAllProcess()
{
	// Don't load HijEngine DLL into NK.EXE (process #0) because NK.EXE already has access to all addresses and because it is difficult to unload from there.
	DWORD dwCurProc = GetCurrentProcessId();
	for (INT iProcID = 1; iProcID < _NumOf(m_pShare->piProcs); iProcID++)
	{
		PPROCINFO pProcess = &m_pShare->piProcs[iProcID];
		if ((pProcess->dwID == 0) || pProcess->dwID == dwCurProc)
		{
			// The slot 0 & self is not used
			continue;
		}

		// Avoid to hijack myself
#ifdef STR_ExcludeProc
		if (_tcsicmp(pProcess->ptzExeFile, STR_ExcludeExeFile) == 0)
		{
			continue;
		}
#endif

		HANDLE hProcess = OpenProcess(0, FALSE, pProcess->dwID);
		if (hProcess)
		{
			m_pShare->piProcs[iProcID].hCodeHij = LoadToProcess(hProcess, m_pShare->piProcs[iProcID].dwTrust);
			CloseHandle(hProcess);
		}
	}

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Replaces pointers to Win32 method table, which COREDLL caches in it's data section when each process loads.
// Later most of COREDLL exported methods will use this cached table to call the kernel directly (bypassing the regular API dispatcher)
// Therefore, CodeHij has to replace this cached pointer in each process.
BOOL HijEngine::HookCoreDll(PVOID pvOld, PVOID pvNew)
{
	_Assert(m_pShare);

	// Get address of any exported routine in COREDLL, which is known to use the cached table. EventModify happened to be one.
	PBYTE pEventModify = (PBYTE) GetProcAddress(GetModuleHandle(TEXT("COREDLL")), TEXT("EventModify"));
	if (!pEventModify)
	{
		return FALSE;
	}

	// Now scan the body of the method and look for a pointer to the table. There is no way to know exactly where the method ends, so use 256 as an upper limit.
	// This technique is fragile an may need adjustments for different CPUs and newer versions of the OS.
	DWORD dwAddrInCoreDll = 0;
	__try
	{
		for (INT i = 0; i < 256; i++)
		{
#if defined(ARM) || defined(SH3)
			// ARM don't like misaligned addresses
			if ((DWORD) (pEventModify + i) & 3)
			{
				continue;
			}
#endif

			MEMORY_BASIC_INFORMATION mi;
			PDWORD pdwAddr = *((PDWORD*) (pEventModify + i));
			if (!VirtualQuery(pdwAddr, &mi, sizeof(mi)) || ((mi.Protect != PAGE_READWRITE) && (mi.Protect != PAGE_EXECUTE_READWRITE)))
			{
				continue;
			}

#if defined(ARM) || defined(SH3)
			if ((DWORD) pdwAddr & 3)
			{
				continue;
			}
#endif

			if (*pdwAddr == (DWORD) pvOld)
			{
				// Found pointer to table pvOld at pdwAddr
				dwAddrInCoreDll = (DWORD) pdwAddr;
				break;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// Null handler
	}

	if (dwAddrInCoreDll == 0)
	{
		// Failed to find cached table pvOld in method EventModify
		return FALSE;
	}

	// Now we know where is cached table pointer is hiding.
	// Replace it in all running processes (each process has it's own copy of COREDLL's data section).
	// Here we rely on fact, that on CE a DLL is always loaded at the same address in all processes.
	__try
	{
		for (INT iProcID = 0; iProcID < _NumOf(m_pShare->piProcs); iProcID++)
		{
			PPROCINFO pProcess = &m_pShare->piProcs[iProcID];
			if (pProcess->dwID == 0)
			{
				// This slot is not used
				continue;
			}

			DWORD dwProcBaseAddr = 0x2000000 * (iProcID + 1);
			PDWORD pdwAddr = (PDWORD) ((dwAddrInCoreDll & 0x1FFFFFF) + dwProcBaseAddr);
			MEMORY_BASIC_INFORMATION mi;
			if (!VirtualQuery(pdwAddr, &mi, sizeof(mi)) || ((mi.Protect != PAGE_READWRITE) && (mi.Protect != PAGE_EXECUTE_READWRITE)))
			{
				continue;
			}

			if (*pdwAddr == (DWORD) pvOld)
			{
				*pdwAddr = (DWORD) pvNew;

				// Double check. This is not really necessary.
				FlushInstructionCache(GetCurrentProcess(), 0, 0);
				_Assert(*pdwAddr == (DWORD) pvNew);
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// Null handler
	}

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a copy of the system API (CINFO structure, method and signature tables).
// Then replace a pointer to the original CINFO by the new one. 
// We keep a table m_Api of such replacement APIs, so CreateDupApi will not allocate a new API for the same original more than once.
// INT iApiSetID - original API set ID to replace
BOOL HijEngine::CreateDupApi(INT iApiSetID, INT iMinNumMethods)
{
	if (iApiSetID < 0 || iApiSetID >= _NumOf(m_Api))
	{
		// Out of range
		return FALSE;
	}

	PAPIINFO pApi = m_Api + iApiSetID;
	if (pApi->bUsed)
	{
		// Already hooked
		return TRUE;
	}

	HANDLE hCurProc = GetCurrentProcess();
	PCINFO pOrigApiSet = m_pSysApiSets[iApiSetID];

	PFNVOID* pHookMthds = NULL;
	PDWORD ppdwRetHookSigns = NULL;
	PFNVOID* pOrgMethods = (PFNVOID*) MapProcessAddress(pOrigApiSet->m_ppMethods, pOrigApiSet->m_pProcessServer);
	PDWORD pOrigSignatures = (PDWORD) MapProcessAddress(pOrigApiSet->m_pdwMethodSignatures, pOrigApiSet->m_pProcessServer);

	PCINFO pOurApiSet = (PCINFO) AllocMemInKernelProcess(sizeof(CINFO));
	if (!pOurApiSet)
	{
		return FALSE;
	}

	INT iNewNum = ((INT) pOrigApiSet->m_wNumMethods > iMinNumMethods) ? (INT) pOrigApiSet->m_wNumMethods : iMinNumMethods;
	if (!CreateDupMethodTable(pOrigApiSet->m_wNumMethods, iNewNum, pOrgMethods, pOrigSignatures, pHookMthds, ppdwRetHookSigns))
	{
		FreeMemInKernelProcess(pOurApiSet);
		return FALSE;
	}
	if (iApiSetID == SH_WIN32)
	{
		m_ppHookWin32Methods = pHookMthds;
		m_pdwHookWin32Signs = ppdwRetHookSigns;
	}

	// Use the same name, dispatch type, API ID and server as the original API
	memcpy(pOurApiSet, pOrigApiSet, sizeof(CINFO));

	// But replace the method and signature tables
	pOurApiSet->m_ppMethods = pHookMthds;
	pOurApiSet->m_pdwMethodSignatures = ppdwRetHookSigns;
	pOurApiSet->m_wNumMethods = (WORD) iNewNum;

	// Now fill our APIINFO structure so we can undo the API replacement later.
	pApi->bUsed = TRUE;
	pApi->bSwapped = FALSE;
	pApi->iOrgApiSetID = iApiSetID;
	pApi->pOrgApiSet = pOrigApiSet;
	pApi->pNewApiSet = pOurApiSet;

	// And finally, replace the pointer to the original CINFO by the pointer to ours.
	m_pSysApiSets[iApiSetID] = pOurApiSet;
	pApi->bSwapped = TRUE;

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Allocate memory in the kernel process space and copy the given method and signature table to this memory.
BOOL HijEngine::CreateDupMethodTable(INT iOrgNum, INT iNewNum, PFNVOID* pMethods, PDWORD pdwSigns, PFNVOID*& ppRetHookMethods, PDWORD& pdwRetHookSigns)
{
	INT iSize = (sizeof(PFNVOID) * iNewNum + sizeof(DWORD) *iNewNum);
	PBYTE pbRemote = (PBYTE) AllocMemInKernelProcess(iSize);
	if (!pbRemote)
	{
		return FALSE;
	}

	ppRetHookMethods = (PFNVOID*) pbRemote;
	pdwRetHookSigns = (PDWORD) (pbRemote + sizeof(PFNVOID) * iNewNum);

	INT i = 0;
	for (; i < iOrgNum; i++)
	{
		// Our replaced API will be served by our process and we always should provide a signature table. 
		// But the original signature table pointer will be NULL if the original API is served by the kernel (therefore no pointer mapping is required).
		// In that case fill our table by FNSIG0, which means that no mapping is required.
		ppRetHookMethods[i] = pMethods[i];
		pdwRetHookSigns[i] = pdwSigns ? pdwSigns[i] : FNSIG0();
	}

	for (; i < iNewNum; i++)
	{
		ppRetHookMethods[i] = NULL;
		pdwRetHookSigns[i] = FNSIG0();
	}

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Load CodeHij to process, dwTruct to retrieve target process trust
HMODULE HijEngine::LoadToProcess(HANDLE hProcess, DWORD& dwTrust)
{
	FARPROC pCeGetCurrentTrust = GetProcAddress(GetModuleHandle(TEXT("COREDLL")), TEXT("CeGetCurrentTrust"));
	if (pCeGetCurrentTrust)
	{
		CALLBACKINFO ci;
		ci.hProcess = hProcess;
		ci.pFunction = pCeGetCurrentTrust;
		ci.pvArg0 = NULL;
		dwTrust = PerformCallBack4(&ci, 0, 0, 0);
	}
	else
	{
		dwTrust = OEM_CERTIFY_FALSE;
	}

	FARPROC pLoadLibraryW = GetProcAddress(GetModuleHandle(TEXT("COREDLL")), TEXT("LoadLibraryW"));
	if (!pLoadLibraryW)
	{
		return NULL;
	}

	// On Smartphone 2005 LoadLibraryW throws exception if target is not trusted. For details see CreateRemoteThread.
	if (dwTrust == OEM_CERTIFY_TRUST)
	{
		CALLBACKINFO ci;
		ci.hProcess = hProcess;
		ci.pFunction = pLoadLibraryW;
		ci.pvArg0 = m_pShare->tzCodeHij;
		return (HMODULE) PerformCallBack4(&ci, 0, 0, 0);
	}
	else
	{
		DWORD dwRet = 0;
		DWORD dwSize = (_tcslen(m_pShare->tzCodeHij) + 1) * sizeof(TCHAR);
		CreateRemoteThread(hProcess, pLoadLibraryW, 0, (PBYTE) m_pShare->tzCodeHij, dwSize, 1000, &dwRet);
		return (HMODULE) dwRet;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Unload CodeHij from a process, where it was loaded by LoadToProcess.
BOOL HijEngine::UnloadFromProcess(HANDLE hProcess, DWORD dwTrust, HMODULE hModule)
{
	FARPROC pFreeLibrary = GetProcAddress(GetModuleHandle(TEXT("COREDLL")), TEXT("FreeLibrary"));
	if (!pFreeLibrary)
	{
		return FALSE;
	}

	if (dwTrust == OEM_CERTIFY_TRUST)
	{
		CALLBACKINFO ci;
		ci.hProcess = hProcess;
		ci.pFunction = pFreeLibrary;
		ci.pvArg0 = (PVOID) hModule;
		return PerformCallBack4(&ci, 0, 0, 0);
	}
	else
	{
		DWORD bRet;
		return CreateRemoteThread(hProcess, pFreeLibrary, (DWORD) hModule, NULL, 0, 1000, &bRet) && bRet;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Allocate memory in the process #0 - nk.exe, which is accessible from all otherprocesses. 
// We need this memory to create duplicate method tables, which we will use to substitute original method tables provided by the kernel or by server processes.
// Note: CE 5.0 has CeVirtualSharedAlloc, but it have not been tested yet.
PVOID HijEngine::AllocMemInKernelProcess(UINT uSize)
{
	PVOID pvMem = (PVOID) CallCoreDllInKernelProcess(TEXT("VirtualAlloc"), 0, uSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pvMem)
	{
		return NULL;
	}

	// Currently pvMem is slot 0 address, so it is not valid unless the current process is NK.EXE. So we need to map it to slot 1, which is the home of NK.EXE.
	HANDLE hNKProc = OpenProcess(0, FALSE, m_pShare->piProcs[0].dwID);
	if (!hNKProc)
	{
		return NULL;
	}

	PVOID pvRemote = MapPtrToProcess(pvMem, hNKProc);
	if (!pvRemote)
	{
		FreeMemInKernelProcess(pvMem);
	}
	CloseHandle(hNKProc);
	return pvRemote;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Free memory, which was allocated in the nk.exe process space by AllocMemInKernelProcess.
BOOL HijEngine::FreeMemInKernelProcess(PVOID pvMem)
{
	return CallCoreDllInKernelProcess(TEXT("VirtualFree"), (DWORD) pvMem, 0, MEM_RELEASE);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Like MapPtrToProcess, but use undocumented PROCESS structure
PVOID HijEngine::MapProcessAddress(PVOID pvAddress, PPROCESS pProcess)
{
	// Slot 0 and process is not the kernel
	if (((DWORD) pvAddress) < 0x2000000 && pProcess)
	{
		PVOID pvOld = pvAddress;
		BYTE bIndex = *(((PBYTE) pProcess) + PROCESS_NUM_OFFSET);
		pvAddress = (PVOID) (((DWORD) pvAddress) + 0x02000000 * (bIndex + 1));
	}
	return pvAddress;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Call other procedure
typedef DWORD (*PCallForward)(PCALLBACKINFO pci, DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4, DWORD dw5, DWORD dw6, DWORD dw7);
DWORD HijEngine::CallForward(HANDLE hProcess, FARPROC pFunction, DWORD dw0, DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4, DWORD dw5, DWORD dw6, DWORD dw7)
{
	CALLBACKINFO ci;
	ci.hProcess = hProcess;
	ci.pFunction = pFunction;
	ci.pvArg0 = (PVOID) dw0;
	return ((PCallForward) IMPLICIT_CALL(SH_WIN32, 113))(&ci, dw1, dw2, dw3, dw4, dw5, dw6, dw7);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uses undocumented method PerformCallBack4 to call exported methods from COREDLL in the specified process.
DWORD HijEngine::CallCoreDllInProcess(HANDLE hProcess, PCTSTR ptzMethod, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3, DWORD dwParam4)
{
	__try
	{
		CALLBACKINFO ci;
		ci.pFunction = GetProcAddress(LoadLibrary(TEXT("COREDLL")), ptzMethod);
		if (!ci.pFunction)
		{
			return 0;
		}

		ci.hProcess = hProcess;
		ci.pvArg0 = (PVOID) dwParam1;
		return PerformCallBack4(&ci, dwParam2, dwParam3, dwParam4);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Call exported methods from COREDLL in the kernel process (nk.exe).
DWORD HijEngine::CallCoreDllInKernelProcess(PCTSTR ptzMethod, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3, DWORD dwParam4)
{
	HANDLE hNKProc = OpenProcess(0, FALSE, m_pShare->piProcs[0].dwID);
	if (!hNKProc)
	{
		return 0;
	}
	DWORD dwRet = CallCoreDllInProcess(hNKProc, ptzMethod, dwParam1, dwParam2, dwParam3, dwParam4);
	CloseHandle(hNKProc);
	return dwRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// For Smartphone 5.0, PerformCallBack4(LoadLibraryW) will throw exception 0xC0000030 (STATUS_INVALID_PARAMETER_MIX) if the target process is not fully trusted.
// On CE 5.0, LoadLibraryW call LoadLibraryExW, which passes an address of a local variable on the stack down to the kernel as the third parameter.
// But the stack belongs to our process and the target application (since it is not trusted) does not have access to our stack.
// Therefore, we have to perform LoadLibraryW on the stack, which belongs to the target process. The easiest way to switch the stack is to create a thread.
// Since ThreadProc has the same arguments as LoadLibraryW, we can pass address of LoadLibraryW directly to CreateThread, then wait for the thread to finish.
// If pbData not NULL, we will allocate memory in the target process.
BOOL HijEngine::CreateRemoteThread(HANDLE hProcess, FARPROC pThreadProc, DWORD dwParam, PBYTE pbData, INT iDataSize, INT iTimeout, PDWORD pdwRet)
{
	// Initialize procedure address
	HMODULE hModule = GetModuleHandle(TEXT("COREDLL"));
	FARPROC pCreateThread = GetProcAddress(hModule, TEXT("CreateThread"));
	FARPROC pWaitForMultipleObjects = GetProcAddress(hModule, TEXT("WaitForMultipleObjects"));
	FARPROC pGetExitCodeThread = GetProcAddress(hModule, TEXT("GetExitCodeThread"));
	FARPROC pCloseHandle = GetProcAddress(hModule, TEXT("CloseHandle"));
	FARPROC pmalloc = GetProcAddress(hModule, TEXT("malloc"));
	FARPROC pfree = GetProcAddress(hModule, TEXT("free"));
	if (!pCreateThread || !pWaitForMultipleObjects || !pGetExitCodeThread || !pCloseHandle || !pmalloc || !pfree)
	{
		return FALSE;
	}

	// IMPLICIT_CALL(SH_WIN32, 113) call kernel routine SC_CallForward, which is similar to PerformCallBack4, but accepts 8 arguments instead of 4.
	// In fact, PerformCallBack4 in COREDLL performs IMPLICIT_CALL(SH_WIN32, 113)
	PBYTE pbMem = NULL;
	PDWORD pdwRemote = NULL;
	if (iTimeout || pdwRet || pbData)
	{
		// We need at least 4 bytes of data in the target process
		INT iSize = iDataSize + sizeof(DWORD);
		pbMem = (PBYTE) CallForward(hProcess, pmalloc, iSize);
		if (!pbMem)
		{
			return FALSE;
		}
		pdwRemote = (PDWORD) MapPtrToProcess(pbMem, hProcess);
		if (pbData)
		{
			memcpy(pdwRemote + 1, pbData, iDataSize);
			dwParam = (DWORD) (pdwRemote + 1);
		}
	}

	BOOL bRet = TRUE;
	HANDLE hThread = (HANDLE) CallForward(hProcess, pCreateThread, 0, 0, (DWORD) pThreadProc, dwParam);
	if (hThread)
	{
		if (iTimeout)
		{
			*pdwRemote = (DWORD) hThread;
			if (CallForward(hProcess, pWaitForMultipleObjects, 1, (DWORD) pdwRemote, 0, iTimeout) != WAIT_OBJECT_0)
			{
				bRet = FALSE;
			}
		}

		if (pdwRet)
		{
			*pdwRet = 0;
			*pdwRemote = 0;
			if (!CallForward(hProcess, pGetExitCodeThread, (DWORD) hThread, (DWORD) pdwRemote))
			{
				// GetExitCodeThread failed in target process
				bRet = FALSE;
			}
			else if (*pdwRemote == STILL_ACTIVE)
			{
				bRet = FALSE;
			}
			else
			{
				*pdwRet = *pdwRemote;
				bRet = TRUE;
			}
		}

		CallForward(hProcess, pCloseHandle, (DWORD) hThread);
	}

	if (pbMem)
	{
		CallForward(hProcess, pfree, (DWORD) pbMem);
	}
	return bRet;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
