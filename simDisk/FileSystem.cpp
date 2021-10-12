#include "FileSystem.h"
#include"Struct.h"
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
	FILE.open("VirtualDisk", ios::binary | ios::in | ios::out);
	if (!FILE) {
		cerr << "FileSystem打开虚拟硬盘失败！" << endl;
	}
	load();//读磁盘状态
}

void FileSystem::init() {
	cout << "正在创建文件系统 ..." << endl;
	FILE.open("VirtualDisk", ios::binary|ios::out);//申请100M磁盘空间
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
	t.dataBlock[0] = 0;//14个数据块的地址，后4块是一级页表，文件最大内容1MB+10KB
	//-------------------------------------
	for (int i = 1; i < S.inodeNum; ++i) {
		t.idx = i;
		FILE.write((char*)&t, sizeof(inode));
	}
	FILE.flush();
	return;
}

void FileSystem::load() {
	//FILE.open("VirtualDisk", ios::binary | ios::in|ios::out);//打开磁盘
	if (!FILE) {
		cerr << "load()" << "打开磁盘失败！" << endl;
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
}

int FileSystem::RequestI() {
	for (int i = 1; i < S.inodeNum; ++i) {
		if (!inodeBMap[i]) {
			inodeBMap.set(i);
			save();
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
		cerr << "RequestD()错误！数据块不足！" << endl;
		return -1;//内存不足，返回错误
	}
	save();
	return 0;//正常返回
}

inode* FileSystem::getInode(int idx) {//根据i节点号返回i节点的指针
	//调用这个函数前必须先打开文件
	if (!FILE) {
		cerr << "getInode()调用前没有打开文件！终止！" << endl;
		//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//打开磁盘
		//return NULL;
	}
	if (idx < 0 || idx >= 8192) {
		cerr << "getInode()错误！i结点序号超出范围！输入的i结点号为："<<idx<<endl;
		return NULL;
	}
	inode* ret = new inode();
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
	for (int i = ret.size()-1; i >0; --i) {
		if (ret[i].empty()) ret.pop_back();
		else break;
	}
	return ret;
}

int FileSystem::getParentDirIndex(string path) {
	if (path.empty()) {
		cerr << "getParentDirIndex()错误，路径为空！" << endl;
		return -1;
	}
	vector<string> pathSplit = split(path,"/");
	inode* curInode;
	int curInodeIdx = -1;
	if (pathSplit[0].empty()) {//绝对路径
		curInodeIdx = 0;
		curInode = getInode(curInodeIdx);
	}
	else if (pathSplit[0] == "."|| pathSplit[0] == "..") {
		curInodeIdx = inodeStack.top();//工作目录的最后一个
		curInode = getInode(curInodeIdx);
		if (pathSplit[0] == ".") {
			curInodeIdx = curInode->parentIdx;//该工作目录的上一层
			curInode = getInode(curInodeIdx);
		}
	}
	else {//错误输入
		cerr << "getParentDirIndex()错误，路径输入错误！" << endl;
		return -1;//错误
	}
	for (int i = 1; i < pathSplit.size()-1; ++i) {//留一级不取
		curInodeIdx = dirFindByName(curInode, pathSplit[i]);//取下一级目录的i节点号
		if (curInodeIdx == 0) {
			cout << "getParentDirIndex()错误!";
			cout << curInode->name << "中没有名字为" << pathSplit[i] << "的文件或目录" << endl;
			return -1;
		}
		else curInode = getInode(curInodeIdx);//取i节点
	}
	return curInodeIdx;
}

int FileSystem::getIndex(string path) {
	if (path == "/") {//根目录没有父目录，不适合下面的操作
		return 0;
	}
	int parentInodeIdx=getParentDirIndex(path);//取其父目录的i结点号
	if (parentInodeIdx == -1) {
		cerr << "getDirIndx()错误！终止命令！" << endl;
		return -1;
	}
	inode* parentInode = getInode(parentInodeIdx);//取其父目录
	vector<string> pathSplit = split(path,"/");//取dir名字
	string dirname = pathSplit.back();
	int inodeIdx=dirFindByName(parentInode, dirname);//找出所取文件的i结点号
	return inodeIdx;
}


DirectoryItem* FileSystem::getDirectorItem(inode* dirInode, int idx) {//取文件夹中的第idx项
	if (dirInode->type != '0') {
		cerr << "getDirectorItem()错误，该文件不是文件夹类型" << endl;
		return NULL;
	}
	if (dirInode->size < idx - 1) {
		cerr << "getDirectorItem()错误，索引超出目录项索引" << endl;
		return NULL;
	}
	int blockNum = idx / 7;//在第几个数据块
	int divation = idx % 7;//在块中的第几个
	FILE.seekg(dirInode->dataBlock[blockNum] * S.blockSize + divation * sizeof(DirectoryItem), ios::beg);
	DirectoryItem* ret = new DirectoryItem;
	FILE.read((char*)ret, sizeof(DirectoryItem));
	return ret;
}

int FileSystem::dirFindByName(inode* dirInode,string name) {
	if (!FILE) {
		cerr << "dirFindByName()调用前没有打开文件！终止！" << endl;
		return -1;
	}
	int FileNum = dirInode->size;//该文件夹总共有多少个文件
	DirectoryItem *dirItem = new DirectoryItem();
	for (int i = 0; i <= FileNum / 7; ++i) {//在第几个文件块里,一个文件块可以放7个目录项
		FILE.seekg(dirInode->dataBlock[i] * S.blockSize, ios::beg);
		for (int j = 0; j<7 && FileNum>i * 7 + j; ++j) {//文件块中的第几个，且不能越界访问
			FILE.read((char*)dirItem, sizeof(DirectoryItem));
			if (name.compare(dirItem->name) == 0) {//名字相同，可以返回结果了
				return dirItem->inodeIdx;//返回i节点序号
			}
		}
	}
	if (!FILE) {
		cout << "这里不行了哦！" << endl;
	}
	return -1;//在该文件夹下没有该文件或目录
}

void FileSystem::postInode(inode* i) {
	if (!FILE) {
		cerr << "postInode()调用时未打开文件！终止！" << endl;
		//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//打开磁盘
	}
	FILE.seekp(S.inodePos + i->idx * sizeof(inode), ios::beg);
	FILE.write((char*)i, sizeof(inode));
	FILE.flush();
}

void FileSystem::postDirItem(inode* parentInode, inode* insertInode) {
	//！！！一级间址还没做，判断不完整
	if (parentInode->size > 70) {
		cerr << "dirInit()失败，父目录已满！" << endl;
		return;
	}
	DirectoryItem* newDirItem = new DirectoryItem;//创建一个新的目录项，并且往里面填信息
	newDirItem->inodeIdx = insertInode->idx;
	strcpy_s(newDirItem->name, 136, insertInode->name);
	newDirItem->type = insertInode->type;
	

	int blockNum = parentInode->size / 7;//写在第几块数据块中
	int deviation = parentInode->size % 7;//写在数据块的哪一项中
	FILE.seekp(parentInode->dataBlock[blockNum] * S.blockSize + deviation * sizeof(DirectoryItem), ios::beg);//把指针移到该目录项存放的位置
	FILE.write((char*)newDirItem, sizeof(DirectoryItem));//写入磁盘中
	parentInode->size++;//目录项的数目加1

	GetLocalTime(&(parentInode->modiTime));//更改父目录的修改时间
	postInode(parentInode);//把父目录inode也写回磁盘中，直接同步
	FILE.flush();
	return;
}

void FileSystem::dirInit(inode* parentInode, string name) {
	//！！！没有完整判断目录是否满
	if (!FILE) {
		cerr << "dirInit()调用前没有打开文件！终止！" << endl;
		//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//打开磁盘
		//return;
	}
	if (parentInode->size > 70) {
		cerr << "dirInit()失败，父目录已满！" << endl;
		return;
	}
	int newDirInodeIdx=RequestI();//为该目录申请一个i节点
	inode* newDirInode = getInode(newDirInodeIdx);//把新申请的inode取出来，写其中信息
	//-------------修改新inode的信息---------------
	newDirInode->idx = newDirInodeIdx;//自身的i节点号
	newDirInode->parentIdx=parentInode->idx;//父目录的inode序号
	newDirInode->linkNum=1;//指向该inode的文件数
	newDirInode->uid=1;//所属用户id
	newDirInode->gid=10000;//所属组id
	newDirInode->mod=777;//保护模式
	strcpy_s(newDirInode->name,136,name.c_str());//文件名
	newDirInode->type='0';//文件类型  0表示目录  1表示文件
	GetSystemTime(&(newDirInode->creatTime));//创建时间  16B
	newDirInode->modiTime=newDirInode->creatTime;//最后一次修改时间
	newDirInode->size=0;//文件大小 单位B
	RequestD(newDirInode->dataBlock,10);//14个数据块的地址，后4块是一级页表，文件最大内容1MB+10KB
	//----------------------------------------------
	postInode(newDirInode);//把申请好的inode写回磁盘
	postDirItem(parentInode, newDirInode);//为父目录加目录项
}


void FileSystem::save() {
	if (!FILE) {
		cerr << "save()调用时未打开文件！终止！" << endl;
		//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//打开磁盘
	}
	FILE.seekp(0, ios::beg);//写超级块
	FILE.write((char*)&S, sizeof(S));
	FILE.seekp(S.inodeBMapPos, ios::beg);//写inode位图
	FILE.write((char*)&inodeBMap, sizeof(inodeBMap));
	FILE.seekp(S.blockBMapPos, ios::beg);//写block位图
	FILE.write((char*)&blockBMap, sizeof(blockBMap));
	FILE.flush();
}


void FileSystem::print() {
	S.print();
	curInode->print();
	cout << "当前工作路径：" << curPath << endl;
}