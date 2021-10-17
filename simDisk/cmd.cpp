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
	cout << "\t磁盘剩余空间:\t\t" << freeSpace << "MB" << endl;
	cout << "\t空闲创建inode数:\t" << freeInode << endl;
	cout << "\t总目录数:\t\t" << dirNum << endl;
	cout << "\t总文件数:\t\t" << fileNum << endl;
}

void FileSystem::md(string path) {
	path = cmpPath(path);
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
	path = cmpPath(path);
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
	path = cmpPath(path);//自动补齐路径
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
	path = cmpPath(path);//自动补齐路径
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

inode* FileSystem::newfile(string path) {
	path = cmpPath(path);//自动补齐路径
	int parentInodeIdx=getParentDirIndex(path);//找到路径中的父目录
	inode* parentInode = getInode(parentInodeIdx);
	if (parentInodeIdx < 0) {
		cerr << "newfile()路径错误！终止命令！" << endl;
		return NULL;
	}
	vector<string> pathSplit=split(path,"/");
	string fileName = pathSplit.back();
	int findNameFlag=dirFindByName(parentInode, fileName);
	if (findNameFlag != -1) {
		cout << "该目录下已经存在名称为" << fileName << "的文件或目录！终止命令!" << endl;
		return NULL;
	}


	//申请i节点
	int newInodeIdx = RequestI();
	inode* newInode = getInode(newInodeIdx);
	//给这个i节点申请数据块
	int flag=RequestD(newInode->dataBlock, 10);//申请10个硬盘块
	//如果失败，报错，释放i节点
	if (flag < 0) {
		delI(newInode);
	}
	//i节点和数据块都申请成功，修改i节点信息
	newInode->idx=newInodeIdx;//inode序号
	newInode->parentIdx=parentInodeIdx;//父目录的inode序号
	newInode->linkNum=1;//指向该inode的文件数
	newInode->uid=0;//所属用户id
	newInode->gid=10000;//所属组id
	newInode->mod=777;//保护模式
	strcpy_s(newInode->name,136,fileName.c_str());//文件名
	newInode->type='1';//文件类型  0表示目录  1表示文件
	GetLocalTime(&(newInode->creatTime));//创建时间  16B
	newInode->modiTime=newInode->creatTime;//最后一次修改时间
	newInode->size=0;//文件大小 单位B
	//数据块已经申请好了,不需要额外处理

	//回存i节点
	postInode(newInode);
	//回存目录项
	postDirItem(parentInode, newInode);
	return newInode;
}

void FileSystem::fwrite(string path, bool appFlag = 1) {
	path = cmpPath(path);//自动补齐路径
	//提取路径所指i节点
	inode* fileInode = getInode(getIndex(path));
	if (fileInode == NULL) {
		cerr << "fwrite()路径查找错误！终止！" << endl;
		return;
	}
	//判断inode是否是文件类型
	if (fileInode->type != '1') {
		cerr << "该路径所指类型不是文件类型！终止！" << endl;
		return;
	}
	if (appFlag) {//以追加模式写文件，先把原文件的内容打印出来
		cout << "原文件内容：" <<endl<<getFileContent(fileInode) << endl;//显示文件内容
		cout << "请输入追加的文件内容：(输入回车+(ctrl+z)结束)" << endl;
	}
	else {
		cout << "请输入写入文件内容：(输入回车+(ctrl+z)结束)" << endl;
	}
	string content;
	char ch;
	while ((ch=getchar())!=EOF) {
		content += ch;
	}
	content.erase(content.size() - 1);//擦去最后一个换行符
	if (fwriteHelp(fileInode, content, appFlag) < 0) {
		cerr << "保存文件错误!" << endl;
		return;
	}
	else {
		cout << "保存成功！" << endl;
	}
}

void FileSystem::cat(string path) {
	path = cmpPath(path);//自动补齐路径
	inode* fileInode = getFileInode(path);
	cout << getFileContent(fileInode) << endl;
	return;
}

void FileSystem::del(string path) {
	path = cmpPath(path);//自动补齐路径
	inode* fileInode = getFileInode(path);//这一步可以保证取出来的是文件
	if (fileInode == NULL) {
		cerr << "路径有误！del命令终止！" << endl;
		return;
	}
	delI(fileInode);
	return;
}

void FileSystem::copy(string src, string dst) {//复制文件
	bool srcFlag = 0, dstFlag = 0;//是否在主机磁盘上，是的话置1
	if (src.size() >= 6 && src.substr(0, 6) == "<host>") {
		srcFlag = 1;
	}
	if (dst.size() >= 6 && dst.substr(0, 6) == "<host>") {
		dstFlag = 1;
	}
	string contents;
	//取出文件中的内容
	if (srcFlag) {//源文件在主机上
		fstream srcFile(src.c_str() + 6,ios::binary|ios::in);//打开文件
		if (!srcFile) {
			cerr << "srcFile打开失败！终止！" << endl;
			return;
		}

		srcFile.seekg(0, std::ios::end);
		contents.resize(srcFile.tellg());//扩展contens大小
		srcFile.seekg(0, std::ios::beg);
		srcFile.read(&contents[0], contents.size());//读入文件信息
		srcFile.close();
	}
	else {//源文件在simdisk上
		contents=getFileContent(getFileInode(src));//直接获取
	}
	//写入文件内容
	if (dstFlag) {//目标文件在主机上
		fstream dstFile(dst.c_str() + 6, ios::binary | ios::out);
		if (!dstFile) {
			cerr << "dstFile打开失败！终止！" << endl;
			return;
		}
		dstFile.seekp(0, ios::beg);
		dstFile.write(&contents[0], contents.size());
		return;
	}
	else {//目标文件在simdisk上
		inode* newInode=newfile(dst);
		if (newInode == NULL) {
			cerr << "创建文件失败！终止！" << endl;
			return;
		}
		fwriteHelp(newInode, contents, 0);
	}
	return;
}

void FileSystem::help()
{
	cout <<"\n\t┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓" << endl;
	cout <<  "\t┃ 1. info                      显示系统信息       ┃" << endl;
	cout <<  "\t┃ 2. cd        path            改变目录           ┃" << endl;
	cout <<  "\t┃ 3. dir       path            显示目录           ┃" << endl;
	cout <<  "\t┃ 4. md        path            创建目录           ┃" << endl;
	cout <<  "\t┃ 5. rd        path            删除目录           ┃" << endl;
	cout <<  "\t┃ 6. newfile   path            建立文件           ┃" << endl;
	cout <<  "\t┃ 7. cat       path            打开文件           ┃" << endl;
	cout <<  "\t┃ 8. copy      src dst         拷贝文件           ┃" << endl;
	cout <<  "\t┃ 9. del       path            删除文件           ┃" << endl;
	cout <<  "\t┃ 10.check                     检测并恢复文件系统 ┃" << endl;
	cout <<  "\t┃ 11.format                    格式化磁盘         ┃" << endl;
	cout <<  "\t┃ 12.fwrite    path appFlag    写文件             ┃" << endl;
	cout <<  "\t┃ 13.ctrl+z                    退出文件系统       ┃" << endl;
	cout <<  "\t┃ 14.help                      显示帮助           ┃" << endl;
	cout <<  "\t┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛" << endl;
}