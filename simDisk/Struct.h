#pragma once
#ifndef _STRUCT_H_
#define _STRUCT_H_
#include<Windows.h>
struct inode {//256B
	int idx;//inode序号
	int parentIdx;//父目录的inode序号
	int linkNum;//指向该inode的文件数
	int uid;//所属用户id
	int gid;//所属组id
	int mod;//保护模式
	char name[136];//文件名
	char type;//文件类型  0表示目录  1表示文件
	SYSTEMTIME creatTime;//创建时间  16B
	SYSTEMTIME modiTime;//最后一次修改时间
	int size;//文件大小 单位B
	int dataBlock[14];//14个数据块的地址，后4块是一级页表，文件最大内容1MB+10KB
};

struct SuperBlock {
	int diskSize;//磁盘容量，单位B  100*1024*1024B
	int blockSize;//磁盘块容量，单位B 1024B
	int blockNum;//磁盘块总数 diskSize/bolckSize 102400块
	int blockUsedNum;//已使用的磁盘块数
	int inodeNum;//i节点总数
	int inodeUsedNum;//已使用的i节点数
	int inodeBMapPos;//i节点位图地址,即第个字节开始，用1块来做i节点位图，即共可存放8192个文件或文件夹
	int inodePos;//i节点区,第几个字节开始是i结点区
	int blockBMapPos;//块位图地址,即第几个字节开始，用13(12.5)个连续的数据块做块位图  
	int blockPos;//数据块地址，即第几个字节开始
};
struct DirectoryItem {//目录项

};

#endif