#include<iostream>
#include"Struct.h"
#include"FileSystem.h"
#include<bitset>

using namespace std;

int main() {//测试代码
	cout << "\n\t┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓" << endl;
	cout << "\t┃                201930340413                     ┃" << endl;
	cout << "\t┃                  19计科2班                      ┃" << endl;
	cout << "\t┃                   高怀基                        ┃" << endl;
	cout << "\t┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛" << endl;

	FileSystem f;
	string cmd;
	f.help();
	cout << f.getWd() << ':';
	while (getline(cin,cmd)) {
		f.getcmd(cmd);
		cout << f.getWd() << ':';
	}
}