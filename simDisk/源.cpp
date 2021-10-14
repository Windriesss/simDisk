#include<iostream>
#include"Struct.h"
#include"FileSystem.h"
#include<bitset>

using namespace std;

int main() {//≤‚ ‘¥˙¬Î
	FileSystem f;
	inode* t;
	t=f.inodeStack[0];
	f.pwd();

	//f.cd("/11111");
	//f.pwd();

	//f.cd("./asc");
	//f.pwd();

	//f.cd("..");
	//f.pwd();
	

	f.dir("/");

	f.format();
	f.md("/test001/tt");
	f.dir(".");



}