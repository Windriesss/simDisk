#include<iostream>
#include"Struct.h"
#include"FileSystem.h"
#include<bitset>

using namespace std;

int main() {//²âÊÔ´úÂë
	FileSystem f;
	//f.print();
	//cout << sizeof(DirectoryItem);
	inode* t;
	t = f.getInode(0);
	t->print();
	/*inode t;
	cout << "inode Size:" << sizeof(t) << endl;
	SuperBlock s;
	cout << "SuperBlock Size:" << sizeof(s) << endl;
	cout<<sizeof(bitset<1024>);*/
}