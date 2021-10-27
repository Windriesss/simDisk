#pragma once
#ifndef _STRUCT_H_
#define _STRUCT_H_
#include<Windows.h>
#include<iostream>
#include<bitset>
using namespace std;

struct DirectoryItem {//Ŀ¼��
	int inodeIdx;//i�ڵ��
	char name[136];//�ļ���
	char type;//�ļ�����
	void print() {
		cout << "i�ڵ�ţ�" << inodeIdx << "  " << "�ļ�����" << name << "  ";
		cout << "�ļ����ͣ�" << (type == 0 ? "Ŀ¼" : "�ļ�") << endl;
	}
};
struct inode {//256B
	int idx;//inode���
	int parentIdx;//��Ŀ¼��inode���
	int linkNum;//ָ���inode���ļ���
	int uid;//�����û�id
	int gid;//������id
	int mod;//����ģʽ
	char name[136];//�ļ���
	char type;//�ļ�����  0��ʾĿ¼  1��ʾ�ļ�
	SYSTEMTIME creatTime;//����ʱ��  16B
	SYSTEMTIME modiTime;//���һ���޸�ʱ��
	int size;//�ļ���С ��λB
	int dataBlock[14];//14�����ݿ�ĵ�ַ����4����һ��ҳ���ļ��������1MB+10KB
	void printTime(SYSTEMTIME st) {
		printf("%u-%u-%u %u:%u:%u\n", st.wYear, st.wMonth, st.wDay,st.wHour, st.wMinute, st.wSecond);
	}
	string getModiTime() {
		string t("%u-%u-%u %u:%u:%u");
		char targetString[1024];
		// ��ʽ��������ȡ������Ҫ���ַ���
		snprintf(targetString, sizeof(targetString), t.c_str(), modiTime.wYear, modiTime.wMonth, modiTime.wDay,
			modiTime.wHour, modiTime.wMinute, modiTime.wSecond);
		t = targetString;
		return t;
	}
	int getSize() {
		if (type == '1') return size;
		if (type == '0') return size * sizeof(DirectoryItem);
		return -1;
	}
	string getType() {
		string ret;
		if (type == '0') ret = "d";
		else if (type == '1') ret = "-";
		return ret;
	}
};

struct SuperBlock {
	int diskSize;//������������λB  100*1024*1024B
	int blockSize;//���̿���������λB 1024B
	int blockNum;//���̿����� diskSize/bolckSize 102400��
	int inodeNum;//i�ڵ�����
	int inodeBMapPos;//i�ڵ�λͼ��ַ,���ڸ��ֽڿ�ʼ����1������i�ڵ�λͼ�������ɴ��8192���ļ����ļ���
	int inodePos;//i�ڵ���,�ڼ����ֽڿ�ʼ��i�����
	int blockBMapPos;//��λͼ��ַ,���ڼ����ֽڿ�ʼ����13(12.5)�����������ݿ�����λͼ  
	int blockPos;//���ݿ��ַ�����ڼ����ֽڿ�ʼ
};

struct logInfo {
	int uid;
};

struct File//Memory mapping file
{
	char cmd[1024 * 5];
	char ret[1024 * 10];
	int errorFlag;
};

struct AccessTable
{
	int readNum[8192];
	bitset<8192> writeFlag;
};

#endif