#include"FileSystem.h"
#include<iostream>
#include<string>
#include<iomanip>
using namespace std;

void FileSystem::format() {//格式化硬盘
	//！！！登录检验
	cout << "确定初始化磁盘吗？（Y/N）" << endl;
	char ch;
	cin >> ch;
	if (ch == 'Y' || ch == 'y') {
		cout << "正在初始化磁盘，请稍等......" << endl;
		init();
		load();
	}
	else if (ch == 'N' || ch == 'n') {
		return;
	}
	else {
		cout << "输入错误！format()命令终止！" << endl;
	}
}

void FileSystem::info() {
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
	S.print();
	cout << "磁盘剩余空间：" << freeSpace << "MB" << endl;
	cout << "空闲创建inode数：" << freeInode << endl;
	cout << "总目录数：" << dirNum << endl;
	cout << "总文件数：" << fileNum << endl;
}

void FileSystem::md(string path) {
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
	cout<<left<<setw(14)<< getMod(dirInode) << setw(5) << dirInode->uid //打印自己的信息 
		<< setw(8) << dirInode->gid << setw(10) 
		<<dirInode->getSize() << setw(24) << dirInode->getModiTime() 
		<< setw(12) << "." << endl;//保护模式 所属 所属组 大小 创建时间 名字 
	inode* subInode;
	for (int i = 0; i < dirInode->size; ++i) {//打印文件夹的目录项
		DirectoryItem* dirItem = getDirectorItem(dirInode, i);
		subInode = getInode(dirItem->inodeIdx);
		cout << left << setw(14) << getMod(subInode) << setw(5) << subInode->uid << setw(8)
			<< subInode->gid << setw(10) 
			<<subInode->getSize() << setw(24) << subInode->getModiTime() 
			<< setw(12) << subInode->name << endl;//保护模式 所属者 所属组 大小 创建时间 名字 
	}
	return;
}

void FileSystem::cd(string path) {
	if (getIndex(path) == -1) {
		cerr << "路径不存在！终止命令！" << endl;
		return;
	}
	updateInodeStack();//更新InodeStack中的信息
	vector<inode*> newStack = inodeStack;
	vector<string> pathSplit = split(path, "/");
	inode* curInode;
	int curInodeIdx = -1;
	if (pathSplit[0].empty()) {//绝对路径,从根目录开始
		while (newStack.size() > 1) newStack.pop_back();//弹出，直到i节点栈中只剩下根目录
		curInodeIdx = 0;
		curInode = newStack[0];
	}
	else if (pathSplit[0] == "." ) {//从当前目录开始
		curInodeIdx = newStack.back()->idx;//工作目录的最后一个
		curInode = newStack.back();
	}
	else if (pathSplit[0] == "..") {
		if (newStack.size() == 1) {
			cerr << "已经是根目录，没有上一级目录！终止！" << endl;
			return;
		}
		newStack.pop_back();
		curInodeIdx = newStack.back()->idx;//工作目录的最后一个
		curInode = newStack.back();
	}
	else {//错误输入
		cerr << "cd()错误，路径输入错误！" << endl;
		return;//错误
	}
	for (int i = 1; i < pathSplit.size(); ++i) {//
		curInodeIdx = dirFindByName(curInode, pathSplit[i]);//取下一级目录的i节点号
		if (curInodeIdx == -1) {
			cout << "cd()错误!";
			cout << curInode->name << "中没有名字为" << pathSplit[i] << "的文件或目录" << endl;
			return;
		}
		else {
			curInode = getInode(curInodeIdx);//取i节点
			if (curInode->type != '0') {
				cout << "路径中的" << curInode->name << "不是文件夹！终止cd！" << endl;
				return;
			}
			newStack.push_back(curInode);//
		}
	}
	inodeStack = newStack;
	//pwd();
	return;
}

void FileSystem::pwd() {
	updateInodeStack();
	cout << getWd() << endl;
}

void FileSystem::rd(string path) {
	int dirInodeIdx=getIndex(path);
	if (dirInodeIdx == -1) {
		cerr <<'"'<<path<<'"' << "不存在！rd()调用终止！" << endl;
		return;
	}
	if (dirInodeIdx == 0) {
		cerr << "根目录不能删除！rd()调用终止！" << endl;
		return;
	}
	inode* dirInode = getInode(dirInodeIdx);
	if (dirInode->size == 0) {//是个空文件夹
		delI(dirInode);//直接把这个文件释放就行
	}
	else {
		cout << path << "非空，确定要删除吗？(Y/N)" << endl;
		char ch;
		cin >> ch;
		if (ch == 'N' || ch == 'n') {
			cout << "放弃删除!" << endl;
			return;
		}
		else if (ch == 'Y' || ch == 'y') {
			delI(dirInode);//级联删除该目录
			return;
		}
		else {
			cerr << "输入错误，rd终止，请重新输入命令！" << endl;
			return;
		}
	}
	return;
}