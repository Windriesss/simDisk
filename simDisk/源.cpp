#include<iostream>
#include"Struct.h"
#include"FileSystem.h"
#include<bitset>

using namespace std;

int main() {//���Դ���
	FileSystem f;
	string cmd;
	f.help();
	cout << f.getWd() << ':';
	while (getline(cin,cmd)) {
		f.getcmd(cmd);
		cout << f.getWd() << ':';
	}
}