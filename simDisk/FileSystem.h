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
	vector<inode*> inodeStack;//路径i节点栈

public:
	FileSystem();
	//-----------系统设置函数---------------------
	void init();//没有虚拟磁盘的话初始化虚拟磁盘
	void load();//加载磁盘
	void save();//保存磁盘信息，主要是把位图存回去

	//-----------获取信息辅助函数-----------------注意，要在FILE打开的情况下才能使用
	inode* getInode(int);//用i节点号取i节点
	inode* getFileInode(string);//
	DirectoryItem* getDirectorItem(inode*, int);//取目录中的第i个目录项
	int getParentDirIndex(string);//找到路径中的父目录
	int getIndex(string);//找到路径所对应的i结点号
	int dirFindByName(inode* ,string);//找到文件夹中，名字为name的i节点号a
	string getMod(inode*);//获取文件保护模式
	string getWd();//获取工作路径字符串
	string getFileContent(inode*);//获取文件信息
	void updateInodeStack();//更新工作i节点栈
	void checkHelp(int idx, bitset<8192>&, bitset<100 * 1024>&);//递归check
	

	//------------修改系统信息辅助函数------------
	int RequestI();//申请一个i节点，相当于创建新文件
	void delI(inode*);//删除i节点，同时释放其所占空间，等同于删除文件或文件夹
	int RequestD(int*, int);//申请n块数据块
	void delD(int);//删除单个数据块
	void delD(int*, int);//删除数据块数组
	void postInode(inode*);//把该inode写回磁盘
	void postDirItem(inode*, inode*);//插入一个新的目录项，第一个inode是父目录，第二个是子目录或文件的inode
	void postDirItem(inode*, DirectoryItem*, int);//在inode*中插入一个目录项，目录项为DirItem,插在第i个位置
	void dirInit(inode*, string);//在文件夹下添加新的文件夹
	int  fwriteHelp(inode*, string, bool);//帮助写入文件
	
	//-----------调试辅助函数-------------------
	void makeTrouble();//胡乱分配一些i节点和数据块
	//------------主体函数---------------
	void getcmd(string cmd);

	//------------命令---------------------------------
	void format();//格式化硬盘
	void info();//显示系统信息
	void md(string);//创建文件夹
	void dir(string);//显示目录信息
	void cd(string);//更改工作目录
	void pwd();//打印当前工作目录
	void rd(string);//删除文件夹
	inode* newfile(string);//创建新文件
	void fwrite(string, bool);//写文件
	void cat(string);//显示文件内容
	void del(string);//删除文件
	void copy(string, string);//复制文件
	void check();
	void help();//命令帮助

	
	//-------------其他辅助函数------------------------
	string modInt2String(int);
	vector<string> split(string, const string&);//分割字符串
	string cmpPath(string);//自动补全路径
	string UTF8ToGB(const char* str);//UTF8转GBK
};

#endif