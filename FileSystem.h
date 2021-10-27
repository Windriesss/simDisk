#pragma once
#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include<Windows.h>
#include"Struct.h"
#include<fstream>
#include<bitset>
#include<vector>
#include<string>
#include<stack>
using namespace std;

class FileSystem {
public: //������ԣ�ʵ��ʹ����Ҫ�ر�
	//-------------��Ҫ���disk������---------
	SuperBlock S; //1��block��
	bitset<8192> inodeBMap; //1��block��  1024B
	bitset<100*1024> blockBMap; //13��block��  12.5*1024B
	//----------------------------------------
	fstream FILE;
	vector<inode*> inodeStack;//·��i�ڵ�ջ
	int uid;
	string outStr;

	File* file;
	AccessTable* accessTable;
	HANDLE hFile, shell, disk, haccessTable, mutex;

public:
	FileSystem(int);
	//-----------ϵͳ���ú���---------------------
	void init();//û��������̵Ļ���ʼ���������
	void load();//���ش���
	void save();//���������Ϣ����Ҫ�ǰ�λͼ���ȥ
	void updateBMap();

	//-----------��ȡ��Ϣ��������-----------------ע�⣬Ҫ��FILE�򿪵�����²���ʹ��
	inode* getInode(int);//��i�ڵ��ȡi�ڵ�
	inode* getFileInode(string);//
	DirectoryItem* getDirectorItem(inode*, int);//ȡĿ¼�еĵ�i��Ŀ¼��
	int getParentDirIndex(string);//�ҵ�·���еĸ�Ŀ¼
	int getIndex(string);//�ҵ�·������Ӧ��i����
	int dirFindByName(inode* ,string);//�ҵ��ļ����У�����Ϊname��i�ڵ��a
	string getMod(inode*);//��ȡ�ļ�����ģʽ
	string getWd();//��ȡ����·���ַ���
	string getFileContent(inode*);//��ȡ�ļ���Ϣ
	void updateInodeStack();//���¹���i�ڵ�ջ
	void checkHelp(int idx, bitset<8192>&, bitset<100 * 1024>&);//�ݹ�check
	

	//------------�޸�ϵͳ��Ϣ��������------------
	int RequestI();//����һ��i�ڵ㣬�൱�ڴ������ļ�
	void delI(inode*);//ɾ��i�ڵ㣬ͬʱ�ͷ�����ռ�ռ䣬��ͬ��ɾ���ļ����ļ���
	int RequestD(int*, int);//����n�����ݿ�
	void delD(int);//ɾ���������ݿ�
	void delD(int*, int);//ɾ�����ݿ�����
	void postInode(inode*);//�Ѹ�inodeд�ش���
	void postDirItem(inode*&, inode*&);//����һ���µ�Ŀ¼���һ��inode�Ǹ�Ŀ¼���ڶ�������Ŀ¼���ļ���inode
	void postDirItem(inode*, DirectoryItem*, int);//��inode*�в���һ��Ŀ¼�Ŀ¼��ΪDirItem,���ڵ�i��λ��
	void dirInit(inode*, string);//���ļ���������µ��ļ���
	int  fwriteHelp(inode*, string, bool);//����д���ļ�
	
	//-----------���Ը�������-------------------
	void makeTrouble();//���ҷ���һЩi�ڵ�����ݿ�
	//------------���庯��---------------
	void getcmd(string cmd);

	//------------����---------------------------------
	void format();//��ʽ��Ӳ��
	void info();//��ʾϵͳ��Ϣ
	void md(string);//�����ļ���
	void dir(string);//��ʾĿ¼��Ϣ
	void cd(string);//���Ĺ���Ŀ¼
	void pwd();//��ӡ��ǰ����Ŀ¼
	void rd(string);//ɾ���ļ���
	inode* newfile(string);//�������ļ�
	void fwrite(string, bool);//д�ļ�
	void cat(string);//��ʾ�ļ�����
	void del(string);//ɾ���ļ�
	void copy(string, string);//�����ļ�
	void check();
	void help();//�������
	void chmod(string, int);//�ı��ļ�����Ȩ��

	
	//-------------������������------------------------
	string modInt2String(int);//��ȡ����ģʽ
	vector<string> split(string, const string&);//�ָ��ַ���
	string cmpPath(string);//�Զ���ȫ·��
	string UTF8ToGB(const char* str);//UTF8תGBK
	LPCWSTR stringToLPCWSTR(string orig);//String תLPCWSTR
};

#endif