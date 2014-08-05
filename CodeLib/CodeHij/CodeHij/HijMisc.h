

// Header
#pragma once
#include <Windows.h>

// Kernel data structure declarations
// The key for accessing CE kernel data structure (KDataStruct) is PUserKData.
// PUserKData is defined in kfuncs.h in SDK as 0xFFFFC800 on ARM and 0x00005800 on other CPUs.
// It is used in the same header to define macros GetCurrentThreadId and GetCurrentProcessId, which directly access the kernel data structures.
// Several kernel data structures are accessed using predefined offsets from PUserKData.
// For example, SYSHANDLE_OFFSET (defined in kfuncs.h), gives access to an array of system handles, 
// KINFO_OFFSET(defined in PUBLIC\COMMON\OAK\INC\pkfuncs.h) gives access to UserKInfo array.
// About 30 indexes in this array are defined in pkfuncs.h (KINX_*) to provide access to such kernel data structures as process array, module list, kernel heap, etc.
// Here we are only interested in KINX_APISETS and KINX_API_MASK.
// KINX_APISETS slot holds a pointer to an array of system API sets (SystemAPISets).
// KINX_API_MASK slot is a bit mask of installed APIs.

#define KINFO_OFFSET					0x300

#define KINX_API_MASK					18
#define KINX_APISETS					24

#define UserKInfo						((PLONG) (PUserKData + KINFO_OFFSET))

// Pointer to struct Process declared in Kernel.h.
typedef PVOID PPROCESS;

#define PROCESS_NUM_OFFSET				0		// Process number (index of the slot)
#define PROCESS_NAME_OFFSET				0x20	// Pointer to the process name

// Structure CINFO, which holds an information about an API (originally declared in PRIVATE\WINCEOS\COREOS\NK\INC\Kernel.h).
typedef struct _CINFO
{
	CHAR m_szApiName[4];				// Used in CreateAPISet and QueryAPISetID
	BYTE m_byDispatchType;				// Kernel vs user mode, handle-based vs direct.
	BYTE m_byApiHandle;					// ID of the API, such as SH_WIN32, SH_SHELL, etc
	WORD m_wNumMethods;					// Number of methods listed in array m_ppMethods
	PFNVOID* m_ppMethods;				// Array of pointers to methods
	PDWORD m_pdwMethodSignatures;		// DWORD-encoded methods arguments
	PPROCESS m_pProcessServer;			// Point to a process, which serves this API
}
CINFO, *PCINFO;

// pkfuncs.h defines a lot of signature generation macros for methods with different number of arguments.
// The purpose of signatures is to figure out which arguments are pointers, so the dispatcher may properly map them.
// We only need to redefine a single macro FNSIG0(), which means that no arguments will be mapped.
#define FNSIG0()					0

// psyscall.h defined macros for calling API methods, such as IMPLICIT_CALL
#if defined(x86)
#define FIRST_METHOD					0xFFFFFE00
#define APICALL_SCALE					2
#elif defined(ARM)
#define FIRST_METHOD					0xF0010000
#define APICALL_SCALE					4
#elif defined(SHx)
#define FIRST_METHOD					0xFFFFFE01
#define APICALL_SCALE					2
#else
#error "Unknown CPU type"
#endif

#define HANDLE_SHIFT					8
#define IMPLICIT_CALL(ApiID, MethodIdx)	(FIRST_METHOD - ((ApiID) << HANDLE_SHIFT | (MethodIdx)) * APICALL_SCALE)

// Undocumented structures declared in PUBLIC\COMMON\OAK\INC\pkfuncs.h
typedef struct _CALLBACKINFO
{
	HANDLE hProcess;					// destination process
	FARPROC pFunction;						// function to call in destination process
	PVOID pvArg0;						// arg0 data
}
CALLBACKINFO, *PCALLBACKINFO;

// Documented in Platform Builder help and declared in PUBLIC\COMMON\OAK\INC\pkfuncs.h
extern "C" BOOL SetKMode(BOOL fMode);
extern "C" DWORD SetProcPermissions(DWORD dwPermissions) ;
extern "C" PVOID MapPtrToProcess(PVOID pvAddress, HANDLE hProcess);

// Undocumented functions exported from COREDLL and declared in PUBLIC\COMMON\OAK\INC\pkfuncs.h
extern "C" DWORD GetProcessIndexFromID(HANDLE hProcess);
extern "C" BOOL GetRomFileInfo(DWORD dwType, LPWIN32_FIND_DATA pFind, DWORD dwCount);
extern "C" DWORD PerformCallBack4(PCALLBACKINFO pci, DWORD dw1, DWORD dw2, DWORD dw3);

// The following are API handles from PUBLIC\COMMON\OAK\INC\psyscall.h, in addition to SH_* handles defined in SDK kfuncs.h
#define HT_EVENT						4
#define HT_MUTEX						5
#define HT_APISET						6
#define HT_FILE							7
#define HT_FIND							8
#define HT_DBFILE						9
#define HT_DBFIND						10
#define HT_SOCKET						11
#define HT_INTERFACE					12
#define HT_SEMAPHORE					13
#define HT_FSMAP						14
#define HT_WNETENUM						15

// Some methods in the SH_WIN32 table. The full list is in psyscall.h.
#define W32_GetRomFileInfo				32
#define W32_CeGetCurrentTrust			85
#define W32_DebugNotify					118
#define W32_LoadLibraryEx				148

// HT_FILE
#define	_ReadFile						2
#define	_WriteFile						3
#define	_GetFileSize					4
#define	_SetFilePointer					5
#define	_GetFileInformationByHandle		6
#define	_FlushFileBuffers				7
#define	_GetFileTime					8
#define	_SetFileTime					9
#define	_SetEndOfFile					10
#define	_DeviceIoControl				11

// Some methods in the SH_FILESYS_APIS table
#define	_CreateDirectoryW				2
#define	_RemoveDirectoryW				3
#define	_MoveFileW						4
#define	_DeleteFileW					6
#define	_GetFileAttributesW				7
#define	_FindFirstFileW					8
#define	_CreateFileW					9
#define	_SetFileAttributesW				31
#define	_DeleteAndRenameFile			43
#define	_CheckPassword					29
#define	_GetDiskFreeSpaceExW			54
#define	_GetTempPathW					27
#define	_FindFirstFileExW				63
#define	_SignalStarted					66
#define	_CreateMsgQueue					79
#define	_OpenMsgQueue					80
#define	_ReadMsgQueue					81
#define	_WriteMsgQueue					82
#define	_GetMsgQueueInfo				83
#define	_CloseMsgQueue					84
#define	_FindFirstChangeNotificationW	88
#define	_FindNextChangeNotification		89
#define	_FindCloseChangeNotification	90
#define	_CeGetFileNotificationInfo		91
#define	_RequestDeviceNotifications		92
#define	_StopDeviceNotifications		93
#define	_AdvertiseInterface				94
#define	_ReportEventW					95
#define	_RegisterEventSourceW			96
#define	_DeregisterEventSource			97
#define	_ClearEventLogW					98
#define	_CeCertVerify					99
#define	_CeCreateTokenFromAccount		100
#define	_CvtSDToStr						102
#define	_CvtStrToSD						103
#define	_ADBCreateAccount				107
#define	_ADBDeleteAccount				108
#define	_ADBGetAccountProperty			109
#define	_ADBSetAccountProperties		110
#define	_ADBGetAccountSecurityInfo		111
#define	_ADBAddAccountToGroup			112
#define	_ADBRemoveAccountFromGroup		113
#define	_ADBEnumAccounts				114
#define	_CeFsIoControlW					115
#define	_OpenEventLog					116
#define	_CloseEventLog					117
#define	_BackupEventLogW				118
#define	_LockEventLog					119
#define	_UnLockEventLog					120
#define	_ReadEventLogRaw				121
#define	_SetFileSecurityW				147
#define	_GetFileSecurityW				148

#define	_RegCloseKey					17
#define	_RegFlushKey					49
#define	_RegCreateKeyExW				18
#define	_RegDeleteKeyW					19
#define	_RegDeleteValueW				20
#define	_RegEnumValueW					21
#define	_RegEnumKeyExW					22
#define	_RegOpenKeyExW					23
#define	_RegQueryInfoKeyW				24
#define	_RegQueryValueExW				25
#define	_RegSetValueExW					26
#define	_CeRegTestSetValueW				134
#define	_GetUserInformation				69
#define	_PSLGetDeviceUniqueID			135
#define	_PSLCryptProtectData			85
#define	_PSLCryptUnprotectData			86
#define	_RegCopyFile					41
#define	_RegRestoreFile					44
#define	_RegSaveKey						64
#define	_RegReplaceKey					65
#define	_SetCurrentUser					67
#define	_SetUserData					68
#define	_PSLGenRandom					87
#define	_CeFindFirstRegChange			104
#define	_CeFindNextRegChange			105
#define	_CeFindCloseRegChange			106

// SH_TAPI
#define	_lineClose						2
#define	_lineConfigDialogEdit			3
#define	_lineDeallocateCall				4
#define	_lineDrop						5
#define	_lineGetDevCaps					6
#define	_lineGetDevConfig				7
#define	_lineGetTranslateCaps			8
#define	_TAPIlineInitialize				9
#define	_lineMakeCall					10
#define	_lineNegotiateAPIVersion		11
#define	_lineOpen						12
#define	_lineSetDevConfig				13
#define	_lineSetStatusMessages			14
#define	_TAPIlineShutdown				15
#define	_lineTranslateAddress			16
#define	_lineTranslateDialog			17
#define	_lineGetID						18
#define	_lineAddProvider				19
#define	_lineSetCurrentLocation			20
#define	_lineAccept						21
#define	_lineAddToConference			22
#define	_lineAnswer						23
#define	_lineBlindTransfer				24
#define	_lineCompleteTransfer			25
#define	_lineDevSpecific				26
#define	_lineDial						27
#define	_lineForward					28
#define	_lineGenerateDigits				29
#define	_lineGenerateTone				30
#define	_lineGetAddressCaps				31
#define	_lineGetAddressID				32
#define	_lineGetAddressStatus			33
#define	_lineGetAppPriority				34
#define	_lineGetCallInfo				35
#define	_lineGetCallStatus				36
#define	_lineGetConfRelatedCalls		37
#define	_lineGetIcon					38
#define	_lineGetLineDevStatus			39
#define	_lineGetMessage					40
#define	_lineGetNewCalls				41
#define	_lineGetNumRings				42
#define	_lineGetProviderList			43
#define	_lineGetStatusMessages			44
#define	_lineHandoff					45
#define	_lineHold						46
#define	_TAPIlineInitializeEx			47
#define	_lineMonitorDigits				48
#define	_lineMonitorMedia				49
#define	_lineNegotiateExtVersion		50
#define	_linePickup						51
#define	_linePrepareAddToConference		52
#define	_lineRedirect					53
#define	_lineReleaseUserUserInfo		54
#define	_lineRemoveFromConference		55
#define	_lineSendUserUserInfo			56
#define	_lineSetAppPriority				57
#define	_lineSetCallParams				58
#define	_lineSetCallPrivilege			59
#define	_lineSetMediaMode				60
#define	_lineSetNumRings				61
#define	_lineSetTerminal				62
#define	_lineSetTollList				63
#define	_lineSetupConference			64
#define	_lineSetupTransfer				65
#define	_lineSwapHold					66
#define	_lineUnhold						67
#define	_phoneClose						68
#define	_phoneConfigDialog				69
#define	_phoneDevSpecific				70
#define	_phoneGetDevCaps				71
#define	_phoneGetGain					72
#define	_phoneGetHookSwitch				73
#define	_phoneGetIcon					74
#define	_phoneGetID						75
#define	_phoneGetMessage				76
#define	_phoneGetRing					77
#define	_phoneGetStatus					78
#define	_phoneGetStatusMessages			79
#define	_phoneGetVolume					80
#define	_TAPIphoneInitializeEx			81
#define	_phoneNegotiateAPIVersion		82
#define	_phoneNegotiateExtVersion		83
#define	_phoneOpen						84
#define	_phoneSetGain					85
#define	_phoneSetHookSwitch				86
#define	_phoneSetRing					87
#define	_phoneSetStatusMessages			88
#define	_phoneSetVolume					89
#define	_TAPIphoneShutdown				90
#define	_lineStartDTMF					91
#define	_lineStopDTMF					92

// For MORE API index declaration, please refer to Platform Builder\PUBLIC\COMMON\OAK\INC\m*.h
