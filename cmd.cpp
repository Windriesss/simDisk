#include"FileSystem.h"
#include<iostream>
#include<string>
#include<iomanip>
using namespace std;

void FileSystem::format() {//格式化硬盘
	//！！！登录检验
	outStr+= "确定初始化磁盘吗？（Y/N）\n";
	strcpy_s(file->ret, 1024 * 10, outStr.c_str());//写结果
	outStr.clear();
	Sleep(100);//停一会，等待结果写入
	ReleaseSemaphore(shell, 1, NULL);//通知shell可以获取命令继续执行
	WaitForSingleObject(disk, INFINITE);//阻塞disk
	string content(file->cmd);
	if (uid != 0) {//不是root用户
		outStr += "用户权限不足！\n";
		return;
	}
	if (content == "Y" || content == "y") {
		init();
		load();
		outStr += "硬盘初始化完毕！\n";
	}
	else if (content == "N" || content == "n") {
		outStr += "放弃初始化\n";
		return;
	}
	else {
		outStr +=  "输入错误！format()命令终止！\n" ;
	}
}

void FileSystem::info() {
	if (!FILE) {
		outStr += "info()";
		outStr += "打开磁盘失败！\n";
		return;
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
	outStr += "\t磁盘容量:\t\t";
	outStr += to_string(S.diskSize / 1024 / 1024);
	outStr += "MB\n";
	outStr += "\t磁盘块总数:\t\t";
	outStr += to_string(S.blockNum);
	outStr += "块\n";
	outStr += "\ti节点总数:\t\t";
	outStr += to_string(S.inodeNum);
	outStr += '\n';
	outStr += "\ti节点位图起始地址:\t";
	outStr += to_string(S.inodeBMapPos / 1024);
	outStr += '\n';
	outStr += "\ti节点区起始地址:\t";
	outStr += to_string(S.inodePos / 1024);
	outStr += '\n';
	outStr += "\t块位图起始地址:\t\t";
	outStr += to_string(S.blockBMapPos / 1024);
	outStr += '\n';
	outStr += "\t数据块起始地址:\t\t";
	outStr += to_string(S.blockPos / 1024);
	outStr += '\n';
	outStr += "\t磁盘剩余空间:\t\t";
	outStr += to_string(freeSpace);
	outStr +="MB\n";
	outStr += "\t空闲inode数:\t\t";
	outStr += to_string(freeInode);
	outStr += '\n';
	outStr += "\t总目录数:\t\t";
	outStr += to_string(dirNum);
	outStr += '\n';
	outStr += "\t总文件数:\t\t";
	outStr += to_string(fileNum);
	outStr += '\n';
}

void FileSystem::md(string path) {
	path = cmpPath(path);
	if (!FILE) {
		outStr += "md()";
		outStr += "打开磁盘失败！\n";
	}
	//预处理
	int parentIdx = getParentDirIndex(path);//获取父文件夹的i节点号
	if (parentIdx == -1) {
		outStr += "路径输入错误，终止命令！\n";
		return;
	}
	inode* parentInode = getInode(parentIdx);
	string newDirName = split(path, "/").back();
	if (newDirName.empty()) {
		outStr += "不能创建与根目录同名的目录，终止命令！\n";;
		return;
	}
	if (dirFindByName(parentInode, newDirName) != -1) {
		outStr += "文件夹下已经存在名为";
		outStr += newDirName;
		outStr += "的文件,终止命令！\n";
		return;
	}
	dirInit(parentInode, newDirName);//创建新的文件夹
	FILE.flush();
}

void FileSystem::dir(string path) {
	path = cmpPath(path);
	if (!FILE) {
		outStr += "dir()";
		outStr += "打开磁盘失败！\n";
		return;
	}
	int inodeIdx = getIndex(path);
	if (inodeIdx < 0) {
		outStr += "路径错误！\n";
		return;
	}
	inode* dirInode = getInode(inodeIdx);
	if (dirInode->type != '0') {
		outStr += "dir()错误，";
		outStr += path;
		outStr += "不是文件夹！\n";
		return;
	}
	outStr += "\t" + getMod(dirInode) + '\t';
	outStr += to_string(dirInode->uid);
	outStr += '\t';
	outStr += to_string(dirInode->gid);
	outStr += '\t';
	outStr += to_string(dirInode->getSize());
	outStr += '\t';
	outStr += dirInode->getModiTime();
	outStr += '\t';
	outStr += '.';
	outStr += '\n';//保护模式 所属者 所属组 大小 创建时间 名字 

	inode* subInode;
	for (int i = 0; i < dirInode->size; ++i) {//打印文件夹的目录项
		DirectoryItem* dirItem = getDirectorItem(dirInode, i);
		subInode = getInode(dirItem->inodeIdx);
		outStr += "\t" + getMod(subInode) + '\t';
		outStr += to_string(subInode->uid);
		outStr += '\t';
		outStr += to_string(subInode->gid);
		outStr += '\t';
		outStr += to_string(subInode->getSize());
		outStr += '\t';
		outStr += subInode->getModiTime();
		outStr += '\t';
		outStr += subInode->name;
		outStr += '\n';//保护模式 所属者 所属组 大小 创建时间 名字 
	}
	return;
}

void FileSystem::cd(string path) {
	path = cmpPath(path);//自动补齐路径
	if (getIndex(path) == -1) {
		outStr +=  "路径不存在！终止命令！\n";
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
			outStr +=  "已经是根目录，没有上一级目录！终止！\n";
			return;
		}
		newStack.pop_back();
		curInodeIdx = newStack.back()->idx;//工作目录的最后一个
		curInode = newStack.back();
	}
	else {//错误输入
		outStr += "cd()错误，路径输入错误！\n";
		return;//错误
	}
	for (int i = 1; i < pathSplit.size(); ++i) {//
		curInodeIdx = dirFindByName(curInode, pathSplit[i]);//取下一级目录的i节点号
		if (curInodeIdx == -1) {
			outStr +=  "cd()错误!";
			outStr += curInode->name;
			outStr += "中没有名字为";
			outStr += pathSplit[i];
			outStr += "的文件或目录\n";
			return;
		}
		else {
			curInode = getInode(curInodeIdx);//取i节点
			if (curInode->type != '0') {
				outStr += "路径中的";
				outStr += curInode->name;
				outStr += "不是文件夹！终止cd！\n";
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
	outStr += getWd();
	outStr += '\n';
}

void FileSystem::rd(string path) {
	path = cmpPath(path);//自动补齐路径
	int dirInodeIdx=getIndex(path);
	if (dirInodeIdx == -1) {
		outStr += '"';
		outStr += path;
		outStr += '"';
		outStr += "不存在！rd()调用终止！\n";
		return;
	}
	if (dirInodeIdx == 0) {
		outStr += "根目录不能删除！rd()调用终止！\n";
		return;
	}
	inode* dirInode = getInode(dirInodeIdx);
	if (dirInode->size == 0) {//是个空文件夹
		delI(dirInode);//直接把这个文件释放就行
	}
	else {

		outStr += path;
		outStr += "非空，确定要删除吗？(Y/N)\n";
		strcpy_s(file->ret, 1024 * 10, outStr.c_str());//写结果
		file->errorFlag = 1;
		outStr.clear();
		Sleep(100);//停一会，等待结果写入
		ReleaseSemaphore(shell, 1, NULL);//通知shell可以获取命令继续执行
		WaitForSingleObject(disk, INFINITE);//阻塞disk
		string content(file->cmd);
		if (content == "N" || content == "n") {
			outStr +=  "放弃删除!\n";
			return;
		}
		else if (content == "Y" || content == "y") {
			delI(dirInode);//级联删除该目录
			return;
		}
		else {
			outStr += "输入错误，rd终止，请重新输入命令！\n";
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
		outStr +=  "newfile()路径错误！终止命令！\n";
		return NULL;
	}
	vector<string> pathSplit=split(path,"/");
	string fileName = pathSplit.back();
	int findNameFlag=dirFindByName(parentInode, fileName);
	if (findNameFlag != -1) {
		outStr += "该目录下已经存在名称为";
		outStr += fileName;
		outStr += "的文件或目录！终止命令!\n";
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
	newInode->uid=uid;//所属用户id
	newInode->gid=10000+uid;//所属组id
	newInode->mod=770;//保护模式
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
	postInode(parentInode);
	return newInode;
}

void FileSystem::fwrite(string path, bool appFlag = 1) {
	path = cmpPath(path);//自动补齐路径
	//提取路径所指i节点
	inode* fileInode = getInode(getIndex(path));
	if (fileInode == NULL) {
		outStr += "fwrite()路径查找错误！终止！\n";
		file->errorFlag = -1;
		return;
	}
	//判断inode是否是文件类型
	if (fileInode->type != '1') {
		outStr += "该路径所指类型不是文件类型！终止！\n";
		file->errorFlag = -1;
		return;
	}
	string mod = getMod(fileInode);//判断保护模式
	if (fileInode->uid != uid) {//文件不属于该操作者
		if (mod[8] != 'w') {
			outStr += "没有写入权限\n";
			file->errorFlag = -1;
			return;
		}
	}
	else {//文件属于该操作者
		if (mod[2] != 'w') {
			outStr += "没有写入权限\n";
			file->errorFlag = -1;
			return;
		}
	}
	//-------------查看是否能写-----------
	//申请访问登记表
	WaitForSingleObject(mutex, INFINITE);
	//查看是否有人在写
	if (accessTable->writeFlag[fileInode->idx]) {//有用户正在写
		outStr += "该文件正在被写入，请稍后再尝试\n";
		file->errorFlag = -1;
		ReleaseSemaphore(mutex, 1, NULL);//释放访问表
		return;
	}
	//查看是否有人在读
	if (accessTable->readNum[fileInode->idx]) {//有用户正在读
		outStr += "该文件正在被读取，请稍后再尝试\n";
		file->errorFlag = -1;
		ReleaseSemaphore(mutex, 1, NULL);//释放访问表
		return;
	}
	//没有人在写或者写,则写标志置1
	accessTable->writeFlag.set(fileInode->idx);
	//释放访问登记表
	ReleaseSemaphore(mutex, 1, NULL);
	//写数据
	//-------------------
	if (appFlag) {//以追加模式写文件，先把原文件的内容打印出来
		outStr += "原文件内容：\n";
		outStr += getFileContent(fileInode);
		outStr += '\n';//显示文件内容
		outStr += "请输入追加的文件内容：(输入回车+(ctrl+z)结束)\n";
	}
	else {
		outStr += "请输入写入文件内容：(输入回车+(ctrl+z)结束)\n";
	}
	strcpy_s(file->ret, 1024 * 10, outStr.c_str());//写结果
	outStr.clear();
	Sleep(100);//停一会，等待结果写入
	ReleaseSemaphore(shell, 1, NULL);//通知shell可以获取命令继续执行
	WaitForSingleObject(disk, INFINITE);//阻塞disk
	string content(file->cmd);
	if (fwriteHelp(fileInode, content, appFlag) < 0) {
		outStr += "保存文件错误!\n";
		file->errorFlag=-1;
	}
	else {
		outStr += "保存成功！\n";
	}
	//写完了，退出时要把写标志请0
	WaitForSingleObject(mutex, INFINITE);
	accessTable->writeFlag.reset(fileInode->idx);
	ReleaseSemaphore(mutex, 1, NULL);
}

void FileSystem::cat(string path) {
	path = cmpPath(path);//自动补齐路径
	inode* fileInode = getFileInode(path);
	if (fileInode == NULL) {
		outStr += "路径错误！\n";
		file->errorFlag = -1;
		return;
	}
	string mod = getMod(fileInode);
	if (fileInode->uid != uid) {//文件不属于该操作者
		if (mod[7] != 'r') {
			outStr += "没有读取权限\n";
			file->errorFlag = -1;
			return;
		}
	}
	else {//文件属于该操作者
		if (mod[1] != 'r') {
			outStr += "没有读取权限\n";
			file->errorFlag = -1;
			return;
		}
	}
	//------------能运行到这里说明有权限读
	//申请访问登记表
	WaitForSingleObject(mutex, INFINITE);
	//查看是否有人在写
	if (accessTable->writeFlag[fileInode->idx]) {//有用户正在写
		outStr += "该文件正在被写入，请稍后再尝试\n";
		file->errorFlag = -1;
		ReleaseSemaphore(mutex, 1, NULL);//释放访问表
		return;
	}
	//没有人在写,则读者数加1
	accessTable->readNum[fileInode->idx] += 1;
	//释放访问登记表
	ReleaseSemaphore(mutex, 1, NULL);
	//读数据
	outStr +=  getFileContent(fileInode);
	outStr += '\n';
	//读完了，直接把数据写进ret中
	strcpy_s(file->ret, 1024 * 10, outStr.c_str());//写结果
	outStr.clear();
	Sleep(100);
	//通知shell可以取数据了
	ReleaseSemaphore(shell, 1, NULL);
	Sleep(200);//留够时间给
	cout << "假装还在读，每次读5s" << endl;
	Sleep(5 * 1000);
	strcpy_s(file->ret, 1024 * 10, "\0");//把返回结果清空，已经提前返回了
	//读完了，退出时要把读者数减一

	WaitForSingleObject(mutex, INFINITE);
	accessTable->readNum[fileInode->idx] -= 1;
	ReleaseSemaphore(mutex, 1, NULL);
	cout << "读完了！" << endl;
	return;
}

void FileSystem::del(string path) {
	path = cmpPath(path);//自动补齐路径
	inode* fileInode = getFileInode(path);//这一步可以保证取出来的是文件
	if (fileInode == NULL) {
		outStr +=  "路径有误！del命令终止！\n";
		return;
	}
	delI(fileInode);
	return;
}

void FileSystem::check() {
	bitset<8192> newInodeBMap; //1个block存  1024B
	bitset<100 * 1024> newBlockBMap; //13个block存  12.5*1024B
	newInodeBMap.reset();
	newBlockBMap.reset();
	for (int i = 0; i < 2063; ++i) {//前面的0-2062块已经被使用
		newBlockBMap.set(i);
	}
	checkHelp(0, newInodeBMap, newBlockBMap);
	if (inodeBMap == newInodeBMap && blockBMap == newBlockBMap) {
		outStr +=  "检查完毕,未发现错误！\n";
		return;
	}
	else {
		outStr +=  "检查完毕,发现以下错误:\n" ;
		if (inodeBMap != newInodeBMap) {
			outStr +=  "发生错误的i结点号:";
			for (int i = 0; i < S.inodeNum; ++i) {
				if (inodeBMap[i] != newInodeBMap[i]) {
					outStr += to_string(i);
					outStr+=' ';
				}
			}
			outStr +='\n';
		}
		if (blockBMap != newBlockBMap) {
			outStr +=  "发生错误的数据块号:";
			for (int i = 0; i < S.blockNum; ++i) {
				if (blockBMap[i] != newBlockBMap[i]) {
					outStr += to_string(i);
					outStr+=' ';
				}
			}
			outStr += '\n';
		}
		inodeBMap = newInodeBMap;
		blockBMap = newBlockBMap;
		save();
		outStr +=  "错误已修复\n";
	}
}
void FileSystem::chmod(string path, int mod) {
	path = cmpPath(path);//自动补齐路径
	inode* fileInode = getFileInode(path);//这一步可以保证取出来的是文件
	if (fileInode == NULL) {
		outStr += "路径有误！chmod命令终止！\n";
		return;
	}
	if (fileInode->uid != uid) {//只有文件所有者可以改变保护模式
		outStr += "没有修改该文件保护模式的权限！\n";
		return;
	}
	fileInode->mod = mod;
	postInode(fileInode);
}
void FileSystem::copy(string src, string dst) {//复制文件
	bool srcFlag = 0, dstFlag = 0;//是否在主机磁盘上，是的话置1
	if (src.size() >= 6 && src.substr(0, 6) == "<host>") {
		srcFlag = 1;
	}
	if (dst.size() >= 6 && dst.substr(0, 6) == "<host>") {
		dstFlag = 1;
	}
	if (!srcFlag) {//不是主机路径，可以自动补全
		src = cmpPath(src);//自动补齐路径
	}
	if (!dstFlag) {//不是主机路径，可以自动补全
		dst = cmpPath(dst);//自动补齐路径
	}

	string contents;
	//取出文件中的内容
	if (srcFlag) {//源文件在主机上
		fstream srcFile(src.c_str() + 6,ios::binary|ios::in);//打开文件
		if (!srcFile) {
			outStr +=  "srcFile打开失败！终止！\n";
			return;
		}

		srcFile.seekg(0, std::ios::end);
		contents.resize(srcFile.tellg());//扩展contens大小
		srcFile.seekg(0, std::ios::beg);
		srcFile.read(&contents[0], contents.size());//读入文件信息
		srcFile.close();
		contents = UTF8ToGB(contents.c_str());
	}
	else {//源文件在simdisk上
		contents=getFileContent(getFileInode(src));//直接获取
	}
	//写入文件内容
	if (dstFlag) {//目标文件在主机上
		fstream dstFile(dst.c_str() + 6, ios::binary | ios::out);
		if (!dstFile) {
			outStr += "dstFile打开失败！终止！\n";
			return;
		}
		dstFile.seekp(0, ios::beg);
		dstFile.write(&contents[0], contents.size());
		return;
	}
	else {//目标文件在simdisk上
		inode* newInode=newfile(dst);
		if (newInode == NULL) {
			outStr +=  "创建文件失败！终止！\n";
			return;
		}
		fwriteHelp(newInode, contents, 0);
	}
	return;
}

void FileSystem::help()
{
	outStr += "\n\t┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
	outStr += "\t┃ 1. info                      显示系统信息       ┃\n";
	outStr += "\t┃ 2. cd        path            改变目录           ┃\n";
	outStr += "\t┃ 3. dir       path            显示目录           ┃\n";
	outStr += "\t┃ 4. md        path            创建目录           ┃\n";
	outStr += "\t┃ 5. rd        path            删除目录           ┃\n";
	outStr += "\t┃ 6. newfile   path            建立文件           ┃\n";
	outStr += "\t┃ 7. cat       path            打开文件           ┃\n";
	outStr += "\t┃ 8. copy      src    dst      拷贝文件           ┃\n";
	outStr += "\t┃ 9. del       path            删除文件           ┃\n";
	outStr += "\t┃ 10.check                     检测并恢复文件系统 ┃\n";
	outStr += "\t┃ 11.format                    格式化磁盘         ┃\n";
	outStr += "\t┃ 12.fwrite    path appFlag    写文件             ┃\n";
	outStr += "\t┃ 13.chmod     path mod        修改文件访问权限   ┃\n";
	outStr += "\t┃ 14.help                      显示帮助           ┃\n";
	outStr += "\t┃ 15.exit                      退出文件系统       ┃\n";
	outStr += "\t┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
}