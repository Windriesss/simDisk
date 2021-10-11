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
	//-------------需要存回disk的数据---------
	SuperBlock S; //1个block存
	bitset<8192> inodeBMap; //1个block存  1024B
	bitset<100*1024> blockBMap; //13个block存  12.5*1024B
	//----------------------------------------
	fstream FILE;
	int curPos;//当前读写指针所在位置
	inode* curInode;//当前工作目录或文件的inode指针
	char curPath[512];//当前路径
	stack<int> inodeStack;//路径i节点栈

public:
	FileSystem();
	void init();
	void load();
	int RequestI();
	int RequestD(int* t, int n);
	void print();//打印磁盘信息，调试用
	void save();
};




#endif