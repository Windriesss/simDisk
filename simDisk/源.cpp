#include<iostream>
#include"Struct.h"
#include<bitset>

using namespace std;

int main() {
	inode t;
	cout << "inode Size:" << sizeof(t) << endl;
	SuperBlock s;
	cout << "SuperBlock Size:" << sizeof(s) << endl;
	cout<<sizeof(bitset<1024>);
}