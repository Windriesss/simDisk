#include"FileSystem.h"
#include<iostream>
#include<string>
#include<iomanip>
using namespace std;

void FileSystem::format() {//��ʽ��Ӳ��
	//��������¼����
	outStr+= "ȷ����ʼ�������𣿣�Y/N��\n";
	strcpy_s(file->ret, 1024 * 10, outStr.c_str());//д���
	outStr.clear();
	Sleep(100);//ͣһ�ᣬ�ȴ����д��
	ReleaseSemaphore(shell, 1, NULL);//֪ͨshell���Ի�ȡ�������ִ��
	WaitForSingleObject(disk, INFINITE);//����disk
	string content(file->cmd);
	if (uid != 0) {//����root�û�
		outStr += "�û�Ȩ�޲��㣡\n";
		return;
	}
	if (content == "Y" || content == "y") {
		init();
		load();
		outStr += "Ӳ�̳�ʼ����ϣ�\n";
	}
	else if (content == "N" || content == "n") {
		outStr += "������ʼ��\n";
		return;
	}
	else {
		outStr +=  "�������format()������ֹ��\n" ;
	}
}

void FileSystem::info() {
	if (!FILE) {
		outStr += "info()";
		outStr += "�򿪴���ʧ�ܣ�\n";
		return;
	}
	int fileNum = 0;//�ļ�����
	int dirNum = 0;//�ļ�������
	int freeSpace = (S.blockNum - blockBMap.count()) / 1024;//�������ݿ�������λMB
	int freeInode = S.inodeNum - inodeBMap.count();//����i�ڵ����
	for (int i = 0; i < S.inodeNum; ++i) {
		if (inodeBMap[i]) {
			inode* t = getInode(i);
			if (t->type == '0') dirNum++;
			if (t->type == '1') fileNum++;
		}
	}
	outStr += "\t��������:\t\t";
	outStr += to_string(S.diskSize / 1024 / 1024);
	outStr += "MB\n";
	outStr += "\t���̿�����:\t\t";
	outStr += to_string(S.blockNum);
	outStr += "��\n";
	outStr += "\ti�ڵ�����:\t\t";
	outStr += to_string(S.inodeNum);
	outStr += '\n';
	outStr += "\ti�ڵ�λͼ��ʼ��ַ:\t";
	outStr += to_string(S.inodeBMapPos / 1024);
	outStr += '\n';
	outStr += "\ti�ڵ�����ʼ��ַ:\t";
	outStr += to_string(S.inodePos / 1024);
	outStr += '\n';
	outStr += "\t��λͼ��ʼ��ַ:\t\t";
	outStr += to_string(S.blockBMapPos / 1024);
	outStr += '\n';
	outStr += "\t���ݿ���ʼ��ַ:\t\t";
	outStr += to_string(S.blockPos / 1024);
	outStr += '\n';
	outStr += "\t����ʣ��ռ�:\t\t";
	outStr += to_string(freeSpace);
	outStr +="MB\n";
	outStr += "\t����inode��:\t\t";
	outStr += to_string(freeInode);
	outStr += '\n';
	outStr += "\t��Ŀ¼��:\t\t";
	outStr += to_string(dirNum);
	outStr += '\n';
	outStr += "\t���ļ���:\t\t";
	outStr += to_string(fileNum);
	outStr += '\n';
}

void FileSystem::md(string path) {
	path = cmpPath(path);
	if (!FILE) {
		outStr += "md()";
		outStr += "�򿪴���ʧ�ܣ�\n";
	}
	//Ԥ����
	int parentIdx = getParentDirIndex(path);//��ȡ���ļ��е�i�ڵ��
	if (parentIdx == -1) {
		outStr += "·�����������ֹ���\n";
		return;
	}
	inode* parentInode = getInode(parentIdx);
	string newDirName = split(path, "/").back();
	if (newDirName.empty()) {
		outStr += "���ܴ������Ŀ¼ͬ����Ŀ¼����ֹ���\n";;
		return;
	}
	if (dirFindByName(parentInode, newDirName) != -1) {
		outStr += "�ļ������Ѿ�������Ϊ";
		outStr += newDirName;
		outStr += "���ļ�,��ֹ���\n";
		return;
	}
	dirInit(parentInode, newDirName);//�����µ��ļ���
	FILE.flush();
}

void FileSystem::dir(string path) {
	path = cmpPath(path);
	if (!FILE) {
		outStr += "dir()";
		outStr += "�򿪴���ʧ�ܣ�\n";
		return;
	}
	int inodeIdx = getIndex(path);
	if (inodeIdx < 0) {
		outStr += "·������\n";
		return;
	}
	inode* dirInode = getInode(inodeIdx);
	if (dirInode->type != '0') {
		outStr += "dir()����";
		outStr += path;
		outStr += "�����ļ��У�\n";
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
	outStr += '\n';//����ģʽ ������ ������ ��С ����ʱ�� ���� 

	inode* subInode;
	for (int i = 0; i < dirInode->size; ++i) {//��ӡ�ļ��е�Ŀ¼��
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
		outStr += '\n';//����ģʽ ������ ������ ��С ����ʱ�� ���� 
	}
	return;
}

void FileSystem::cd(string path) {
	path = cmpPath(path);//�Զ�����·��
	if (getIndex(path) == -1) {
		outStr +=  "·�������ڣ���ֹ���\n";
		return;
	}
	updateInodeStack();//����InodeStack�е���Ϣ
	vector<inode*> newStack = inodeStack;
	vector<string> pathSplit = split(path, "/");
	inode* curInode;
	int curInodeIdx = -1;
	if (pathSplit[0].empty()) {//����·��,�Ӹ�Ŀ¼��ʼ
		while (newStack.size() > 1) newStack.pop_back();//������ֱ��i�ڵ�ջ��ֻʣ�¸�Ŀ¼
		curInodeIdx = 0;
		curInode = newStack[0];
	}
	else if (pathSplit[0] == "." ) {//�ӵ�ǰĿ¼��ʼ
		curInodeIdx = newStack.back()->idx;//����Ŀ¼�����һ��
		curInode = newStack.back();
	}
	else if (pathSplit[0] == "..") {
		if (newStack.size() == 1) {
			outStr +=  "�Ѿ��Ǹ�Ŀ¼��û����һ��Ŀ¼����ֹ��\n";
			return;
		}
		newStack.pop_back();
		curInodeIdx = newStack.back()->idx;//����Ŀ¼�����һ��
		curInode = newStack.back();
	}
	else {//��������
		outStr += "cd()����·���������\n";
		return;//����
	}
	for (int i = 1; i < pathSplit.size(); ++i) {//
		curInodeIdx = dirFindByName(curInode, pathSplit[i]);//ȡ��һ��Ŀ¼��i�ڵ��
		if (curInodeIdx == -1) {
			outStr +=  "cd()����!";
			outStr += curInode->name;
			outStr += "��û������Ϊ";
			outStr += pathSplit[i];
			outStr += "���ļ���Ŀ¼\n";
			return;
		}
		else {
			curInode = getInode(curInodeIdx);//ȡi�ڵ�
			if (curInode->type != '0') {
				outStr += "·���е�";
				outStr += curInode->name;
				outStr += "�����ļ��У���ֹcd��\n";
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
	path = cmpPath(path);//�Զ�����·��
	int dirInodeIdx=getIndex(path);
	if (dirInodeIdx == -1) {
		outStr += '"';
		outStr += path;
		outStr += '"';
		outStr += "�����ڣ�rd()������ֹ��\n";
		return;
	}
	if (dirInodeIdx == 0) {
		outStr += "��Ŀ¼����ɾ����rd()������ֹ��\n";
		return;
	}
	inode* dirInode = getInode(dirInodeIdx);
	if (dirInode->size == 0) {//�Ǹ����ļ���
		delI(dirInode);//ֱ�Ӱ�����ļ��ͷž���
	}
	else {

		outStr += path;
		outStr += "�ǿգ�ȷ��Ҫɾ����(Y/N)\n";
		strcpy_s(file->ret, 1024 * 10, outStr.c_str());//д���
		file->errorFlag = 1;
		outStr.clear();
		Sleep(100);//ͣһ�ᣬ�ȴ����д��
		ReleaseSemaphore(shell, 1, NULL);//֪ͨshell���Ի�ȡ�������ִ��
		WaitForSingleObject(disk, INFINITE);//����disk
		string content(file->cmd);
		if (content == "N" || content == "n") {
			outStr +=  "����ɾ��!\n";
			return;
		}
		else if (content == "Y" || content == "y") {
			delI(dirInode);//����ɾ����Ŀ¼
			return;
		}
		else {
			outStr += "�������rd��ֹ���������������\n";
			return;
		}
	}
	return;
}

inode* FileSystem::newfile(string path) {
	path = cmpPath(path);//�Զ�����·��
	int parentInodeIdx=getParentDirIndex(path);//�ҵ�·���еĸ�Ŀ¼
	inode* parentInode = getInode(parentInodeIdx);
	if (parentInodeIdx < 0) {
		outStr +=  "newfile()·��������ֹ���\n";
		return NULL;
	}
	vector<string> pathSplit=split(path,"/");
	string fileName = pathSplit.back();
	int findNameFlag=dirFindByName(parentInode, fileName);
	if (findNameFlag != -1) {
		outStr += "��Ŀ¼���Ѿ���������Ϊ";
		outStr += fileName;
		outStr += "���ļ���Ŀ¼����ֹ����!\n";
		return NULL;
	}

	//����i�ڵ�
	int newInodeIdx = RequestI();
	inode* newInode = getInode(newInodeIdx);
	//�����i�ڵ��������ݿ�
	int flag=RequestD(newInode->dataBlock, 10);//����10��Ӳ�̿�
	//���ʧ�ܣ������ͷ�i�ڵ�
	if (flag < 0) {
		delI(newInode);
	}
	//i�ڵ�����ݿ鶼����ɹ����޸�i�ڵ���Ϣ
	newInode->idx=newInodeIdx;//inode���
	newInode->parentIdx=parentInodeIdx;//��Ŀ¼��inode���
	newInode->linkNum=1;//ָ���inode���ļ���
	newInode->uid=uid;//�����û�id
	newInode->gid=10000+uid;//������id
	newInode->mod=770;//����ģʽ
	strcpy_s(newInode->name,136,fileName.c_str());//�ļ���
	newInode->type='1';//�ļ�����  0��ʾĿ¼  1��ʾ�ļ�
	GetLocalTime(&(newInode->creatTime));//����ʱ��  16B
	newInode->modiTime=newInode->creatTime;//���һ���޸�ʱ��
	newInode->size=0;//�ļ���С ��λB
	//���ݿ��Ѿ��������,����Ҫ���⴦��
	//�ش�i�ڵ�
	postInode(newInode);
	//�ش�Ŀ¼��
	postDirItem(parentInode, newInode);
	postInode(parentInode);
	return newInode;
}

void FileSystem::fwrite(string path, bool appFlag = 1) {
	path = cmpPath(path);//�Զ�����·��
	//��ȡ·����ָi�ڵ�
	inode* fileInode = getInode(getIndex(path));
	if (fileInode == NULL) {
		outStr += "fwrite()·�����Ҵ�����ֹ��\n";
		file->errorFlag = -1;
		return;
	}
	//�ж�inode�Ƿ����ļ�����
	if (fileInode->type != '1') {
		outStr += "��·����ָ���Ͳ����ļ����ͣ���ֹ��\n";
		file->errorFlag = -1;
		return;
	}
	string mod = getMod(fileInode);//�жϱ���ģʽ
	if (fileInode->uid != uid) {//�ļ������ڸò�����
		if (mod[8] != 'w') {
			outStr += "û��д��Ȩ��\n";
			file->errorFlag = -1;
			return;
		}
	}
	else {//�ļ����ڸò�����
		if (mod[2] != 'w') {
			outStr += "û��д��Ȩ��\n";
			file->errorFlag = -1;
			return;
		}
	}
	//-------------�鿴�Ƿ���д-----------
	//������ʵǼǱ�
	WaitForSingleObject(mutex, INFINITE);
	//�鿴�Ƿ�������д
	if (accessTable->writeFlag[fileInode->idx]) {//���û�����д
		outStr += "���ļ����ڱ�д�룬���Ժ��ٳ���\n";
		file->errorFlag = -1;
		ReleaseSemaphore(mutex, 1, NULL);//�ͷŷ��ʱ�
		return;
	}
	//�鿴�Ƿ������ڶ�
	if (accessTable->readNum[fileInode->idx]) {//���û����ڶ�
		outStr += "���ļ����ڱ���ȡ�����Ժ��ٳ���\n";
		file->errorFlag = -1;
		ReleaseSemaphore(mutex, 1, NULL);//�ͷŷ��ʱ�
		return;
	}
	//û������д����д,��д��־��1
	accessTable->writeFlag.set(fileInode->idx);
	//�ͷŷ��ʵǼǱ�
	ReleaseSemaphore(mutex, 1, NULL);
	//д����
	//-------------------
	if (appFlag) {//��׷��ģʽд�ļ����Ȱ�ԭ�ļ������ݴ�ӡ����
		outStr += "ԭ�ļ����ݣ�\n";
		outStr += getFileContent(fileInode);
		outStr += '\n';//��ʾ�ļ�����
		outStr += "������׷�ӵ��ļ����ݣ�(����س�+(ctrl+z)����)\n";
	}
	else {
		outStr += "������д���ļ����ݣ�(����س�+(ctrl+z)����)\n";
	}
	strcpy_s(file->ret, 1024 * 10, outStr.c_str());//д���
	outStr.clear();
	Sleep(100);//ͣһ�ᣬ�ȴ����д��
	ReleaseSemaphore(shell, 1, NULL);//֪ͨshell���Ի�ȡ�������ִ��
	WaitForSingleObject(disk, INFINITE);//����disk
	string content(file->cmd);
	if (fwriteHelp(fileInode, content, appFlag) < 0) {
		outStr += "�����ļ�����!\n";
		file->errorFlag=-1;
	}
	else {
		outStr += "����ɹ���\n";
	}
	//д���ˣ��˳�ʱҪ��д��־��0
	WaitForSingleObject(mutex, INFINITE);
	accessTable->writeFlag.reset(fileInode->idx);
	ReleaseSemaphore(mutex, 1, NULL);
}

void FileSystem::cat(string path) {
	path = cmpPath(path);//�Զ�����·��
	inode* fileInode = getFileInode(path);
	if (fileInode == NULL) {
		outStr += "·������\n";
		file->errorFlag = -1;
		return;
	}
	string mod = getMod(fileInode);
	if (fileInode->uid != uid) {//�ļ������ڸò�����
		if (mod[7] != 'r') {
			outStr += "û�ж�ȡȨ��\n";
			file->errorFlag = -1;
			return;
		}
	}
	else {//�ļ����ڸò�����
		if (mod[1] != 'r') {
			outStr += "û�ж�ȡȨ��\n";
			file->errorFlag = -1;
			return;
		}
	}
	//------------�����е�����˵����Ȩ�޶�
	//������ʵǼǱ�
	WaitForSingleObject(mutex, INFINITE);
	//�鿴�Ƿ�������д
	if (accessTable->writeFlag[fileInode->idx]) {//���û�����д
		outStr += "���ļ����ڱ�д�룬���Ժ��ٳ���\n";
		file->errorFlag = -1;
		ReleaseSemaphore(mutex, 1, NULL);//�ͷŷ��ʱ�
		return;
	}
	//û������д,���������1
	accessTable->readNum[fileInode->idx] += 1;
	//�ͷŷ��ʵǼǱ�
	ReleaseSemaphore(mutex, 1, NULL);
	//������
	outStr +=  getFileContent(fileInode);
	outStr += '\n';
	//�����ˣ�ֱ�Ӱ�����д��ret��
	strcpy_s(file->ret, 1024 * 10, outStr.c_str());//д���
	outStr.clear();
	Sleep(100);
	//֪ͨshell����ȡ������
	ReleaseSemaphore(shell, 1, NULL);
	Sleep(200);//����ʱ���
	cout << "��װ���ڶ���ÿ�ζ�5s" << endl;
	Sleep(5 * 1000);
	strcpy_s(file->ret, 1024 * 10, "\0");//�ѷ��ؽ����գ��Ѿ���ǰ������
	//�����ˣ��˳�ʱҪ�Ѷ�������һ

	WaitForSingleObject(mutex, INFINITE);
	accessTable->readNum[fileInode->idx] -= 1;
	ReleaseSemaphore(mutex, 1, NULL);
	cout << "�����ˣ�" << endl;
	return;
}

void FileSystem::del(string path) {
	path = cmpPath(path);//�Զ�����·��
	inode* fileInode = getFileInode(path);//��һ�����Ա�֤ȡ���������ļ�
	if (fileInode == NULL) {
		outStr +=  "·������del������ֹ��\n";
		return;
	}
	delI(fileInode);
	return;
}

void FileSystem::check() {
	bitset<8192> newInodeBMap; //1��block��  1024B
	bitset<100 * 1024> newBlockBMap; //13��block��  12.5*1024B
	newInodeBMap.reset();
	newBlockBMap.reset();
	for (int i = 0; i < 2063; ++i) {//ǰ���0-2062���Ѿ���ʹ��
		newBlockBMap.set(i);
	}
	checkHelp(0, newInodeBMap, newBlockBMap);
	if (inodeBMap == newInodeBMap && blockBMap == newBlockBMap) {
		outStr +=  "������,δ���ִ���\n";
		return;
	}
	else {
		outStr +=  "������,�������´���:\n" ;
		if (inodeBMap != newInodeBMap) {
			outStr +=  "���������i����:";
			for (int i = 0; i < S.inodeNum; ++i) {
				if (inodeBMap[i] != newInodeBMap[i]) {
					outStr += to_string(i);
					outStr+=' ';
				}
			}
			outStr +='\n';
		}
		if (blockBMap != newBlockBMap) {
			outStr +=  "������������ݿ��:";
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
		outStr +=  "�������޸�\n";
	}
}
void FileSystem::chmod(string path, int mod) {
	path = cmpPath(path);//�Զ�����·��
	inode* fileInode = getFileInode(path);//��һ�����Ա�֤ȡ���������ļ�
	if (fileInode == NULL) {
		outStr += "·������chmod������ֹ��\n";
		return;
	}
	if (fileInode->uid != uid) {//ֻ���ļ������߿��Ըı䱣��ģʽ
		outStr += "û���޸ĸ��ļ�����ģʽ��Ȩ�ޣ�\n";
		return;
	}
	fileInode->mod = mod;
	postInode(fileInode);
}
void FileSystem::copy(string src, string dst) {//�����ļ�
	bool srcFlag = 0, dstFlag = 0;//�Ƿ������������ϣ��ǵĻ���1
	if (src.size() >= 6 && src.substr(0, 6) == "<host>") {
		srcFlag = 1;
	}
	if (dst.size() >= 6 && dst.substr(0, 6) == "<host>") {
		dstFlag = 1;
	}
	if (!srcFlag) {//��������·���������Զ���ȫ
		src = cmpPath(src);//�Զ�����·��
	}
	if (!dstFlag) {//��������·���������Զ���ȫ
		dst = cmpPath(dst);//�Զ�����·��
	}

	string contents;
	//ȡ���ļ��е�����
	if (srcFlag) {//Դ�ļ���������
		fstream srcFile(src.c_str() + 6,ios::binary|ios::in);//���ļ�
		if (!srcFile) {
			outStr +=  "srcFile��ʧ�ܣ���ֹ��\n";
			return;
		}

		srcFile.seekg(0, std::ios::end);
		contents.resize(srcFile.tellg());//��չcontens��С
		srcFile.seekg(0, std::ios::beg);
		srcFile.read(&contents[0], contents.size());//�����ļ���Ϣ
		srcFile.close();
		contents = UTF8ToGB(contents.c_str());
	}
	else {//Դ�ļ���simdisk��
		contents=getFileContent(getFileInode(src));//ֱ�ӻ�ȡ
	}
	//д���ļ�����
	if (dstFlag) {//Ŀ���ļ���������
		fstream dstFile(dst.c_str() + 6, ios::binary | ios::out);
		if (!dstFile) {
			outStr += "dstFile��ʧ�ܣ���ֹ��\n";
			return;
		}
		dstFile.seekp(0, ios::beg);
		dstFile.write(&contents[0], contents.size());
		return;
	}
	else {//Ŀ���ļ���simdisk��
		inode* newInode=newfile(dst);
		if (newInode == NULL) {
			outStr +=  "�����ļ�ʧ�ܣ���ֹ��\n";
			return;
		}
		fwriteHelp(newInode, contents, 0);
	}
	return;
}

void FileSystem::help()
{
	outStr += "\n\t������������������������������������������������������������������������������������������������������\n";
	outStr += "\t�� 1. info                      ��ʾϵͳ��Ϣ       ��\n";
	outStr += "\t�� 2. cd        path            �ı�Ŀ¼           ��\n";
	outStr += "\t�� 3. dir       path            ��ʾĿ¼           ��\n";
	outStr += "\t�� 4. md        path            ����Ŀ¼           ��\n";
	outStr += "\t�� 5. rd        path            ɾ��Ŀ¼           ��\n";
	outStr += "\t�� 6. newfile   path            �����ļ�           ��\n";
	outStr += "\t�� 7. cat       path            ���ļ�           ��\n";
	outStr += "\t�� 8. copy      src    dst      �����ļ�           ��\n";
	outStr += "\t�� 9. del       path            ɾ���ļ�           ��\n";
	outStr += "\t�� 10.check                     ��Ⲣ�ָ��ļ�ϵͳ ��\n";
	outStr += "\t�� 11.format                    ��ʽ������         ��\n";
	outStr += "\t�� 12.fwrite    path appFlag    д�ļ�             ��\n";
	outStr += "\t�� 13.chmod     path mod        �޸��ļ�����Ȩ��   ��\n";
	outStr += "\t�� 14.help                      ��ʾ����           ��\n";
	outStr += "\t�� 15.exit                      �˳��ļ�ϵͳ       ��\n";
	outStr += "\t������������������������������������������������������������������������������������������������������\n";
}