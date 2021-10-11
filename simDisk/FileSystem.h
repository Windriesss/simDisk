#pragma once
#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include<Windows.h>
#include"Struct.h"
#include<fstream>
#include<bitset>

class FileSystem {
	//-------------��Ҫ���disk������---------
	SuperBlock S; //1��block��
	bitset<9102U> inodeBMap; //1��block��  1024B
	bitset<102400U> blockBMap; //13��block��  12.5*1024B
	//----------------------------------------
	fstream FILE;
	int curPos;//��ǰ��дָ������λ��
	inode* curInode;//��ǰ����Ŀ¼���ļ���inodeָ��
	char curPath[512];//��ǰ·��
	
public:
	FileSystem();
	void init();
	void load() {};
	int RequestD(int* t, int n);
};




#endif