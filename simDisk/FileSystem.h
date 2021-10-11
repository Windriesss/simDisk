#pragma once
#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include<Windows.h>
#include"Struct.h"
#include<fstream>
#include<bitset>

class FileSystem {
	//-------------需要存回disk的数据---------
	SuperBlock S; //1个block存
	bitset<9102U> inodeBMap; //1个block存  1024B
	bitset<102400U> blockBMap; //13个block存  12.5*1024B
	//----------------------------------------
	fstream FILE;
	int curPos;//当前读写指针所在位置
	inode* curInode;//当前工作目录或文件的inode指针
	char curPath[512];//当前路径
	
public:
	FileSystem();
	void init();
	void load() {};
	int RequestD(int* t, int n);
};




#endif