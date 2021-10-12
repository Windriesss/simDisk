#include "FileSystem.h"
#include"Struct.h"
#include<bitset>
#include <fstream>
#include <cstring>
#include <iostream>
#include <windows.h>

using namespace std;

FileSystem::FileSystem() {
	FILE.open("VirtualDisk", ios::binary | ios::in);//���Կ���������Ӳ��
	if (!FILE) {
		cout << "δ��������Ӳ�̣���������Ӳ��" << endl;
		init();
	}
	FILE.close();
	//-------------�Ȱ�init���Ժ�----------
	FILE.open("VirtualDisk", ios::binary | ios::in | ios::out);
	if (!FILE) {
		cerr << "FileSystem������Ӳ��ʧ�ܣ�" << endl;
	}
	load();//������״̬
}

void FileSystem::init() {
	cout << "���ڴ����ļ�ϵͳ ..." << endl;
	FILE.open("VirtualDisk", ios::binary|ios::out);//����100M���̿ռ�
	if (!FILE) {
		cerr << "�����������ʧ��!" << endl;
		return;
	}
	//------------��ʼ��������------------
	S.diskSize = 100 * 1024 * 1024;//������������λB  100*1024*1024B
	S.blockSize = 1024;//���̿���������λB 1024B
	S.blockNum = S.diskSize / S.blockSize;//���̿����� diskSize/bolckSize 102400��
	S.blockUsedNum = 1 + 1 + 2048 + 13;//��ʹ�õĴ��̿���
	S.inodeNum = 8192;//i�ڵ�����
	S.inodeUsedNum = 1;//��ʹ�õ�i�ڵ���
	S.inodeBMapPos = 1 * S.blockSize;//i�ڵ�λͼ��ַ,���ڼ����ֽڿ�ʼ����1������i�ڵ�λͼ�������ɴ��8192���ļ����ļ���
	S.inodePos = 2 * S.blockSize;//i�ڵ���,�ڸ��ֽڿ�ʼ��i�����
	S.blockBMapPos = 2050 * S.blockSize;//��λͼ��ַ,���ڼ����ֽڿ�ʼ����13(12.5)�����������ݿ�����λͼ  
	S.blockPos = 2063 * S.blockSize;
	//-------------������ʼ��������-------------


	FILE.seekp(0, ios::beg);//����ռ䣬ȫ������Ϊ0
	for (int i = 0; i < S.blockNum * S.blockSize; ++i) {//����blockNum�����̿�
		FILE.write(" ", sizeof(char));
	}


	FILE.seekp(0, ios::beg);//�Ƶ���ͷ����䳬������
	FILE.write((char*)&S, sizeof(S));
	
	FILE.seekp(S.inodeBMapPos, ios::beg);//׼����inodeλͼ
	inodeBMap.set(0);//0���������Ŀ¼
	FILE.write((char*)&inodeBMap, sizeof(inodeBMap));//��inodeλͼ

	FILE.seekp(S.blockBMapPos, ios::beg);//׼����blockBnodeλͼ
	for (int i = 0; i < 2063; ++i) {//ǰ���0-2062���Ѿ���ʹ��
		blockBMap.set(i);
	}
	FILE.write((char*)&blockBMap, sizeof(blockBMap));//��blockλͼ
	//---------------��Ŀ¼i�ڵ�--------------
	inode t;
	t.idx=0;//inode���
	t.parentIdx=-1;//��Ŀ¼��inode���
	t.linkNum=1;//ָ���inode���ļ���
	t.uid=0;//�����û�id  0��ʾroot�û�
	t.gid=10000;//������id    10000��ʾroot��
	t.mod=777;//����ģʽ �����˿ɶ���д��ִ��
	strcpy_s(t.name,136,"/");//�ļ���
	t.type='0';//�ļ�����  0��Ŀ¼��1���ļ�
	GetLocalTime(&t.creatTime);//����ʱ��  16B
	t.modiTime=t.creatTime;//���һ���޸�ʱ��
	t.size=0;//�ļ���С ��λB
	t.dataBlock[0] = { -1 };//14�����ݿ�ĵ�ַ����4����һ��ҳ���ļ��������1MB+10KB
	//--------------------------------

	int Rflag = RequestD(t.dataBlock, 10);
	if (Rflag==-1) {//���ݿ鲻�㣬����ʧ��

	}
	FILE.seekp(S.inodePos, ios::beg);
	FILE.write((char*)&t, sizeof(inode));//�Ѹ�Ŀ¼i�ڵ�д��ȥ
	//-----��inode�ĳɿ�inode---------
	//t.idx = 0;//inode��� ��ѭ�����
	t.parentIdx = -1;//��Ŀ¼��inode���
	t.linkNum = 0;//ָ���inode���ļ���
	t.uid = -1;//�����û�id  0��ʾroot�û�
	t.gid = -10000;//������id    10000��ʾroot��
	t.mod = 000;//����ģʽ �����˿ɶ���д��ִ��
	strcpy_s(t.name, 136, "");//�ļ���
	t.type = 'x';//�ļ�����  0��Ŀ¼��1���ļ���xΪ��
	//&t.creatTime;����ʱ�䲻��
	//t.modiTime = t.creatTime;//���һ���޸�ʱ��
	//t.size = 0;//�ļ���С ��λB
	t.dataBlock[0] = 0;//14�����ݿ�ĵ�ַ����4����һ��ҳ���ļ��������1MB+10KB
	//-------------------------------------
	for (int i = 1; i < S.inodeNum; ++i) {
		t.idx = i;
		FILE.write((char*)&t, sizeof(inode));
	}
	FILE.flush();
	return;
}

void FileSystem::load() {
	//FILE.open("VirtualDisk", ios::binary | ios::in|ios::out);//�򿪴���
	if (!FILE) {
		cerr << "load()" << "�򿪴���ʧ�ܣ�" << endl;
	}
	FILE.seekg(0, ios::beg);//��������
	FILE.read((char*)&S, sizeof(S));
	FILE.seekg(S.inodeBMapPos, ios::beg);//��inodeλͼ
	FILE.read((char*)&inodeBMap, sizeof(inodeBMap));
	FILE.seekg(S.blockBMapPos, ios::beg);//��blockλͼ
	FILE.read((char*)&blockBMap, sizeof(blockBMap));
	curInode = new inode;
	FILE.seekg(S.inodePos, ios::beg);//����Ŀ¼i���
	FILE.read((char*)curInode, sizeof(inode));//��ǰ����Ŀ¼���ļ���inode
	curPos = -1;//û�п�ʼ��д
	strcpy_s(curPath, 512, curInode->name);//��ǰ·��
	inodeStack.push(0);//�Ѹ�Ŀ¼��i���ż��뵽·��i�ڵ�ջ��
}

int FileSystem::RequestI() {
	for (int i = 1; i < S.inodeNum; ++i) {
		if (!inodeBMap[i]) {
			inodeBMap.set(i);
			save();
			return i;
		}
	}
	return -1;//-1��ʾû��i�ڵ���
}

int FileSystem::RequestD(int* t, int n) {
	int getNum = 0;
	int i;
	for (i = 0; i < S.blockNum&&getNum<n; ++i) {
		if (!blockBMap[i]) {//�ҵ�һλ��λ
			blockBMap.set(i);//������һλ
			t[getNum++] = i;
		}
	}
	if (i == S.blockNum) {//�ڴ治��
		cerr << "RequestD()�������ݿ鲻�㣡" << endl;
		return -1;//�ڴ治�㣬���ش���
	}
	save();
	return 0;//��������
}

inode* FileSystem::getInode(int idx) {//����i�ڵ�ŷ���i�ڵ��ָ��
	//�����������ǰ�����ȴ��ļ�
	if (!FILE) {
		cerr << "getInode()����ǰû�д��ļ�����ֹ��" << endl;
		//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//�򿪴���
		//return NULL;
	}
	if (idx < 0 || idx >= 8192) {
		cerr << "getInode()����i�����ų�����Χ�������i����Ϊ��"<<idx<<endl;
		return NULL;
	}
	inode* ret = new inode();
	FILE.seekg(S.inodePos+idx*sizeof(inode), ios::beg);//�Ƶ���Ӧ��i�ڵ��λ��
	FILE.read((char*)ret, sizeof(inode));//����
	return ret;
}


vector<string> FileSystem::split(string str, const string& t) {
	vector<string> ret;
	if (str.length() == 0) return ret;
	str += t;//������һ���ָ��������ڷָ�
	while (str.length()) {
		int nextPos = str.find_first_of(t);//�ҵ���һ���ָ������ֵ�λ��
		string sub = str.substr(0, nextPos);//����ǰ���һ���Ӵ��ָ����
		ret.push_back(sub);
		str = str.substr(nextPos + 1, str.size() - (nextPos + 1));//��ĸ���а��Ӵ���ȥ
	}
	for (int i = ret.size()-1; i >0; --i) {
		if (ret[i].empty()) ret.pop_back();
		else break;
	}
	return ret;
}

int FileSystem::getParentDirIndex(string path) {
	if (path.empty()) {
		cerr << "getParentDirIndex()����·��Ϊ�գ�" << endl;
		return -1;
	}
	vector<string> pathSplit = split(path,"/");
	inode* curInode;
	int curInodeIdx = -1;
	if (pathSplit[0].empty()) {//����·��
		curInodeIdx = 0;
		curInode = getInode(curInodeIdx);
	}
	else if (pathSplit[0] == "."|| pathSplit[0] == "..") {
		curInodeIdx = inodeStack.top();//����Ŀ¼�����һ��
		curInode = getInode(curInodeIdx);
		if (pathSplit[0] == ".") {
			curInodeIdx = curInode->parentIdx;//�ù���Ŀ¼����һ��
			curInode = getInode(curInodeIdx);
		}
	}
	else {//��������
		cerr << "getParentDirIndex()����·���������" << endl;
		return -1;//����
	}
	for (int i = 1; i < pathSplit.size()-1; ++i) {//��һ����ȡ
		curInodeIdx = dirFindByName(curInode, pathSplit[i]);//ȡ��һ��Ŀ¼��i�ڵ��
		if (curInodeIdx == 0) {
			cout << "getParentDirIndex()����!";
			cout << curInode->name << "��û������Ϊ" << pathSplit[i] << "���ļ���Ŀ¼" << endl;
			return -1;
		}
		else curInode = getInode(curInodeIdx);//ȡi�ڵ�
	}
	return curInodeIdx;
}

int FileSystem::getIndex(string path) {
	if (path == "/") {//��Ŀ¼û�и�Ŀ¼�����ʺ�����Ĳ���
		return 0;
	}
	int parentInodeIdx=getParentDirIndex(path);//ȡ�丸Ŀ¼��i����
	if (parentInodeIdx == -1) {
		cerr << "getDirIndx()������ֹ���" << endl;
		return -1;
	}
	inode* parentInode = getInode(parentInodeIdx);//ȡ�丸Ŀ¼
	vector<string> pathSplit = split(path,"/");//ȡdir����
	string dirname = pathSplit.back();
	int inodeIdx=dirFindByName(parentInode, dirname);//�ҳ���ȡ�ļ���i����
	return inodeIdx;
}


DirectoryItem* FileSystem::getDirectorItem(inode* dirInode, int idx) {//ȡ�ļ����еĵ�idx��
	if (dirInode->type != '0') {
		cerr << "getDirectorItem()���󣬸��ļ������ļ�������" << endl;
		return NULL;
	}
	if (dirInode->size < idx - 1) {
		cerr << "getDirectorItem()������������Ŀ¼������" << endl;
		return NULL;
	}
	int blockNum = idx / 7;//�ڵڼ������ݿ�
	int divation = idx % 7;//�ڿ��еĵڼ���
	FILE.seekg(dirInode->dataBlock[blockNum] * S.blockSize + divation * sizeof(DirectoryItem), ios::beg);
	DirectoryItem* ret = new DirectoryItem;
	FILE.read((char*)ret, sizeof(DirectoryItem));
	return ret;
}

int FileSystem::dirFindByName(inode* dirInode,string name) {
	if (!FILE) {
		cerr << "dirFindByName()����ǰû�д��ļ�����ֹ��" << endl;
		return -1;
	}
	int FileNum = dirInode->size;//���ļ����ܹ��ж��ٸ��ļ�
	DirectoryItem *dirItem = new DirectoryItem();
	for (int i = 0; i <= FileNum / 7; ++i) {//�ڵڼ����ļ�����,һ���ļ�����Է�7��Ŀ¼��
		FILE.seekg(dirInode->dataBlock[i] * S.blockSize, ios::beg);
		for (int j = 0; j<7 && FileNum>i * 7 + j; ++j) {//�ļ����еĵڼ������Ҳ���Խ�����
			FILE.read((char*)dirItem, sizeof(DirectoryItem));
			if (name.compare(dirItem->name) == 0) {//������ͬ�����Է��ؽ����
				return dirItem->inodeIdx;//����i�ڵ����
			}
		}
	}
	if (!FILE) {
		cout << "���ﲻ����Ŷ��" << endl;
	}
	return -1;//�ڸ��ļ�����û�и��ļ���Ŀ¼
}

void FileSystem::postInode(inode* i) {
	if (!FILE) {
		cerr << "postInode()����ʱδ���ļ�����ֹ��" << endl;
		//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//�򿪴���
	}
	FILE.seekp(S.inodePos + i->idx * sizeof(inode), ios::beg);
	FILE.write((char*)i, sizeof(inode));
	FILE.flush();
}

void FileSystem::postDirItem(inode* parentInode, inode* insertInode) {
	//������һ����ַ��û�����жϲ�����
	if (parentInode->size > 70) {
		cerr << "dirInit()ʧ�ܣ���Ŀ¼������" << endl;
		return;
	}
	DirectoryItem* newDirItem = new DirectoryItem;//����һ���µ�Ŀ¼���������������Ϣ
	newDirItem->inodeIdx = insertInode->idx;
	strcpy_s(newDirItem->name, 136, insertInode->name);
	newDirItem->type = insertInode->type;
	

	int blockNum = parentInode->size / 7;//д�ڵڼ������ݿ���
	int deviation = parentInode->size % 7;//д�����ݿ����һ����
	FILE.seekp(parentInode->dataBlock[blockNum] * S.blockSize + deviation * sizeof(DirectoryItem), ios::beg);//��ָ���Ƶ���Ŀ¼���ŵ�λ��
	FILE.write((char*)newDirItem, sizeof(DirectoryItem));//д�������
	parentInode->size++;//Ŀ¼�����Ŀ��1

	GetLocalTime(&(parentInode->modiTime));//���ĸ�Ŀ¼���޸�ʱ��
	postInode(parentInode);//�Ѹ�Ŀ¼inodeҲд�ش����У�ֱ��ͬ��
	FILE.flush();
	return;
}

void FileSystem::dirInit(inode* parentInode, string name) {
	//������û�������ж�Ŀ¼�Ƿ���
	if (!FILE) {
		cerr << "dirInit()����ǰû�д��ļ�����ֹ��" << endl;
		//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//�򿪴���
		//return;
	}
	if (parentInode->size > 70) {
		cerr << "dirInit()ʧ�ܣ���Ŀ¼������" << endl;
		return;
	}
	int newDirInodeIdx=RequestI();//Ϊ��Ŀ¼����һ��i�ڵ�
	inode* newDirInode = getInode(newDirInodeIdx);//���������inodeȡ������д������Ϣ
	//-------------�޸���inode����Ϣ---------------
	newDirInode->idx = newDirInodeIdx;//�����i�ڵ��
	newDirInode->parentIdx=parentInode->idx;//��Ŀ¼��inode���
	newDirInode->linkNum=1;//ָ���inode���ļ���
	newDirInode->uid=1;//�����û�id
	newDirInode->gid=10000;//������id
	newDirInode->mod=777;//����ģʽ
	strcpy_s(newDirInode->name,136,name.c_str());//�ļ���
	newDirInode->type='0';//�ļ�����  0��ʾĿ¼  1��ʾ�ļ�
	GetSystemTime(&(newDirInode->creatTime));//����ʱ��  16B
	newDirInode->modiTime=newDirInode->creatTime;//���һ���޸�ʱ��
	newDirInode->size=0;//�ļ���С ��λB
	RequestD(newDirInode->dataBlock,10);//14�����ݿ�ĵ�ַ����4����һ��ҳ���ļ��������1MB+10KB
	//----------------------------------------------
	postInode(newDirInode);//������õ�inodeд�ش���
	postDirItem(parentInode, newDirInode);//Ϊ��Ŀ¼��Ŀ¼��
}


void FileSystem::save() {
	if (!FILE) {
		cerr << "save()����ʱδ���ļ�����ֹ��" << endl;
		//FILE.open("VirtualDisk", ios::binary | ios::out | ios::in);//�򿪴���
	}
	FILE.seekp(0, ios::beg);//д������
	FILE.write((char*)&S, sizeof(S));
	FILE.seekp(S.inodeBMapPos, ios::beg);//дinodeλͼ
	FILE.write((char*)&inodeBMap, sizeof(inodeBMap));
	FILE.seekp(S.blockBMapPos, ios::beg);//дblockλͼ
	FILE.write((char*)&blockBMap, sizeof(blockBMap));
	FILE.flush();
}


void FileSystem::print() {
	S.print();
	curInode->print();
	cout << "��ǰ����·����" << curPath << endl;
}