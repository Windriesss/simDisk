#pragma once
#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include<Windows.h>
#include"Struct.h"
#include<fstream>
#include<bitset>
#include<vector>
#include<stack>
using namespace std;

class FileSystem {
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
	void init();
	void load();
	int RequestI();
	int RequestD(int* t, int n);
	void print();//��ӡ������Ϣ��������
	void save();
};




#endif