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
	cout << "����ʣ��ռ䣺" << freeSpace << "MB" << endl;
	cout << "���д���inode����" << freeInode << endl;
	cout << "��Ŀ¼����" << dirNum << endl;
	cout << "���ļ�����" << fileNum << endl;
}

void FileSystem::md(string path) {
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