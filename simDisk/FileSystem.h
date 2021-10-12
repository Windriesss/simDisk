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
	int curPos;//��ǰ��дָ������λ��
	inode* curInode;//��ǰ����Ŀ¼���ļ���inodeָ��
	char curPath[512];//��ǰ·��
	stack<int> inodeStack;//·��i�ڵ�ջ

public:
	FileSystem();
	//-----------ϵͳ���ú���---------------------
	void init();//û��������̵Ļ���ʼ���������
	void load();//���ش���
	void save();//���������Ϣ����Ҫ�ǰ�λͼ���ȥ

	//-----------��ȡ��Ϣ��������-----------------ע�⣬Ҫ��FILE�򿪵�����²���ʹ��
	inode* getInode(int);//��i�ڵ��ȡi�ڵ�
	DirectoryItem* getDirectorItem(inode*, int);//ȡĿ¼�еĵ�i��Ŀ¼��
	int getParentDirIndex(string);//�ҵ�·���еĸ�Ŀ¼
	int getIndex(string);//�ҵ�·������Ӧ��i���
	int dirFindByName(inode* ,string);//�ҵ��ļ����У�����Ϊname��i�ڵ��

	//------------�޸�ϵͳ��Ϣ��������------------
	int RequestI();//����һ��i�ڵ�
	int RequestD(int*, int);//����n�����ݿ�
	void postInode(inode*);//�Ѹ�inodeд�ش���
	void postDirItem(inode*, inode*);//����һ���µ�Ŀ¼���һ��inode�Ǹ�Ŀ¼���ڶ�������Ŀ¼���ļ���inode
	void dirInit(inode*, string);//���ļ���������µ��ļ���
	
	//-----------���Ը�������-------------------
	void print();//��ӡ������Ϣ��������


	//------------����---------------------------------
	void info();//��ʾϵͳ��Ϣ
	void md(string);//�����ļ���
	void dir(string);//��ʾĿ¼��Ϣ

	//-------------������������------------------------
	vector<string> split(string, const string&);//�ָ��ַ���
};
#endif