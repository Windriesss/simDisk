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
public: //方便测试，实际使用需要关闭
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
	//-----------系统设置函数---------------------
	void init();//没有虚拟磁盘的话初始化虚拟磁盘
	void load();//加载磁盘
	void save();//保存磁盘信息，主要是把位图存回去

	//-----------获取信息辅助函数-----------------注意，要在FILE打开的情况下才能使用
	inode* getInode(int);//用i节点号取i节点
	DirectoryItem* getDirectorItem(inode*, int);//取目录中的第i个目录项
	int getParentDirIndex(string);//找到路径中的父目录
	int getIndex(string);//找到路径所对应的i结点
	int dirFindByName(inode* ,string);//找到文件夹中，名字为name的i节点号

	//------------修改系统信息辅助函数------------
	int RequestI();//申请一个i节点
	int RequestD(int*, int);//申请n块数据块
	void postInode(inode*);//把该inode写回磁盘
	void postDirItem(inode*, inode*);//插入一个新的目录项，第一个inode是父目录，第二个是子目录或文件的inode
	void dirInit(inode*, string);//在文件夹下添加新的文件夹
	
	//-----------调试辅助函数-------------------
	void print();//打印磁盘信息，调试用


	//------------命令---------------------------------
	void info();//显示系统信息
	void md(string);//创建文件夹
	void dir(string);//显示目录信息

	//-------------其他辅助函数------------------------
	vector<string> split(string, const string&);//分割字符串
};
#endif