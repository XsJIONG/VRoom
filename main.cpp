#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
using namespace std;

string readPassword() {
	string s;
	char ch;
	while ((ch = _getch()) != '\r') {
		if(ch == '\b') {
			if (!s.length()) continue;
			printf("\b \b");
			s.erase(s.end()-1);
		} else s+=ch,cout << '*';
	}
}
int main() {
	system("title VRoom - By Xs.JIONG");
	char pw[100];
	int i;
	char ch;

	return 0;
}
