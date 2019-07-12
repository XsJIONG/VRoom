/*
	Copyright(c) 2019 XsJIONG.
	All Rights Reserved.

	编译提示：
	Dev-Cpp：请在顶部工具->编译选项中选择“编译时加入如下命令”并输入-lwsock32 -std=c++11
	其它编译器请加入-lwsock32 (若不能编译则也应加入-std=c++11）

	Compiling Notice:
	For Dev-Cpp users, please add '-lwsock32 -std=c++11' to compiling options.
	For other IDE users, please add '-lwsock32' to compiling options. If you still fail to compile, please add '-std=c++11'.
*/

// 程序语言，true为英文，false为中文 
// The language program will use, true for English and false for Chinese
const bool ENGLISH = false;





// 主要代码
// Main Code
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <iostream>
#include <string>
#include <sstream>
#include <mmsystem.h>
#include <cwctype>
#include <conio.h>
#define THREAD DWORD WINAPI
#define BufSize 1024
using namespace std;

const char* _L[2][20]={
{"版本 V1.0","错误代码：%d","{系统} %s 加入了房间","[%s] %s","{系统} %s 重命名为 %s","{系统} 语法错误，正确语法为\"rename [字符串]\"","{系统} 未知命令","=========================\n= Welcome to VRoom V1.0 =\n=========================\n用户名：%s 房间ID：%d\n按 [H] 键获取帮助","按 [Enter] 来编辑信息并发送\n按 [/] 执行命令\n按 [Esc] 退出程序\n\n命令列表\nrename [字符串] - 更改名字\nconfig - 显示用户名和房间ID\ncls - 清空屏幕\nexit - 退出当前房间","初始化中...","初始化失败！","创建套接字失败！","绑定套接字失败！","欢迎来到VRoom！","用户名：","用户名：%s","房间ID：","房间ID：%d","title VRoom - By Xs.JIONG","{系统} 用户名：%s，房间ID：%d"},
{"Version V1.0","Error Code:%d","{System} %s joined the room","[%s] %s","{System} %s has renamed to %s","{System} syntax error, expected \"rename [string]\"","{System} Undefined command","=========================\n= Welcome to VRoom V1.0 =\n=========================\nUserName:%s RoomID:%d\nPress [H] for help","Press [Enter] to edit a message and send it\nPress [/] to execute a command\nPress [Esc] to exit program\n\nCommand List\nrename [string] - change your name\nconfig - display the username and room id\ncls - clear the screen\nexit - exit current room","Initializing...","Failed to initialize!","Failed to create socket!","Failed to bind socket!","Welcome to VRoom!","UserName:","UserName:%s","RoomID:","RoomID:%d","title VRoom - By Xs.JIONG","{System} UserName:%s, RoomID:%d"}
};
const char** L=_L[ENGLISH];
const int SYSTEM_COLOR = FOREGROUND_BLUE;
const int NORMAL_COLOR = 0;

const int PORT=8898;
namespace Type {
	const char JOIN=1;
	const char MSG=2;
	const char RENAME=3;
};
namespace Global {
	char* UserName;
	short UserLength;
	string Content;
	WSADATA WData;
	SOCKET UDPSocket;
	HWND ConsoleWindow;
	SOCKADDR_IN RecvAddr,SendAddr;
	char MsgBuf[BufSize];
	int AddrSize;
	int Port;
	void sendR(string data) {
		char* p=(char*) data.data();
		sendR(p);
	}
	void sendR(char* data) {
		sendto(Global::UDPSocket, data, strlen(data)+1, 0, (sockaddr*)&Global::SendAddr, sizeof(Global::SendAddr));
	}
	void sendR(char* data, int len) {
		sendto(Global::UDPSocket, data, len, 0, (sockaddr*)&Global::SendAddr, sizeof(Global::SendAddr));
	}
};
namespace UI {
	COORD coord;
	HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO BufferInfo;
	string ReadTmp;
	inline void setColor(int c) {
		SetConsoleTextAttribute(ConsoleHandle,FOREGROUND_INTENSITY|c);
	}
	inline void moveCursor(int x, int y) {
		coord.X=x;
		coord.Y=y;
		SetConsoleCursorPosition(ConsoleHandle, coord);
	}
	inline void cls() {
		system("cls");
	}
	inline void pauseNul() {
		system("pause>nul");
	}
	string& inputString() {
		cls();
		system("color f0");
		ReadTmp.clear();
		getline(cin,ReadTmp);
		system("color 0f");
		cls();
		cout<<Global::Content;
		return ReadTmp;
	}
	inline int getCursorX() {
		GetConsoleScreenBufferInfo(ConsoleHandle, &BufferInfo);
		return BufferInfo.dwCursorPosition.X;
	}
	inline int getCursorY() {
		GetConsoleScreenBufferInfo(ConsoleHandle, &BufferInfo);
		return BufferInfo.dwCursorPosition.Y;
	}
	void print(const char *a) {
		cout<<a;
		Global::Content+=a;
	}
	void print(string a) {
		cout<<a;
		Global::Content+=a;
	}
	void printChar(char a) {
		putchar(a);
		Global::Content+=a;
	}
	CONSOLE_SCREEN_BUFFER_INFO _Buffer;
	short CWidth=80,CHeight=10;
	void setCenterShow() {
		static long scrWidth,scrHeight;
		static RECT rect;
		scrWidth=GetSystemMetrics(SM_CXSCREEN);
		scrHeight=GetSystemMetrics(SM_CYSCREEN);
		GetWindowRect(Global::ConsoleWindow,&rect);
		SetWindowPos(Global::ConsoleWindow,HWND_TOPMOST,(scrWidth-rect.right+rect.left)/2,(scrHeight-rect.bottom+rect.top)/2,rect.right-rect.left,rect.bottom-rect.top,SWP_SHOWWINDOW);
	}
	char _Tmp[2048];
	template <class...T>
	void printCenter(const char* f, T...arg) {
		sprintf(_Tmp,f,arg...);
		for (int i=(CWidth-strlen(_Tmp))>>1;i>0;i--) printChar(' ');
		print(_Tmp);
		printChar('\n');
	}
	template <class...T>
	void printCenterM(const char* f, T...arg) {
		sprintf(_Tmp,f,arg...);
		int st=0,i,j;
		while (1) {
			for (i=st;_Tmp[i]!='\n'&&_Tmp[i]!='\0';i++);
			for (j=(CWidth-i+st)>>1;j>0;j--) printChar(' ');
			if (_Tmp[i]=='\0') {
				print(_Tmp+st);
				printChar('\n');
				break;
			}
			_Tmp[i]='\0';
			print(_Tmp+st);
			printChar('\n');
			st=i+1;
		}
	}
	template <class...T>
	void printC(const char* f, T...arg) {
		sprintf(_Tmp,f,arg...);
		print(_Tmp);
		printChar('\n');
	}
	void setConsoleSize(short a, short b) {
		static char tmp[30];
		sprintf(tmp,"mode con cols=%d lines=%d",a,b);
		system(tmp);
		CWidth=a,CHeight=b;
	}
	string ReadPassword() {
		static string s;
		char ch;
		while ((ch = _getch()) != '\r') {
			if(ch == '\b') {
				if (!s.length()) continue;
				printf("\b \b");
				s.erase(s.end()-1);
			} else s+=ch,putchar('*');
		}
		return "";
	}
	void ReportError() {
		printC(L[1],WSAGetLastError());
		pauseNul();
		exit(0);
	}
	bool Running=0;
	WNDCLASSEX WC;
	inline void ParseCmd() {
		char type=Global::MsgBuf[0];
		char* con=Global::MsgBuf+1;
		switch (type) {
			case Type::JOIN: {
				printC(L[2],con);
				break;
			}
			case Type::MSG: {
				static short s;
				memcpy((char*)&s,con,2);
				con[s+2]='\0';
				printC(L[3],con+2,con+s+3);
				break;
			}
			case Type::RENAME: {
				short s;
				memcpy((char*)&s,con,2);
				con[s+2]='\0';
				printC(L[4],con+2,con+s+3);
				break;
			}
		}
	}
	inline void Split() {
		for (int i=0; i<CWidth; i++) putchar('=');
	}
	THREAD ResultThread(LPVOID para) {
		int len;
		while (Running) {
			fill(Global::MsgBuf,Global::MsgBuf+strlen(Global::MsgBuf),'\0');
			len=recvfrom(Global::UDPSocket,Global::MsgBuf,BufSize,0,(SOCKADDR*)&Global::RecvAddr,&Global::AddrSize);
			if (len>0) {
				Global::MsgBuf[len]='\0';
				ParseCmd();
			}
		}
	}
	void sendMsg(string msg) {
		int len=Global::UserLength+msg.length()+5;
		char* i=new char[len];
		i[0]=Type::MSG;
		memcpy(i+1,(char*)&Global::UserLength,2);
		memcpy(i+3,Global::UserName,Global::UserLength);
		i[Global::UserLength+3]=' ';
		memcpy(i+Global::UserLength+4,msg.data(),msg.length()+1);
		Global::sendR(i,len);
		delete[] i;
	}
	void CSplit() {
		for (int i=0; i<CWidth; i++) printChar('=');
	}
	void Main();
	void Disconnect() {
		Global::Content.clear();
		closesocket(Global::UDPSocket);
		UI::Main();
	}
	inline void ExecCmd(const char* s) {
		if (!memcmp(s,"exit",4)) {
			UI::Disconnect();
			return;
		} else if (!memcmp(s,"rename",6)) {
			char *ns=(char*) s+6;
			int len=strlen(ns);
			int i=0;
			while (ns[i]==' '&&i<len) i++;
			if (i>=len-1) {
				printC(L[5]);
				return;
			}
			int leng=Global::UserLength+len-i+5;
			char *msg=new char[leng];
			msg[0]=Type::RENAME;
			memcpy(msg+1,(char*)&Global::UserLength,2);
			memcpy(msg+3,Global::UserName,Global::UserLength);
			msg[Global::UserLength+3]=' ';
			memcpy(msg+Global::UserLength+4,ns+i,len-i+1);
			Global::sendR(msg,leng);
			memcpy(Global::UserName,ns+i,len-i);
			Global::UserLength=len-i;
			delete[] msg;
			return;
		} else if (!memcmp(s,"cls",3)) {
			Global::Content.clear();
			cls();
			return;
		} else if (!memcmp(s,"exit",4)) {
			Disconnect();
			return;
		} else if (!memcmp(s,"config",6)) {
			printC(L[19],Global::UserName,Global::Port);
			return;
		}
		printC(L[6]);
	}
	void Chat() {
		cls();
		Global::Content.clear();
		setConsoleSize(70,20);
		setCenterShow();
		printCenterM(L[7],Global::UserName,Global::Port);
		CSplit();
		char c[Global::UserLength+2];
		c[0]=Type::JOIN;
		memcpy(c+1,Global::UserName,Global::UserLength+1);
		Global::sendR(c,Global::UserLength+2);
		while (1) {
			switch (getch()) {
				case 13:
					sendMsg(inputString());
					break;
				case 27:
					exit(0);
					break;
				case '/':
					ExecCmd(inputString().c_str());
					break;
				case 'h':
					string back=Global::Content;
					cls();
					printCenterM(L[8]);
					pauseNul();
					cls();
					Global::Content=back;
					cout<<Global::Content;
					break;
			}
		}
	}
	void Connect() {
		printC(L[9]);
		if (WSAStartup(MAKEWORD(2,2),&Global::WData)!=0) {
			printC(L[10]);
			ReportError();
			return;
		}
		if ((Global::UDPSocket=socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET) {
			printC(L[11]);
			ReportError();
			WSACleanup();
			return;
		}
		Global::RecvAddr.sin_family=AF_INET;
		Global::RecvAddr.sin_port=htons(Global::Port);
		Global::RecvAddr.sin_addr.S_un.S_addr = INADDR_ANY;
		Global::SendAddr.sin_family = AF_INET;
		Global::SendAddr.sin_port = htons(Global::Port);
		Global::SendAddr.sin_addr.S_un.S_addr = INADDR_BROADCAST;
		Global::AddrSize=sizeof(SOCKADDR_IN);
		bool BroadOption=1;
		setsockopt(Global::UDPSocket,SOL_SOCKET,SO_BROADCAST,(char*)&BroadOption,sizeof(BroadOption));
		int ret = bind(Global::UDPSocket,(SOCKADDR*)&Global::RecvAddr,sizeof(SOCKADDR));
		if (ret==SOCKET_ERROR) {
			printC(L[12]);
			ReportError();
			WSACleanup();
			return;
		}
		Running=1;
		CreateThread(NULL,0,ResultThread,NULL,0,NULL);
		Chat();
	}
	int readInt() {
		char c=getchar();
		while (c<'0'||c>'9') c=getchar();
		int ret=0;
		do	{
			ret=(ret<<3)+(ret<<1)+c-'0';
			c=getchar();
		} while (c>='0'&&c<='9');
		return ret;
	}
	void Main() {
		cls();
		setConsoleSize(40,6);
		setCenterShow();
		printCenter(L[13]);
		printCenter(L[0]);
		Split();
		print(L[14]);
		string tmp;
		getline(cin,tmp);
		if (!(Global::UserLength=tmp.length())) {
			Main();
			return;
		}
		Global::UserName=(char*) tmp.data();
		UI::moveCursor(0,UI::getCursorY()-1);
		printCenter(L[15],Global::UserName);
		print(L[16]);
		Global::Port=readInt();
		if (!Global::Port) {
			Main();
			return;
		}
		UI::moveCursor(0,UI::getCursorY()-1);
		printCenter(L[17],Global::Port);
		Global::Content.clear();
		Connect();
	}
};
int main() {
	system(L[18]);
	Global::ConsoleWindow=GetForegroundWindow();

	UI::Main();
	return 0;
}
