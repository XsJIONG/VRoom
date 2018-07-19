#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <iostream>
#include <string>
#define THREAD DWORD WINAPI
#define BufSize 1024
using namespace std;

const char* VERSION="1.0";

const int PORT=8898;
namespace Type {
	const int JOIN=0;
	const int MSG=1;
};
namespace Global {
	char* UserName;
	short UserLength;
	WSADATA WData;
	SOCKET UDPSocket;
	SOCKADDR_IN RecvAddr,SendAddr;
	char MsgBuf[BufSize];
	int AddrSize;
	int Port;
	void sendR(string data) {
		char* p=(char*) data.data();
		sendR(data);
	}
	void sendR(char* data) {
		sendto(Global::UDPSocket, data, strlen(data), 0, (sockaddr*)&Global::SendAddr, sizeof(Global::SendAddr));
	}
};
namespace UI {
HANDLE CH;
void cls() {
	system("cls");
}
void moveCursor(short x, short y) {
	COORD _Coord;
	_Coord.X=x;
	_Coord.Y=y;
	SetConsoleCursorPosition(CH,_Coord);
}
void moveCursor(COORD c) {
	SetConsoleCursorPosition(CH,c);
}
CONSOLE_SCREEN_BUFFER_INFO _Buffer;
short CWidth=80,CHeight=10;
void setCenterShow(){
	HWND h=GetConsoleWindow();
	long scrWidth,scrHeight;
	RECT rect;
	scrWidth=GetSystemMetrics(SM_CXSCREEN);
	scrHeight=GetSystemMetrics(SM_CYSCREEN);
	GetWindowRect(h,&rect);
	SetWindowPos(h,HWND_TOPMOST,(scrWidth-rect.right)/2,(scrHeight-rect.bottom)/2,rect.right-rect.left,rect.bottom-rect.top,SWP_SHOWWINDOW);
}
void setConsoleSize(short a, short b) {
	char tmp[30];
	sprintf(tmp,"mode con cols=%d lines=%d",a,b);
	system(tmp);
	CWidth=a,CHeight=b;
}
inline void InitConsole() {
	CH=GetStdHandle(STD_OUTPUT_HANDLE);
}
COORD getCursor() {
	GetConsoleScreenBufferInfo(CH, &_Buffer);
	return _Buffer.dwCursorPosition;
}
short getCursorX() {
	GetConsoleScreenBufferInfo(CH, &_Buffer);
	return _Buffer.dwCursorPosition.X;
}
short getCursorY() {
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &_Buffer);
	return _Buffer.dwCursorPosition.Y;
}
char _Tmp[1024];
template <class...T>
void CenterPrint(const char* f, T...arg) {
	sprintf(_Tmp,f,arg...);
	int ss=(CWidth-strlen(_Tmp))/2;
	for (int i=0;i<ss;i++) putchar(' ');
	printf("%s",_Tmp);
	putchar('\n');
}
string ReadPassword() {
	string s;
	char ch;
	while ((ch = _getch()) != '\r') {
		if(ch == '\b') {
			if (!s.length()) continue;
			printf("\b \b");
			s.erase(s.end()-1);
		} else s+=ch,putchar('*');
	}
}
void ReportError() {
	printf("Error Code:%d\n",WSAGetLastError());
	system("pause>nul");
	exit(0);
}
bool Running=0;
inline void ParseCmd() {
	char type=Global::MsgBuf[0];
	char* con=Global::MsgBuf+1;
	switch (type) {
		case Type::JOIN:{
			printf("[System] %s joined the room\n",con);
			break;
		}
		case Type::MSG:{
			system("title hqduihqwudhqw");
			int i=0;
			while (con[++i]!='\0');
            printf("[%s] %s",con,con+i);
            break;
		}
	}
}
void Split() {
	for (int i=0;i<CWidth;i++) putchar('=');
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
	char* i=new char[Global::UserLength+msg.length()+3];
	memcpy(i+1,Global::UserName,Global::UserLength+1);
	memcpy(i+Global::UserLength+2,msg.data(),msg.length()+1);
	i[0]=Type::MSG;
	Global::sendR(i);
}
void Chat() {
	cls();
	setConsoleSize(70,20);
	SetConsoleScreenBufferSize(CH,(COORD){70,200});
	setCenterShow();
	CenterPrint("====================");
	CenterPrint("| Welcome to VRoom |");
	CenterPrint("====================");
	CenterPrint("UserName:%s Port:%d",Global::UserName,Global::Port);
	Split();
	printf("[System] %s joined the room\n",Global::UserName);
	int c=CHeight-getCursorY();
	for (int i=0;i<c-1;i++) putchar('\n');
	moveCursor(0,CHeight-1);
	getchar();
	string s;
	char ch;
	while (true) {
		while ((ch = _getch()) != '\r') {
			if(ch == '\b') {
				if (!s.length()) continue;
				printf("\b \b");
				s.erase(s.end()-1);
			} else s+=ch,putchar(ch);
		}
		moveCursor(0,getCursorY());
		for (int i=0;i<CWidth;i++) putchar(' ');
		moveCursor(0,getCursorY());
		sendMsg(s);
	}
}
void Connect() {
	puts("Initianizing...");
	if (WSAStartup(MAKEWORD(2,2),&Global::WData)!=0) {
		puts("Error initianizing!");
		ReportError();
		return;
	}
	if ((Global::UDPSocket=socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET) {
		puts("Error creating socket!");
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
		puts("Error binding socket!");
		ReportError();
		WSACleanup();
		return;
	}
	Running=1;
	CreateThread(NULL,0,ResultThread,NULL,0,NULL);
	Chat();
}
void Main() {
	cls();
	setConsoleSize(40,6);
	setCenterShow();
	CenterPrint("Welcome to VRoom!");
	CenterPrint("VER %s",VERSION);
	Split();
	printf("UserName:",VERSION);
	string tmp;
	getline(cin,tmp);
	if (!(Global::UserLength=tmp.length())) Main();
	Global::UserName=(char*) tmp.data();
	moveCursor(0,getCursorY()-1);
	CenterPrint("UserName:%s",Global::UserName);
	printf("RoomID:");
	cin>>Global::Port;
	moveCursor(0,getCursorY()-1);
	CenterPrint("RoomID:%d",Global::Port);
	Connect();
}
};
int main() {
	system("title VRoom - By Xs.JIONG");
	UI::InitConsole();
	UI::Main();
	return 0;
}
