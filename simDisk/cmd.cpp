#include"FileSystem.h"
#include<iostream>
#include<string>
#include<iomanip>
using namespace std;

void FileSystem::info() {
	//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//打开磁盘
	if (!FILE) {
		cerr << "info()" << "打开磁盘失败！" << endl;
	}
	int fileNum = 0;//文件总数
	int dirNum = 0;//文件夹总数
	int freeSpace = (S.blockNum - blockBMap.count()) / 1024;//空闲数据块数，单位MB
	int freeInode = S.inodeNum - inodeBMap.count();//空闲i节点个数
	for (int i = 0; i < S.inodeNum; ++i) {
		if (inodeBMap[i]) {
			inode* t = getInode(i);
			if (t->type == '0') dirNum++;
			if (t->type == '1') fileNum++;
		}
	}
	//FILE.close();
	S.print();
	cout << "磁盘剩余空间：" << freeSpace << "MB" << endl;
	cout << "空闲创建inode数：" << freeInode << endl;
	cout << "总目录数：" << dirNum << endl;
	cout << "总文件数：" << fileNum << endl;
}

void FileSystem::md(string path) {
	//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//打开磁盘
	if (!FILE) {
		cerr << "md()" << "打开磁盘失败！" << endl;
	}
	//预处理
	int parentIdx = getParentDirIndex(path);//获取父文件夹的i节点号
	if (parentIdx == -1) {
		cerr << "路径输入错误，终止命令！" << endl;
		return;
	}
	inode* parentInode = getInode(parentIdx);
	string newDirName = split(path, "/").back();
	if (newDirName.empty()) {
		cerr << "不能创建与根目录同名的目录，终止命令！" << endl;
		return;
	}
	if (dirFindByName(parentInode, newDirName) != -1) {
		cerr << "文件夹下已经存在名为" << newDirName << "的文件,终止命令！" << endl;
		return;
	}
	dirInit(parentInode, newDirName);//创建新的文件夹
	FILE.flush();
}

void FileSystem::dir(string path) {
	if (!FILE) {
		cerr << "dir()" << "打开磁盘失败！" << endl;
	}
	int inodeIdx = getIndex(path);//
	inode* dirInode = getInode(inodeIdx);
	if (dirInode->type != '0') {
		cerr << "dir()错误，" << path << "不是文件夹！" << endl;
	}
	cout<<left<<setw(10)<< dirInode->mod << setw(8) << dirInode->uid << setw(8) << dirInode->gid << setw(12) <<
		dirInode->getSize() << setw(24) << dirInode->getModiTime() << setw(12) << "." << endl;//保护模式 所属者 所属组 大小 创建时间 名字 
	for (int i = 0; i < dirInode->size; ++i) {//打印文件夹的目录项
		DirectoryItem* dirItem = getDirectorItem(dirInode, i);
		dirItem->print();
		cout << endl;
	}
	//FILE.close();
	return;
}