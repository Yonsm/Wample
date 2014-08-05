

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Header
#include <Windows.h>
HMODULE g_hInst = NULL;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CodeHij struct
struct CodeHij
{
#pragma pack(push, 4)
	DWORD m_dwBak[2];
	DWORD m_dwMovPc;
	DWORD m_dwDest;
#pragma pack(pop)

	inline BOOL IsHooked()
	{
		return (BOOL) m_dwDest;
	}

	BOOL Hook(FARPROC pfnNew, FARPROC pfnApi)
	{
		if (!IsHooked() && pfnApi)
		{
			// Generate stub
			m_dwMovPc = 0xE51FF004;
			m_dwDest = (DWORD) pfnApi + sizeof(m_dwBak);
			ReadProcessMemory(GetCurrentProcess(), pfnApi, (PBYTE) &m_dwBak, sizeof(m_dwBak), NULL);

			// Modify API entry
			DWORD dwCode[2] = {0xE51FF004, (DWORD) pfnNew};
			if (!WriteProcessMemory(GetCurrentProcess(), pfnApi, (PBYTE) &dwCode, sizeof(dwCode), NULL))
			{
				// Failed
				m_dwDest = 0;
			}
		}
		return IsHooked();
	}

	BOOL Hook(FARPROC pfnNew, PCTSTR ptzApi, PCTSTR ptzModule = TEXT("COREDLL"))
	{
		return Hook(pfnNew, GetProcAddress(GetModuleHandle(ptzModule), ptzApi));
	}

	BOOL Unhook()
	{
		if (IsHooked())
		{
			m_dwDest = 0;
			return WriteProcessMemory(GetCurrentProcess(), (PVOID) (m_dwDest - sizeof(m_dwBak)), (PBYTE) &m_dwBak, sizeof(m_dwBak), NULL);
		}
		else
		{
			return TRUE;
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hook data
#pragma comment(linker, "/SECTION:CodeInj,SRW")
#pragma data_seg("CodeInj")
CodeHij g_hMessageBox = {0};
//CodeHij g_hTrackPopu = {0};
#pragma data_seg()

UINT WINAPI MyMessageBox(HWND hWnd, PCTSTR ptzText, PCTSTR ptzCaption, UINT uFlags)
{
	return ((UINT (WINAPI *)(HWND hWnd, PCTSTR ptzText, PCTSTR ptzCaption, UINT uFlags)) &g_hMessageBox)
		(hWnd, ptzText, TEXT("Hooked by CodeHij!"), uFlags);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Entry
BOOL APIENTRY DllMain(HANDLE hInstance, DWORD dwReason, PVOID pvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		g_hInst = (HMODULE) hInstance;
		if (!g_hMessageBox.IsHooked())
		{
			g_hMessageBox.Hook((FARPROC) MyMessageBox, TEXT("MessageBoxW"));
		}
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instructs an in-process server to create its registry entries for all classes supported in this server module.
#pragma comment(linker, "/EXPORT:DllRegisterServer,PRIVATE")
STDAPI DllRegisterServer()
{
	HKEY hKey;
	HRESULT hResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("System\\Kernel"), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (hResult == S_OK)
	{
		TCHAR tzPath[MAX_PATH];
		UINT uLen = GetModuleFileName(g_hInst, tzPath, MAX_PATH);
		MessageBoxW(NULL, tzPath, TEXT(__FUNCTION__), MB_ICONINFORMATION);

		// TODO: Need to merge exist item
		hResult = RegSetValueEx(hKey, TEXT("InjectDLL"), 0, REG_MULTI_SZ, (PBYTE) tzPath, (uLen + 1) * sizeof(TCHAR));
		RegCloseKey(hKey);
	}
	g_hMessageBox.Unhook();
	return hResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instructs an in-process server to remove only those entries created through DllRegisterServer.
#pragma comment(linker, "/EXPORT:DllUnregisterServer,PRIVATE")
STDAPI DllUnregisterServer()
{
	HKEY hKey;
	HRESULT hResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("System\\Kernel"), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (hResult == S_OK)
	{
		// TODO: 
		hResult = RegDeleteValue(hKey, TEXT("InjectDLL"));
		RegCloseKey(hKey);
	}
	g_hMessageBox.Unhook();
	return hResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
