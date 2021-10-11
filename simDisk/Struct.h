#pragma once
#ifndef _STRUCT_H_
#define _STRUCT_H_
#include<Windows.h>
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
};
struct DirectoryItem {//Ŀ¼��

};

#endif