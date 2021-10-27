#include<iostream>
#include"Struct.h"
#include"FileSystem.h"
#include<bitset>
#include<tchar.h>
#include<vector>
#include<string>
#include<Windows.h>

using namespace std;

int _tmain(int argc, _TCHAR* argv[]) {//测试代码
	if (argc==1) {//主进程代码  服务器
		cout << "\n\t┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓" << endl;
		cout << "\t┃                201930340413                     ┃" << endl;
		cout << "\t┃                  19计科2班                      ┃" << endl;
		cout << "\t┃                   高怀基                        ┃" << endl;
		cout << "\t┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛" << endl;
		cout << "服务器主进程已启动，等待连接..."<< endl;
		HANDLE hlog, fileSystemMain,haccessTable,haccessTableMutex; //声明句柄
		hlog = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, sizeof(logInfo), TEXT("log")); //创建注册用的映射文件
		if (hlog == NULL)
		{
			cout << "创建文件映射失败" << endl;
			return -1;
		}
		logInfo* login;
		login = (logInfo*)MapViewOfFile(hlog, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);//用来传递登录成功的userid
		fileSystemMain = CreateSemaphore(NULL, 0, 1, TEXT("fileSystemMain"));//文件系统可以注册写东西
		haccessTable = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, sizeof(AccessTable), TEXT("accessTable"));
		haccessTableMutex = CreateSemaphore(NULL, 1, 1, TEXT("Mutex"));
		if (haccessTable == NULL|| haccessTableMutex==NULL) {
			cout << "创建accessTable或Mutex失败" << endl;
			return -1;
		}
		//haccessTableMutex创建了就行，在主进程中用不到，不需要打开
		AccessTable* accessTable= (AccessTable*)MapViewOfFile(haccessTable, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);//打开文件映射
		ZeroMemory(accessTable->readNum, sizeof(accessTable->readNum));//初始化访问登记表
		accessTable->writeFlag.reset();
		
		vector<STARTUPINFO> startUpInfo;
		vector<PROCESS_INFORMATION> PInfo;
		for(int i=0;i<5;++i) {//根据命令，输入返回值
			WaitForSingleObject(fileSystemMain, INFINITE);//文件系统先写东西
			int uid = login->uid;
			//创建子进程
			STARTUPINFO startup_info;
			startUpInfo.push_back(startup_info);
			PROCESS_INFORMATION process_info;
			PInfo.push_back(process_info);

			wchar_t argument[256];
			//wsprintf(argument, L"\"%s\" Hello", argv[0]);
			wsprintf(argument, L"%s %d",argv[0], uid);
			ZeroMemory(&(PInfo[i]), sizeof(PInfo[i]));
			ZeroMemory(&(startUpInfo[i]), sizeof(startUpInfo[i]));
			startUpInfo[i].cb = sizeof(startUpInfo[i]);
			if (CreateProcess(NULL, argument, 0, 0, 0, CREATE_NEW_CONSOLE, 0, 0, &(startUpInfo[i]), &(PInfo[i])) == 0)
			{
				cout << "Error " << GetLastError() << endl;
			}
		};
	}
	else {//子进程代码 
		Sleep(100);//等0.1秒钟，等待shell注册各种handle
		int uid;
		uid = _tstoi(argv[1]);
		FileSystem f(uid);
		wcout << "用户已登录,uid: " << argv[1] << endl;
	
		//-----------开始通信----------
		f.getcmd("help");
		f.outStr += '>';
		strcpy_s((f.file)->ret, 1024 * 10, f.outStr.c_str());
		Sleep(100);//等待写入
		ReleaseSemaphore(f.shell,1, NULL);//通知shell可以执行
		WaitForSingleObject(f.disk, INFINITE);//阻塞disk
		while (1) {
			f.outStr.clear();
			cout << "1" << endl;
			f.file->errorFlag = 0;//开始操作前，错误标志应该清0
			string cmd(f.file->cmd);
			cout << cmd << endl;
			f.getcmd(cmd);
			f.outStr += f.getWd();
			f.outStr += '>';
			strcpy_s(f.file->ret, 1024 * 10, f.outStr.c_str());//写结果
			cout << f.outStr.c_str();
			Sleep(100);//缓冲时间，等待写完
			if (cmd.size()>=3&&cmd.substr(0, 3) == "cat" && f.file->errorFlag == 0) {//成功进行读操作，则不需要在此处唤醒shell读操作停滞5秒，已经在系统内提前release了,不需要在这里通知
				cout << '2' << endl;
				WaitForSingleObject(f.disk, INFINITE);//阻塞disk
			}
			//else if(cmd.size() >= 3 && cmd.substr(0, 6) == "format"&&f.file->errorFlag == 0)
			else {
				cout << '3' << endl;
				ReleaseSemaphore(f.shell, 1, NULL);//通知shell可以获取命令继续执行
				WaitForSingleObject(f.disk, INFINITE);//阻塞disk
			}
		}
	}
}