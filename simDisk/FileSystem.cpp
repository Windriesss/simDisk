#include "FileSystem.h"
#include<bitset>
#include <fstream>
#include <cstring>
#include <iostream>
#include <windows.h>

using namespace std;


FileSystem::FileSystem() {
	fstream FILE;
	FILE.open("VirtualDisk", ios::binary | ios::in|ios::out);
	if (!FILE) {
		cout << "δ��������Ӳ�̣���������Ӳ��" << endl;
		init();
		load();
	}
	FILE.close();
	FILE.open("VirtualDisk", ios::binary | ios::in | ios::out);
	
	FILE.write((char*)&S, sizeof(S));//��Ӳ��״̬������
	//0:������  1:inodeλͼ  2:inode������  2050:���ݿ�λͼ  2063:���ݿ�
	//Ĭ������
	curPos=-1;//û�п�ʼ��д
	FILE.seekg(S.inodePos, ios::beg);
	FILE.read((char*)&curInode,sizeof(curInode));//��ǰ����Ŀ¼���ļ���inode
	strcpy_s(curPath,512,curInode->name);//��ǰ·��
	FILE.close();
}

void FileSystem::init() {
	cout << "���ڴ����ļ�ϵͳ ..." << endl;
	fstream FILE("VirtualDisk", ios::binary|ios::out);//����100M���̿ռ�
	if (!FILE) {
		cerr << "�����������ʧ��!" << endl;
		return;
	}
	FILE.seekp(0, ios::beg);//����ռ䣬ȫ������Ϊ0
	for (int i = 0; i < S.blockNum * S.blockSize; ++i) {//����blockNum�����̿�
		FILE.write(0, sizeof(char));
	}
	//------------��ʼ��������------------
	S.diskSize = 100 * 1024 * 1024;//������������λB  100*1024*1024B
	S.blockSize = 1024;//���̿���������λB 1024B
	S.blockNum = S.diskSize / S.blockSize;//���̿����� diskSize/bolckSize 102400��
	S.blockUsedNum = 1 + 1 + 2048 + 13;//��ʹ�õĴ��̿���
	S.inodeNum = 8192;//i�ڵ�����
	S.inodeUsedNum = 1;//��ʹ�õ�i�ڵ���
	S.inodeBMapPos = 1 * S.blockSize;//i�ڵ�λͼ��ַ,���ڼ����ֽڿ�ʼ����1������i�ڵ�λͼ�������ɴ��8192���ļ����ļ���
	S.inodePos = 2 * S.blockSize;//i�ڵ���,�ڸ��ֽڿ�ʼ��i�����
	S.blockBMapPos = 2050 * S.blockSize;//��λͼ��ַ,���ڼ����ֽڿ�ʼ����13(12.5)�����������ݿ�����λͼ  
	S.blockPos = 2063 * S.blockSize;
	//-------------������ʼ��������-------------

	FILE.seekp(0, ios::beg);//�Ƶ���ͷ����䳬������
	FILE.write((char*)&S, sizeof(S));
	
	FILE.seekp(S.inodeBMapPos, ios::beg);//׼����inodeλͼ
	inodeBMap.set(0);//0���������Ŀ¼
	FILE.write((char*)&inodeBMap, sizeof(inodeBMap));//��inodeλͼ

	FILE.seekp(S.blockBMapPos, ios::beg);//׼����blockBnodeλͼ
	for (int i = 0; i < 2063; ++i) {//ǰ���0-2062���Ѿ���ʹ��
		blockBMap.set(i);
	}
	FILE.write((char*)&blockBMap, sizeof(blockBMap));//��blockλͼ
	//---------------��Ŀ¼i�ڵ�--------------
	inode t;
	t.idx=0;//inode���
	t.parentIdx=-1;//��Ŀ¼��inode���
	t.linkNum=1;//ָ���inode���ļ���
	t.uid=0;//�����û�id  0��ʾroot�û�
	t.gid=10000;//������id    10000��ʾroot��
	t.mod=777;//����ģʽ �����˿ɶ���д��ִ��
	strcpy_s(t.name,136,"/");//�ļ���
	t.type=0;//�ļ�����  0��Ŀ¼��1���ļ�
	GetLocalTime(&t.creatTime);//����ʱ��  16B
	t.modiTime=t.creatTime;//���һ���޸�ʱ��
	t.size=0;//�ļ���С ��λB
	t.dataBlock[0] = { -1 };//14�����ݿ�ĵ�ַ����4����һ��ҳ���ļ��������1MB+10KB
	//--------------------------------

	int Rflag = RequestD(t.dataBlock, 10);
	if (Rflag==-1) {//���ݿ鲻�㣬����ʧ��

	}
	FILE.seekp(S.inodePos, ios::beg);
	FILE.write((char*)&t, sizeof(t));//�Ѹ�Ŀ¼i�ڵ�д��ȥ
	//-----��inode�ĳɿ�inode---------
	//t.idx = 0;//inode��� ��ѭ�����
	t.parentIdx = -1;//��Ŀ¼��inode���
	t.linkNum = 0;//ָ���inode���ļ���
	t.uid = -1;//�����û�id  0��ʾroot�û�
	t.gid = -10000;//������id    10000��ʾroot��
	t.mod = 000;//����ģʽ �����˿ɶ���д��ִ��
	strcpy_s(t.name, 136, "");//�ļ���
	t.type = -1;//�ļ�����  0��Ŀ¼��1���ļ���-1Ϊ��
	//&t.creatTime;����ʱ�䲻��
	//t.modiTime = t.creatTime;//���һ���޸�ʱ��
	//t.size = 0;//�ļ���С ��λB
	//t.dataBlock[0] = { 0 };//14�����ݿ�ĵ�ַ����4����һ��ҳ���ļ��������1MB+10KB
	//-------------------------------------
	for (int i = 1; i < S.inodeNum; ++i) {
		t.idx = i;
		FILE.write((char*)&t, sizeof(t));
	}
	return;
}

int FileSystem::RequestD(int* t, int n) {
	int getNum = 0;
	int i;
	for (i = 0; i < S.blockNum&&getNum<n; ++i) {
		if (!blockBMap[i]) {//�ҵ�һλ��λ
			blockBMap.set(i);//������һλ
			t[getNum++] = i;
		}
	}
	if (i == S.blockNum) {//�ڴ治��
		return -1;//�ڴ治�㣬���ش���
	}
	return 0;//��������

}