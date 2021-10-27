#include<iostream>
#include"Struct.h"
#include"FileSystem.h"
#include<bitset>
#include<tchar.h>
#include<vector>
#include<string>
#include<Windows.h>

using namespace std;

int _tmain(int argc, _TCHAR* argv[]) {//���Դ���
	if (argc==1) {//�����̴���  ������
		cout << "\n\t������������������������������������������������������������������������������������������������������" << endl;
		cout << "\t��                201930340413                     ��" << endl;
		cout << "\t��                  19�ƿ�2��                      ��" << endl;
		cout << "\t��                   �߻���                        ��" << endl;
		cout << "\t������������������������������������������������������������������������������������������������������" << endl;
		cout << "���������������������ȴ�����..."<< endl;
		HANDLE hlog, fileSystemMain,haccessTable,haccessTableMutex; //�������
		hlog = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, sizeof(logInfo), TEXT("log")); //����ע���õ�ӳ���ļ�
		if (hlog == NULL)
		{
			cout << "�����ļ�ӳ��ʧ��" << endl;
			return -1;
		}
		logInfo* login;
		login = (logInfo*)MapViewOfFile(hlog, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);//�������ݵ�¼�ɹ���userid
		fileSystemMain = CreateSemaphore(NULL, 0, 1, TEXT("fileSystemMain"));//�ļ�ϵͳ����ע��д����
		haccessTable = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, sizeof(AccessTable), TEXT("accessTable"));
		haccessTableMutex = CreateSemaphore(NULL, 1, 1, TEXT("Mutex"));
		if (haccessTable == NULL|| haccessTableMutex==NULL) {
			cout << "����accessTable��Mutexʧ��" << endl;
			return -1;
		}
		//haccessTableMutex�����˾��У������������ò���������Ҫ��
		AccessTable* accessTable= (AccessTable*)MapViewOfFile(haccessTable, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);//���ļ�ӳ��
		ZeroMemory(accessTable->readNum, sizeof(accessTable->readNum));//��ʼ�����ʵǼǱ�
		accessTable->writeFlag.reset();
		
		vector<STARTUPINFO> startUpInfo;
		vector<PROCESS_INFORMATION> PInfo;
		for(int i=0;i<5;++i) {//����������뷵��ֵ
			WaitForSingleObject(fileSystemMain, INFINITE);//�ļ�ϵͳ��д����
			int uid = login->uid;
			//�����ӽ���
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
	else {//�ӽ��̴��� 
		Sleep(100);//��0.1���ӣ��ȴ�shellע�����handle
		int uid;
		uid = _tstoi(argv[1]);
		FileSystem f(uid);
		wcout << "�û��ѵ�¼,uid: " << argv[1] << endl;
	
		//-----------��ʼͨ��----------
		f.getcmd("help");
		f.outStr += '>';
		strcpy_s((f.file)->ret, 1024 * 10, f.outStr.c_str());
		Sleep(100);//�ȴ�д��
		ReleaseSemaphore(f.shell,1, NULL);//֪ͨshell����ִ��
		WaitForSingleObject(f.disk, INFINITE);//����disk
		while (1) {
			f.outStr.clear();
			cout << "1" << endl;
			f.file->errorFlag = 0;//��ʼ����ǰ�������־Ӧ����0
			string cmd(f.file->cmd);
			cout << cmd << endl;
			f.getcmd(cmd);
			f.outStr += f.getWd();
			f.outStr += '>';
			strcpy_s(f.file->ret, 1024 * 10, f.outStr.c_str());//д���
			cout << f.outStr.c_str();
			Sleep(100);//����ʱ�䣬�ȴ�д��
			if (cmd.size()>=3&&cmd.substr(0, 3) == "cat" && f.file->errorFlag == 0) {//�ɹ����ж�����������Ҫ�ڴ˴�����shell������ͣ��5�룬�Ѿ���ϵͳ����ǰrelease��,����Ҫ������֪ͨ
				cout << '2' << endl;
				WaitForSingleObject(f.disk, INFINITE);//����disk
			}
			//else if(cmd.size() >= 3 && cmd.substr(0, 6) == "format"&&f.file->errorFlag == 0)
			else {
				cout << '3' << endl;
				ReleaseSemaphore(f.shell, 1, NULL);//֪ͨshell���Ի�ȡ�������ִ��
				WaitForSingleObject(f.disk, INFINITE);//����disk
			}
		}
	}
}