#include"FileSystem.h"
#include<iostream>
#include<string>
#include<iomanip>
using namespace std;

void FileSystem::format() {//��ʽ��Ӳ��
	//��������¼����
	cout << "ȷ����ʼ�������𣿣�Y/N��" << endl;
	char ch;
	cin >> ch;
	if (ch == 'Y' || ch == 'y') {
		cout << "���ڳ�ʼ�����̣����Ե�......" << endl;
		init();
		load();

	}
	else if (ch == 'N' || ch == 'n') {
		return;
	}
	else {
		cout << "�������format()������ֹ��" << endl;
	}
}

void FileSystem::info() {
	if (!FILE) {
		cerr << "info()" << "�򿪴���ʧ�ܣ�" << endl;
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
	S.print();
	cout << "\t����ʣ��ռ�:\t\t" << freeSpace << "MB" << endl;
	cout << "\t���д���inode��:\t" << freeInode << endl;
	cout << "\t��Ŀ¼��:\t\t" << dirNum << endl;
	cout << "\t���ļ���:\t\t" << fileNum << endl;
}

void FileSystem::md(string path) {
	path = cmpPath(path);
	if (!FILE) {
		cerr << "md()" << "�򿪴���ʧ�ܣ�" << endl;
	}
	//Ԥ����
	int parentIdx = getParentDirIndex(path);//��ȡ���ļ��е�i�ڵ��
	if (parentIdx == -1) {
		cerr << "·�����������ֹ���" << endl;
		return;
	}
	inode* parentInode = getInode(parentIdx);
	string newDirName = split(path, "/").back();
	if (newDirName.empty()) {
		cerr << "���ܴ������Ŀ¼ͬ����Ŀ¼����ֹ���" << endl;
		return;
	}
	if (dirFindByName(parentInode, newDirName) != -1) {
		cerr << "�ļ������Ѿ�������Ϊ" << newDirName << "���ļ�,��ֹ���" << endl;
		return;
	}
	dirInit(parentInode, newDirName);//�����µ��ļ���
	FILE.flush();
}

void FileSystem::dir(string path) {
	path = cmpPath(path);
	if (!FILE) {
		cerr << "dir()" << "�򿪴���ʧ�ܣ�" << endl;
	}
	int inodeIdx = getIndex(path);//
	inode* dirInode = getInode(inodeIdx);
	if (dirInode->type != '0') {
		cerr << "dir()����" << path << "�����ļ��У�" << endl;
	}
	cout<<left<<setw(14)<< getMod(dirInode) << setw(5) << dirInode->uid //��ӡ�Լ�����Ϣ 
		<< setw(8) << dirInode->gid << setw(10) 
		<<dirInode->getSize() << setw(24) << dirInode->getModiTime() 
		<< setw(12) << "." << endl;//����ģʽ ���� ������ ��С ����ʱ�� ���� 
	inode* subInode;
	for (int i = 0; i < dirInode->size; ++i) {//��ӡ�ļ��е�Ŀ¼��
		DirectoryItem* dirItem = getDirectorItem(dirInode, i);
		subInode = getInode(dirItem->inodeIdx);
		cout << left << setw(14) << getMod(subInode) << setw(5) << subInode->uid << setw(8)
			<< subInode->gid << setw(10) 
			<<subInode->getSize() << setw(24) << subInode->getModiTime() 
			<< setw(12) << subInode->name << endl;//����ģʽ ������ ������ ��С ����ʱ�� ���� 
	}
	return;
}

void FileSystem::cd(string path) {
	path = cmpPath(path);//�Զ�����·��
	if (getIndex(path) == -1) {
		cerr << "·�������ڣ���ֹ���" << endl;
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
			cerr << "�Ѿ��Ǹ�Ŀ¼��û����һ��Ŀ¼����ֹ��" << endl;
			return;
		}
		newStack.pop_back();
		curInodeIdx = newStack.back()->idx;//����Ŀ¼�����һ��
		curInode = newStack.back();
	}
	else {//��������
		cerr << "cd()����·���������" << endl;
		return;//����
	}
	for (int i = 1; i < pathSplit.size(); ++i) {//
		curInodeIdx = dirFindByName(curInode, pathSplit[i]);//ȡ��һ��Ŀ¼��i�ڵ��
		if (curInodeIdx == -1) {
			cout << "cd()����!";
			cout << curInode->name << "��û������Ϊ" << pathSplit[i] << "���ļ���Ŀ¼" << endl;
			return;
		}
		else {
			curInode = getInode(curInodeIdx);//ȡi�ڵ�
			if (curInode->type != '0') {
				cout << "·���е�" << curInode->name << "�����ļ��У���ֹcd��" << endl;
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
	path = cmpPath(path);//�Զ�����·��
	int dirInodeIdx=getIndex(path);
	if (dirInodeIdx == -1) {
		cerr <<'"'<<path<<'"' << "�����ڣ�rd()������ֹ��" << endl;
		return;
	}
	if (dirInodeIdx == 0) {
		cerr << "��Ŀ¼����ɾ����rd()������ֹ��" << endl;
		return;
	}
	inode* dirInode = getInode(dirInodeIdx);
	if (dirInode->size == 0) {//�Ǹ����ļ���
		delI(dirInode);//ֱ�Ӱ�����ļ��ͷž���
	}
	else {
		cout << path << "�ǿգ�ȷ��Ҫɾ����(Y/N)" << endl;
		char ch;
		cin >> ch;
		if (ch == 'N' || ch == 'n') {
			cout << "����ɾ��!" << endl;
			return;
		}
		else if (ch == 'Y' || ch == 'y') {
			delI(dirInode);//����ɾ����Ŀ¼
			return;
		}
		else {
			cerr << "�������rd��ֹ���������������" << endl;
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
		cerr << "newfile()·��������ֹ���" << endl;
		return NULL;
	}
	vector<string> pathSplit=split(path,"/");
	string fileName = pathSplit.back();
	int findNameFlag=dirFindByName(parentInode, fileName);
	if (findNameFlag != -1) {
		cout << "��Ŀ¼���Ѿ���������Ϊ" << fileName << "���ļ���Ŀ¼����ֹ����!" << endl;
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
	newInode->uid=0;//�����û�id
	newInode->gid=10000;//������id
	newInode->mod=777;//����ģʽ
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
	return newInode;
}

void FileSystem::fwrite(string path, bool appFlag = 1) {
	path = cmpPath(path);//�Զ�����·��
	//��ȡ·����ָi�ڵ�
	inode* fileInode = getInode(getIndex(path));
	if (fileInode == NULL) {
		cerr << "fwrite()·�����Ҵ�����ֹ��" << endl;
		return;
	}
	//�ж�inode�Ƿ����ļ�����
	if (fileInode->type != '1') {
		cerr << "��·����ָ���Ͳ����ļ����ͣ���ֹ��" << endl;
		return;
	}
	if (appFlag) {//��׷��ģʽд�ļ����Ȱ�ԭ�ļ������ݴ�ӡ����
		cout << "ԭ�ļ����ݣ�" <<endl<<getFileContent(fileInode) << endl;//��ʾ�ļ�����
		cout << "������׷�ӵ��ļ����ݣ�(����س�+(ctrl+z)����)" << endl;
	}
	else {
		cout << "������д���ļ����ݣ�(����س�+(ctrl+z)����)" << endl;
	}
	string content;
	char ch;
	while ((ch=getchar())!=EOF) {
		content += ch;
	}
	content.erase(content.size() - 1);//��ȥ���һ�����з�
	if (fwriteHelp(fileInode, content, appFlag) < 0) {
		cerr << "�����ļ�����!" << endl;
		return;
	}
	else {
		cout << "����ɹ���" << endl;
	}
}

void FileSystem::cat(string path) {
	path = cmpPath(path);//�Զ�����·��
	inode* fileInode = getFileInode(path);
	cout << getFileContent(fileInode) << endl;
	return;
}

void FileSystem::del(string path) {
	path = cmpPath(path);//�Զ�����·��
	inode* fileInode = getFileInode(path);//��һ�����Ա�֤ȡ���������ļ�
	if (fileInode == NULL) {
		cerr << "·������del������ֹ��" << endl;
		return;
	}
	delI(fileInode);
	return;
}

void FileSystem::copy(string src, string dst) {//�����ļ�
	bool srcFlag = 0, dstFlag = 0;//�Ƿ������������ϣ��ǵĻ���1
	if (src.size() >= 6 && src.substr(0, 6) == "<host>") {
		srcFlag = 1;
	}
	if (dst.size() >= 6 && dst.substr(0, 6) == "<host>") {
		dstFlag = 1;
	}
	string contents;
	//ȡ���ļ��е�����
	if (srcFlag) {//Դ�ļ���������
		fstream srcFile(src.c_str() + 6,ios::binary|ios::in);//���ļ�
		if (!srcFile) {
			cerr << "srcFile��ʧ�ܣ���ֹ��" << endl;
			return;
		}

		srcFile.seekg(0, std::ios::end);
		contents.resize(srcFile.tellg());//��չcontens��С
		srcFile.seekg(0, std::ios::beg);
		srcFile.read(&contents[0], contents.size());//�����ļ���Ϣ
		srcFile.close();
	}
	else {//Դ�ļ���simdisk��
		contents=getFileContent(getFileInode(src));//ֱ�ӻ�ȡ
	}
	//д���ļ�����
	if (dstFlag) {//Ŀ���ļ���������
		fstream dstFile(dst.c_str() + 6, ios::binary | ios::out);
		if (!dstFile) {
			cerr << "dstFile��ʧ�ܣ���ֹ��" << endl;
			return;
		}
		dstFile.seekp(0, ios::beg);
		dstFile.write(&contents[0], contents.size());
		return;
	}
	else {//Ŀ���ļ���simdisk��
		inode* newInode=newfile(dst);
		if (newInode == NULL) {
			cerr << "�����ļ�ʧ�ܣ���ֹ��" << endl;
			return;
		}
		fwriteHelp(newInode, contents, 0);
	}
	return;
}

void FileSystem::help()
{
	cout <<"\n\t������������������������������������������������������������������������������������������������������" << endl;
	cout <<  "\t�� 1. info                      ��ʾϵͳ��Ϣ       ��" << endl;
	cout <<  "\t�� 2. cd        path            �ı�Ŀ¼           ��" << endl;
	cout <<  "\t�� 3. dir       path            ��ʾĿ¼           ��" << endl;
	cout <<  "\t�� 4. md        path            ����Ŀ¼           ��" << endl;
	cout <<  "\t�� 5. rd        path            ɾ��Ŀ¼           ��" << endl;
	cout <<  "\t�� 6. newfile   path            �����ļ�           ��" << endl;
	cout <<  "\t�� 7. cat       path            ���ļ�           ��" << endl;
	cout <<  "\t�� 8. copy      src dst         �����ļ�           ��" << endl;
	cout <<  "\t�� 9. del       path            ɾ���ļ�           ��" << endl;
	cout <<  "\t�� 10.check                     ��Ⲣ�ָ��ļ�ϵͳ ��" << endl;
	cout <<  "\t�� 11.format                    ��ʽ������         ��" << endl;
	cout <<  "\t�� 12.fwrite    path appFlag    д�ļ�             ��" << endl;
	cout <<  "\t�� 13.ctrl+z                    �˳��ļ�ϵͳ       ��" << endl;
	cout <<  "\t�� 14.help                      ��ʾ����           ��" << endl;
	cout <<  "\t������������������������������������������������������������������������������������������������������" << endl;
}