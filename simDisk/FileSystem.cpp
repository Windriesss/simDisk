#include "FileSystem.h"
#include<bitset>
#include <fstream>
#include <cstring>
#include <iostream>
#include <windows.h>

using namespace std;

FileSystem::FileSystem() {
	FILE.open("VirtualDisk", ios::binary | ios::in);//试试看有无虚拟硬盘
	if (!FILE) {
		cout << "未发现虚拟硬盘，创建虚拟硬盘" << endl;
		init();
	}
	FILE.close();
	//-------------先把init调试好----------
	load();//读磁盘状态

}

void FileSystem::init() {
	cout << "正在创建文件系统 ..." << endl;
	fstream FILE("VirtualDisk", ios::binary|ios::out);//申请100M磁盘空间
	if (!FILE) {
		cerr << "创建虚拟磁盘失败!" << endl;
		return;
	}
	//------------初始化超级块------------
	S.diskSize = 100 * 1024 * 1024;//磁盘容量，单位B  100*1024*1024B
	S.blockSize = 1024;//磁盘块容量，单位B 1024B
	S.blockNum = S.diskSize / S.blockSize;//磁盘块总数 diskSize/bolckSize 102400块
	S.blockUsedNum = 1 + 1 + 2048 + 13;//已使用的磁盘块数
	S.inodeNum = 8192;//i节点总数
	S.inodeUsedNum = 1;//已使用的i节点数
	S.inodeBMapPos = 1 * S.blockSize;//i节点位图地址,即第几个字节开始，用1块来做i节点位图，即共可存放8192个文件或文件夹
	S.inodePos = 2 * S.blockSize;//i节点区,第个字节开始是i结点区
	S.blockBMapPos = 2050 * S.blockSize;//块位图地址,即第几个字节开始，用13(12.5)个连续的数据块做块位图  
	S.blockPos = 2063 * S.blockSize;
	//-------------结束初始化超级块-------------


	FILE.seekp(0, ios::beg);//申请空间，全部设置为0
	for (int i = 0; i < S.blockNum * S.blockSize; ++i) {//申请blockNum个磁盘块
		FILE.write(" ", sizeof(char));
	}


	FILE.seekp(0, ios::beg);//移到开头，填充超级块区
	FILE.write((char*)&S, sizeof(S));
	
	FILE.seekp(S.inodeBMapPos, ios::beg);//准备填inode位图
	inodeBMap.set(0);//0结点分配给根目录
	FILE.write((char*)&inodeBMap, sizeof(inodeBMap));//填inode位图

	FILE.seekp(S.blockBMapPos, ios::beg);//准备填blockBnode位图
	for (int i = 0; i < 2063; ++i) {//前面的0-2062块已经被使用
		blockBMap.set(i);
	}
	FILE.write((char*)&blockBMap, sizeof(blockBMap));//填block位图
	//---------------根目录i节点--------------
	inode t;
	t.idx=0;//inode序号
	t.parentIdx=-1;//父目录的inode序号
	t.linkNum=1;//指向该inode的文件数
	t.uid=0;//所属用户id  0表示root用户
	t.gid=10000;//所属组id    10000表示root组
	t.mod=777;//保护模式 所有人可读可写可执行
	strcpy_s(t.name,136,"/");//文件名
	t.type='0';//文件类型  0是目录，1是文件
	GetLocalTime(&t.creatTime);//创建时间  16B
	t.modiTime=t.creatTime;//最后一次修改时间
	t.size=0;//文件大小 单位B
	t.dataBlock[0] = { -1 };//14个数据块的地址，后4块是一级页表，文件最大内容1MB+10KB
	//--------------------------------

	int Rflag = RequestD(t.dataBlock, 10);
	if (Rflag==-1) {//数据块不足，申请失败

	}
	FILE.seekp(S.inodePos, ios::beg);
	FILE.write((char*)&t, sizeof(inode));//把根目录i节点写回去
	//-----把inode改成空inode---------
	//t.idx = 0;//inode序号 在循环里改
	t.parentIdx = -1;//父目录的inode序号
	t.linkNum = 0;//指向该inode的文件数
	t.uid = -1;//所属用户id  0表示root用户
	t.gid = -10000;//所属组id    10000表示root组
	t.mod = 000;//保护模式 所有人可读可写可执行
	strcpy_s(t.name, 136, "");//文件名
	t.type = 'x';//文件类型  0是目录，1是文件，x为空
	//&t.creatTime;两个时间不改
	//t.modiTime = t.creatTime;//最后一次修改时间
	//t.size = 0;//文件大小 单位B
	//t.dataBlock[0] = { 0 };//14个数据块的地址，后4块是一级页表，文件最大内容1MB+10KB
	//-------------------------------------
	for (int i = 1; i < S.inodeNum; ++i) {
		t.idx = i;
		FILE.write((char*)&t, sizeof(inode));
	}
	FILE.close();
	return;
}

void FileSystem::load() {//读不到正确的东西
	FILE.open("VirtualDisk", ios::binary | ios::in);//打开磁盘
	if (!FILE) {
		cerr << "打开磁盘失败！";
	}
	FILE.seekg(0, ios::beg);//读超级块
	FILE.read((char*)&S, sizeof(S));
	FILE.seekg(S.inodeBMapPos, ios::beg);//读inode位图
	FILE.read((char*)&inodeBMap, sizeof(inodeBMap));
	FILE.seekg(S.blockBMapPos, ios::beg);//读block位图
	FILE.read((char*)&blockBMap, sizeof(blockBMap));
	curInode = new inode;
	FILE.seekg(S.inodePos, ios::beg);//读根目录i结点
	FILE.read((char*)curInode, sizeof(inode));//当前工作目录或文件的inode
	curPos = -1;//没有开始读写
	strcpy_s(curPath, 512, curInode->name);//当前路径
	inodeStack.push(0);//把根目录的i结点号加入到路径i节点栈中
	FILE.close();
}

int FileSystem::RequestI() {
	for (int i = 1; i < S.inodeNum; ++i) {
		if (!inodeBMap[i]) {
			inodeBMap.set(i);
			return i;
		}
	}
	return -1;//-1表示没有i节点了
}

int FileSystem::RequestD(int* t, int n) {
	int getNum = 0;
	int i;
	for (i = 0; i < S.blockNum&&getNum<n; ++i) {
		if (!blockBMap[i]) {//找到一位空位
			blockBMap.set(i);//申请这一位
			t[getNum++] = i;
		}
	}
	if (i == S.blockNum) {//内存不够
		return -1;//内存不足，返回错误
	}
	return 0;//正常返回
}

inode* FileSystem::getInode(int idx) {//根据i节点号返回i节点的指针
	if (idx < 0 || idx >= 8192) {
		cerr << "i结点序号超出范围!" << endl;
		return NULL;
	}
	inode* ret = new inode();
	FILE.open("VirtualDisk", ios::binary | ios::in);//打开磁盘
	if (!FILE) {
		cerr << "打开磁盘失败！";
	}
	FILE.seekg(S.inodePos+idx*sizeof(inode), ios::beg);//移到对应的i节点的位置
	FILE.read((char*)ret, sizeof(inode));//读入
	return ret;
}


vector<string> FileSystem::split(string str, const string& t) {
	vector<string> ret;
	if (str.length() == 0) return ret;
	str += t;//在最后加一个分隔符，便于分割
	while (str.length()) {
		int nextPos = str.find_first_of(t);//找到第一个分隔符出现的位置
		string sub = str.substr(0, nextPos);//把最前面的一个子串分割出来
		ret.push_back(sub);
		str = str.substr(nextPos + 1, str.size() - (nextPos + 1));//在母串中把子串截去
	}
	return ret;
}


void FileSystem::save() {
	FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//打开磁盘
	if (!FILE) {
		cerr << "打开磁盘失败！";
	}
	FILE.seekp(0, ios::beg);//写超级块
	FILE.write((char*)&S, sizeof(S));
	FILE.seekp(S.inodeBMapPos, ios::beg);//写inode位图
	FILE.write((char*)&inodeBMap, sizeof(inodeBMap));
	FILE.seekp(S.blockBMapPos, ios::beg);//写block位图
	FILE.write((char*)&blockBMap, sizeof(blockBMap));
	FILE.close();
}


void FileSystem::print() {
	S.print();
	curInode->print();
	cout << "当前工作路径：" << curPath << endl;
}