#include"FileSystem.h"
#include<iostream>
#include<string>
#include<iomanip>
using namespace std;

void FileSystem::info() {
	//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//�򿪴���
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
	//FILE.close();
	S.print();
	cout << "����ʣ��ռ䣺" << freeSpace << "MB" << endl;
	cout << "���д���inode����" << freeInode << endl;
	cout << "��Ŀ¼����" << dirNum << endl;
	cout << "���ļ�����" << fileNum << endl;
}

void FileSystem::md(string path) {
	//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//�򿪴���
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
	cout<<left<<setw(10)<< dirInode->mod << setw(8) << dirInode->uid << setw(8) << dirInode->gid << setw(12) <<
		dirInode->getSize() << setw(24) << dirInode->getModiTime() << setw(12) << "." << endl;//����ģʽ ������ ������ ��С ����ʱ�� ���� 
	for (int i = 0; i < dirInode->size; ++i) {//��ӡ�ļ��е�Ŀ¼��
		DirectoryItem* dirItem = getDirectorItem(dirInode, i);
		dirItem->print();
		cout << endl;
	}
	//FILE.close();
	return;
}