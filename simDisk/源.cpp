#include<iostream>
#include"Struct.h"
#include"FileSystem.h"
#include<bitset>

using namespace std;

int main() {//���Դ���
	cout << "\n\t������������������������������������������������������������������������������������������������������" << endl;
	cout << "\t��                201930340413                     ��" << endl;
	cout << "\t��                  19�ƿ�2��                      ��" << endl;
	cout << "\t��                   �߻���                        ��" << endl;
	cout << "\t������������������������������������������������������������������������������������������������������" << endl;

	FileSystem f;
	string cmd;
	f.help();
	cout << f.getWd() << ':';
	while (getline(cin,cmd)) {
		f.getcmd(cmd);
		cout << f.getWd() << ':';
	}
}