#include "FileSystem.h"
#include"Struct.h"
#include<bitset>
#include <fstream>
#include <cstring>
#include <iostream>
#include <windows.h>

using namespace std;

FileSystem::FileSystem() {
	inodeBMap.reset();
	blockBMap.reset();
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
		return;
	}
	load();//������״̬
}

void FileSystem::init() {
	cout << "���ڴ����ļ�ϵͳ ..." << endl;
	if (FILE) {
		FILE.close();//������˾͹رգ���һ�ַ�ʽ��
	}
	FILE.open("VirtualDisk", ios::binary|ios::out);//ֻд��ʽ�Żᴴ���µ��ļ�
	if (!FILE) {
		cerr << "�����������ʧ��!" << endl;
		return;
	}
	//------------��ʼ��������------------
	S.diskSize = 100 * 1024 * 1024;//������������λB  100*1024*1024B
	S.blockSize = 1024;//���̿���������λB 1024B
	S.blockNum = S.diskSize / S.blockSize;//���̿����� diskSize/bolckSize 102400��
	S.inodeNum = 8192;//i�ڵ�����
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
	FILE.close();
	FILE.open("VirtualDisk", ios::binary | ios::out|ios::in);
	return;
}

void FileSystem::load() {
	if (!FILE) {
		cerr << "load()" << "�򿪴���ʧ�ܣ�" << endl;
	}
	FILE.seekg(0, ios::beg);//��������
	FILE.read((char*)&S, sizeof(S));
	FILE.seekg(S.inodeBMapPos, ios::beg);//��inodeλͼ
	FILE.read((char*)&inodeBMap, sizeof(inodeBMap));
	FILE.seekg(S.blockBMapPos, ios::beg);//��blockλͼ
	FILE.read((char*)&blockBMap, sizeof(blockBMap));
	FILE.seekg(S.inodePos, ios::beg);//����Ŀ¼i���
	inodeStack.clear();//���inodeջ;
	inodeStack.push_back(getInode(0));//�Ѹ�Ŀ¼��i���ż��뵽·��i�ڵ�ջ��
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
			t[getNum++] = i;
		}
	}
	if (i == S.blockNum) {//�ڴ治��
		cerr << "RequestD()�������ݿ鲻�㣡" << endl;
		return -1;//�ڴ治�㣬���ش���
	}
	for (i = 0; i < n; ++i) {//ȷ�����㹻������ݿ��һ������
		blockBMap.set(t[i]);//������һλ
	}
	save();//д���ڴ棬ͬ������
	return 0;//��������
}

void FileSystem::delI(inode* delInode) {
	
	//��Ŀ¼ҲҪɾ��Ŀ¼�Ȼ�������һ�����λ
	inode* parentInode = getInode(delInode->parentIdx);//ȡ��Ŀ¼�ڵ�
	for (int i = 0; i < parentInode->size-1; ++i) {//��������һ��Ļ�����Ŀ¼��Ŀ¼������һ���൱�ڶ�����Ŀ¼��
		DirectoryItem* dirItem = getDirectorItem(parentInode, i);//����ȡ��Ŀ¼��
		if (strcmp(delInode->name, dirItem->name) == 0) {//�������Ҫ�ҵ�Ŀ¼��
			DirectoryItem* lastItem = getDirectorItem(parentInode, parentInode->size - 1);//ȡ�����һ��Ŀ¼��
			postDirItem(parentInode, lastItem, i);
		}
	}
	parentInode->size -= 1;//��Ŀ¼��Ŀ¼������һ
	postInode(parentInode);//�����˵�i�ڵ㣬��ʱ���

	delInode->linkNum -= 1;//��������һ
	if (delInode->linkNum != 0) {//���б���ļ�ָ�����inode
		postInode(delInode);//����i�ڵ����Ϣ
	}
	else {//���һ��ָ���inode���ļ�
		if (delInode->type == '0') {//����ļ��Ǹ��ļ��У�Ҫ����ɾ��
			for (int i = 0; i < delInode->size; ++i) {
				DirectoryItem* dirItem = getDirectorItem(delInode, i);//ȡ��Ŀ¼��
				delI(getInode(dirItem->inodeIdx));//ɾ��Ŀ¼����ָ�Ľ��
			}
		}
		inodeBMap.reset(delInode->idx);//�ͷŸ�i�ڵ�
		delD(delInode->dataBlock, 10);//�ͷŸ��ļ���ռ�ռ�
		save();//����i�ڵ�λͼ����Ϣ
	}

}

void FileSystem::delD(int blockIdx) {
	if (blockIdx > 0) {
		blockBMap.reset(blockIdx);//
		save();
	}

}

void FileSystem::delD(int* dataBlock, int n) {
	for (int i = 0; i < n; ++i) {
		if (dataBlock[i] > 0) {
			blockBMap.reset(dataBlock[i]);
		}
	}
	save();
}

inode* FileSystem::getInode(int idx) {//����i�ڵ�ŷ���i�ڵ��ָ��
	//�����������ǰ�����ȴ��ļ�
	if (!FILE) {
		cerr << "getInode()����ǰû�д��ļ�����ֹ��" << endl;
		return NULL;
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

inode* FileSystem::getFileInode(string path) {
	inode* fileInode = getInode(getIndex(path));
	if (fileInode == NULL) {
		cerr << "getFileInode()·�����Ҵ�����ֹ��" << endl;
		return NULL;
	}
	//�ж�inode�Ƿ����ļ�����
	if (fileInode->type != '1') {
		cerr << "��·����ָ���Ͳ����ļ����ͣ���ֹ��" << endl;
		return NULL;
	}
	return fileInode;
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
		updateInodeStack();//���¹���i�ڵ�ջ
		curInodeIdx = inodeStack.back()->idx;//����Ŀ¼�����һ��
		curInode = inodeStack.back();
		if (pathSplit[0] == "..") {
			if (inodeStack.size() <= 1) {
				cout << "getParentDirIndex()����û����һ��Ŀ¼����ֹ��" << endl;
			}
			curInodeIdx = curInode->parentIdx;//�ù���Ŀ¼����һ��
			curInode = inodeStack[inodeStack.size() - 2];
		}
	}
	else {//��������
		cerr << "getParentDirIndex()����·���������" << endl;
		return -1;//����
	}

	for (int i = 1; i < pathSplit.size()-1; ++i) {//��һ����ȡ
		curInodeIdx = dirFindByName(curInode, pathSplit[i]);//ȡ��һ��Ŀ¼��i�ڵ��
		if (curInodeIdx < 0) {
			cout << "getParentDirIndex()����!";
			cout << curInode->name << "��û������Ϊ" << pathSplit[i] << "���ļ���Ŀ¼" << endl;
			return -1;
		}
		else curInode = getInode(curInodeIdx);//ȡi�ڵ�
	}
	if (curInode->type != '0') {
		cerr << "getParentDirIndex()���ô��󣡸�·������һ�������ļ��У�" << endl;
		return -1;
	}
	return curInodeIdx;
}

int FileSystem::getIndex(string path) {
	if (path == "/") {//��Ŀ¼û�и�Ŀ¼�����ʺ�����Ĳ���
		return 0;
	}
	if (path == ".") {
		return inodeStack.back()->idx;
	}
	if (path == "..") {
		if (inodeStack.size() <= 1) {
			cerr << "getIndex()����û����һ��Ŀ¼��" << endl;
			return -1;
		}
		else return inodeStack[inodeStack.size() - 2]->idx;
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

string FileSystem::getMod(inode* fileNode) {
	return fileNode->getType() + modInt2String(fileNode->mod);
}

void FileSystem::updateInodeStack() {
	if (!FILE) {
		cerr << "updateInodeStack()����ǰû�д�FILE����ע�⣡" << endl;
		return;
	}
	int len = inodeStack.size();
	for (int i = 0; i < len; ++i) {
		FILE.seekg(S.inodePos + inodeStack[i]->idx * sizeof(inode), ios::beg);
		FILE.read((char*)inodeStack[i], sizeof(inode));
	}
	return;
}

string FileSystem::getWd() {
	string ret="/";
	int len = inodeStack.size();
	for (int i = 1; i < len; ++i) {
		ret += inodeStack[i]->name;
		if (i != len - 1) {
			ret += '/';
		}
	}
	return ret;
}

string FileSystem::getFileContent(inode* fileInode) {
	string ret;
	if (fileInode == NULL) {
		cerr << "getFileContent()�����˿սڵ㣡��ֹ��" << endl;
		return ret;
	}
	char buf[1025];
	int fileSize = fileInode->size;//�ļ���С
	int blockTotal = fileSize / S.blockSize;//��ռ���ݿ����
	int readedSize = 0;//�Ѿ�������
	for (int i = 0; i <= blockTotal; ++i) {
		FILE.seekg(fileInode->dataBlock[i] * S.blockSize, ios::beg);//��λ�����ݿ鿪ͷ
		int readCount = min(S.blockSize, fileSize - readedSize);
		FILE.read(buf, readCount);
		buf[readCount] = '\0';//�ֶ�����ַ���������־
		readedSize += S.blockSize;
		ret += buf;
	}
	return ret;
}

void FileSystem::checkHelp(int idx, bitset<8192>& newInodeBMap, bitset<100 * 1024>& newBlockBMap) {//�ݹ�check
	inode* fileInode = getInode(idx);
	if (newInodeBMap[fileInode->idx]) {//�Ѿ��жϹ���
		return;
	}
	newInodeBMap.set(fileInode->idx);
	for (int i = 0; i < 10; ++i) {
		if (newBlockBMap[fileInode->dataBlock[i]]) {//�Ѿ�������
			char buf[1024];
			FILE.seekg(fileInode->dataBlock[i] * S.blockSize, ios::beg);//�Ƶ�ָ�����ݿ⣬׼��������
			FILE.read(buf, S.blockSize);
			int* newBlockIdx=new int;
			RequestD(newBlockIdx, 1);
			FILE.seekp(*newBlockIdx * S.blockSize, ios::beg);
			fileInode->dataBlock[i] = *newBlockIdx;
			postInode(fileInode);
		}
		else {
			newBlockBMap.set(fileInode->dataBlock[i]);
		}
	}
	if (fileInode->type == '0') {//������ļ������ͣ������µ��ļ�Ҳȫ���ж�һ��
		for (int i = 0; i < fileInode->size; ++i) {
			DirectoryItem* dItem = getDirectorItem(fileInode, i);
			checkHelp(dItem->inodeIdx,newInodeBMap,newBlockBMap);
		}
	}
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
	return -1;//�ڸ��ļ�����û�и��ļ���Ŀ¼
}

void FileSystem::postInode(inode* i) {
	if (!FILE) {
		cerr << "postInode()����ʱδ���ļ�����ֹ��" << endl;
		return;
	}
	FILE.seekp(S.inodePos + i->idx * sizeof(inode), ios::beg);
	FILE.write((char*)i, sizeof(inode));
	FILE.flush();
}

void FileSystem::postDirItem(inode* parentInode, inode* insertInode) {
	//������һ����ַ��û�����жϲ�����
	if (parentInode->size > 70) {
		cerr << "postDirItem()ʧ�ܣ���Ŀ¼������" << endl;
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

void FileSystem::postDirItem(inode* parentInode, DirectoryItem* dirItem, int n) {
	if (parentInode->size > 70) {
		cerr << "postDirItem()ʧ�ܣ���Ŀ¼������" << endl;
		return;
	}
	if (n > parentInode->size) {
		cerr << "postDirItem()ʧ�ܣ�����Ŀ¼���С!" << endl;
		return;
	}
	int blockNum = n / 7;//д�ڵڼ������ݿ���
	int deviation = n % 7;//д�����ݿ����һ����
	FILE.seekp(parentInode->dataBlock[blockNum] * S.blockSize + deviation * sizeof(DirectoryItem), ios::beg);//��ָ���Ƶ���Ŀ¼���ŵ�λ��
	FILE.write((char*)dirItem, sizeof(DirectoryItem));//д�������

	GetLocalTime(&(parentInode->modiTime));//���ĸ�Ŀ¼���޸�ʱ��
	postInode(parentInode);//�Ѹ�Ŀ¼inodeҲд�ش����У�ֱ��ͬ��
	FILE.flush();
	return;
}



void FileSystem::dirInit(inode* parentInode, string name) {
	//������û�������ж�Ŀ¼�Ƿ���
	if (!FILE) {
		cerr << "dirInit()����ǰû�д��ļ�����ֹ��" << endl;
		return;
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
	GetLocalTime(&(newDirInode->creatTime));//����ʱ��  16B
	newDirInode->modiTime=newDirInode->creatTime;//���һ���޸�ʱ��
	newDirInode->size=0;//�ļ���С ��λB
	RequestD(newDirInode->dataBlock,10);//14�����ݿ�ĵ�ַ����4����һ��ҳ���ļ��������1MB+10KB
	//----------------------------------------------
	postInode(newDirInode);//������õ�inodeд�ش���
	postDirItem(parentInode, newDirInode);//Ϊ��Ŀ¼��Ŀ¼��
}


int FileSystem::fwriteHelp(inode* fileInode, string content, bool appFlag = 1) {
	//�ж�д����С�Ƿ���(�Ƿ�׷��)
	int contentSize = content.size();
	int preSize = fileInode->size;
	if (appFlag) {//�ж��Ƿ�Ϊ׷��ģʽ
		if (contentSize + preSize > 10 * 1024) {//�ж��Ƿ���
			cerr << "׷�Ӻ��ļ���С�������ƣ�ֹͣд�룡" << endl;
			return -1;
		}
		else {
			fileInode->size += contentSize;
		}
	}
	else {
		if (contentSize > 10 * 1024) {
			cerr << "д���ļ������ݳ��ޣ�ֹͣд�룡" << endl;
			return -1;
		}
		else fileInode->size = contentSize;
	}
	//������ַд�뻹û���
	//���ļ���д����
	int blockNum = 0, deviation = 0;//д����̿�ź�ƫ��ֵ
	if (appFlag) {//׷��ģʽ�Ļ�Ҫ��ƫ��
		blockNum = preSize / S.blockSize;
		deviation = preSize % S.blockSize;
	}
	FILE.seekp(fileInode->dataBlock[blockNum++] * S.blockSize + deviation, ios::beg);
	int insertSize = S.blockSize - deviation;//�˴�д���ֵ��������һ��
	int insertedNum = 0;//�Ѿ�д���ֵ
	const char* fileContent = content.c_str();
	for (; blockNum < 10 && insertedNum < contentSize;) {
		FILE.write(fileContent + insertedNum, insertSize);//������д���ļ���
		insertedNum += insertSize;//д�������׷�ӵ���д���¼ֵ��
		insertSize = min(contentSize - insertedNum, S.blockSize);//����1024��ȫ��д�룬����1024��ֻд1024
		FILE.seekp(fileInode->dataBlock[blockNum++] * S.blockSize, ios::beg);//�ƶ�дָ�룬��֤��һ��д���λ������ȷ��
	}
	//���i�ڵ�
	postInode(fileInode);
	return 0;
}

void FileSystem::makeTrouble() {
	srand(int(time(0)));
	cout << "������i�ڵ�ŷ��������:";
	for (int i = 0; i < 5; ++i) {
		int t = rand() % 8000 + 1000;
		cout << t << ' ';
		inodeBMap.set(t);
	}
	cout << endl;
	cout << "���������ݿ���������:";
	for (int i = 0; i < 5; ++i) {
		int t = rand() % 80000 +20000;
		cout << t << ' ';
		blockBMap.set(t);
	}
	cout << endl;
	return;

}

void FileSystem::save() {
	if (!FILE) {
		cerr << "save()����ʱδ���ļ�����ֹ��" << endl;
	}
	FILE.seekp(0, ios::beg);//д������
	FILE.write((char*)&S, sizeof(S));
	FILE.seekp(S.inodeBMapPos, ios::beg);//дinodeλͼ
	FILE.write((char*)&inodeBMap, sizeof(inodeBMap));
	FILE.seekp(S.blockBMapPos, ios::beg);//дblockλͼ
	FILE.write((char*)&blockBMap, sizeof(blockBMap));
	FILE.flush();
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
	for (int i = ret.size() - 1; i > 0; --i) {
		if (ret[i].empty()) ret.pop_back();
		else break;
	}
	return ret;
}

string FileSystem::modInt2String(int mod) {
	int other, group, owner;
	other = mod % 10; mod /= 10;
	group = mod % 10; mod /= 10;
	owner = mod;
	string arr[8] = { "---","--x","-w-","-wx","r--","r-x","rw-","rwx" };
	return arr[owner] + arr[group] + arr[other];
}

void FileSystem::getcmd(string cmd) {
	if (cmd == "") return;
	vector<string> $ = split(cmd, " ");
	if ($[0] == "format") {
		format();
	}
	else if ($[0] == "info") {
		info();
	}
	else if ($[0] == "md") {
		if ($.size() < 2) {
			cout << "����Ĳ������㣬�����룺md path" << endl;
			return;
		}
		md($[1]);
	}
	else if ($[0] == "dir") {
		if ($.size() < 2) {
			dir(".");
			return;
		}
		dir($[1]);
	}
	else if ($[0] == "cd") {
		if ($.size() < 2) {
			cout << "����Ĳ������㣬�����룺cmd path" << endl;
			return;
		}
		cd($[1]);
	}
	else if ($[0] == "pwd") {
		pwd();
	}
	else if ($[0] == "rd") {
		if ($.size() < 2) {
			cout << "����Ĳ������㣬�����룺rd path" << endl;
			return;
		}
		rd($[1]);
	}
	else if ($[0] == "newfile") {
		if ($.size() < 2) {
			cout << "����Ĳ������㣬�����룺newfile path" << endl;
			return;
		}
		newfile($[1]);
	}
	else if ($[0] == "fwrite") {
		if ($.size() < 2) {
			cout << "����Ĳ������㣬�����룺fwrite path appFlag" << endl;
			return;
		}
		fwrite($[1], atoi($[2].c_str()));
	}
	else if ($[0] == "cat") {
		if ($.size() < 2) {
			cout << "����Ĳ������㣬�����룺cat path" << endl;
			return;
		}
		cat($[1]);
	}
	else if ($[0] == "del") {
		if ($.size() < 2) {
			cout << "����Ĳ������㣬�����룺del path" << endl;
			return;
		}
		del($[1]);
	}
	else if ($[0] == "copy") {
		if ($.size() < 3) {
			cout << "����Ĳ������㣬�����룺copy srcPath dstPath" << endl;
			return;
		}
		copy($[1], $[2]);
	}
	else if ($[0] == "help") {
		help();
	}
	else if ($[0] == "check") {
		check();
	}
	else if ($[0] == "makeTrouble") {
		makeTrouble();
	}
	else {
		cout << "δʶ���������������룡" << endl;
	}
}

string FileSystem::cmpPath(string path) {
	if (path == "")
		return ".";
	if (path[0] == '.' || path[0] == '/')
		return path;
	return "./" + path;
}

string FileSystem::UTF8ToGB(const char* str){//��host�����ı��ļ��������ʽ����
	string result;
	WCHAR* strSrc;
	LPSTR szRes;

	int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	strSrc = new WCHAR[i + 1];
	MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

	i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
	szRes = new CHAR[i + 1];
	WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

	result = szRes;
	delete[]strSrc;
	delete[]szRes;

	return result;
}