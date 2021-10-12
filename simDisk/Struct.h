#pragma once
#ifndef _STRUCT_H_
#define _STRUCT_H_
#include<Windows.h>
#include<iostream>
using namespace std;
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
	void print() {
		cout << "inode���:" << idx << endl;
		cout << "��Ŀ¼��inode���:" << parentIdx << endl;
		cout << "ָ���inode���ļ���:" << linkNum << endl;
		cout << "�����û�id:" << uid << endl;
		cout << "������id:" << gid << endl;
		cout << "����ģʽ:" << mod << endl;
		cout << "�ļ���:" << name << endl;
		cout << "�ļ����� :" << type << endl;
		cout << "����ʱ��:"; printTime(creatTime);
		cout << "���һ���޸�ʱ��:"; printTime(modiTime);
		cout << "�ļ���С:" << size <<"B" << endl;
		cout << "���ݿ�ĵ�ַ:";
		for (int i = 0; i < 14; ++i) {
			if (dataBlock[i] > 0)
				cout << dataBlock[i] << ' ';
		}
		cout << endl;
	}
};

struct SuperBlock {
	int diskSize;//������������λB  100*1024*1024B
	int blockSize;//���̿���������λB 1024B
	int blockNum;//���̿����� diskSize/bolckSize 102400��
	int blockUsedNum;//��ʹ�õĴ��̿���
	int inodeNum;//i�ڵ�����
	int inodeUsedNum;//��ʹ�õ�i�ڵ���
	int inodeBMapPos;//i�ڵ�λͼ��ַ,���ڸ��ֽڿ�ʼ����1������i�ڵ�λͼ�������ɴ��8192���ļ����ļ���
	int inodePos;//i�ڵ���,�ڼ����ֽڿ�ʼ��i�����
	int blockBMapPos;//��λͼ��ַ,���ڼ����ֽڿ�ʼ����13(12.5)�����������ݿ�����λͼ  
	int blockPos;//���ݿ��ַ�����ڼ����ֽڿ�ʼ
	void print() {
		cout << "��������:" << diskSize/1024/1024<<"MB" << endl;
		cout << "���̿�����:" << blockNum<<"��" << endl;
		cout << "��ʹ�õĴ��̿���:" << blockUsedNum<<"��" << endl;
		cout << "i�ڵ�����:" << inodeNum << endl;
		cout << "��ʹ�õ�i�ڵ���:" << inodeUsedNum << endl;
		cout << "i�ڵ�λͼ��ַ:" << inodeBMapPos << endl;
		cout << "i�ڵ���:" << inodePos << endl;
		cout << "��λͼ��ַ:" << blockBMapPos << endl;
		cout << "���ݿ��ַ:" << blockPos << endl;
	}
};
struct DirectoryItem {//Ŀ¼��
	int inodeIdx;//i�ڵ��
	char name[136];//�ļ���
	char type;//�ļ�����
	void print() {
		cout << "i�ڵ�ţ�" << inodeIdx << "  ";
		if (type == '0') {
			cout << "Ŀ¼����" << name << "  ";
		}
		else if (type == '1') {
			cout << "�ļ�����" << name << "  ";
		}
		cout << endl;
	}
};

#endif