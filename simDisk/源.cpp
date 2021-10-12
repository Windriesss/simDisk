#include<iostream>
#include"Struct.h"
#include"FileSystem.h"
#include<bitset>

using namespace std;

int main() {//²âÊÔ´úÂë
	FileSystem f;
	inode* t;
	f.dir("/");
	f.md("/testmd");
	f.dir("/");
}