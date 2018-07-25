#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <iostream>
#include <string>
#include <vconsole.h>
#include <vfield.h>
#include <sstream>
#include <cwctype>
#define THREAD DWORD WINAPI
#define BufSize 1024
using namespace std;

const char* VERSION="1.0";

const int PORT=8898;
namespace Type {
	const char JOIN=1;
	const char MSG=2;
	const char RENAME=3;
	const short MsgLimit=100;
};
namespace Global {
	char* UserName;
	short UserLength;
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
VField *Content,*Edit;
void cls() {
	system("cls");
}
CONSOLE_SCREEN_BUFFER_INFO _Buffer;
short CWidth=80,CHeight=10;
void setCenterShow(){
	long scrWidth,scrHeight;
	RECT rect;
	scrWidth=GetSystemMetrics(SM_CXSCREEN);
	scrHeight=GetSystemMetrics(SM_CYSCREEN);
	GetWindowRect(Global::ConsoleWindow,&rect);
	SetWindowPos(Global::ConsoleWindow,HWND_TOPMOST,(scrWidth-rect.right)/2,(scrHeight-rect.bottom)/2,rect.right-rect.left,rect.bottom-rect.top,SWP_SHOWWINDOW);
}
char _Tmp[2048];
template <class...T>
void printCenter(const char* f, T...arg) {
	sprintf(_Tmp,f,arg...);
	int ss=(CWidth-strlen(_Tmp))/2;
	for (int i=0;i<ss;i++) putchar(' ');
	printf("%s",_Tmp);
	putchar('\n');
}
string __TMP;
bool ContentEmpty;
template <class...T>
void printCCenter(const char* f, T...arg) {
	sprintf(_Tmp,f,arg...);
	int ss=(CWidth-strlen(_Tmp))/2;
	__TMP.clear();
	if (ContentEmpty) ContentEmpty=0; else __TMP+='\n';
	while (ss--) __TMP+=' ';
	__TMP+=_Tmp;
	Content->append(__TMP);
}
template <class...T>
void printC(const char* f, T...arg) {
	sprintf(_Tmp,f,arg...);
	__TMP.clear();
	if (ContentEmpty) ContentEmpty=0; else __TMP+='\n';
	__TMP+=_Tmp;
	Content->append(__TMP);
}
void setConsoleSize(short a, short b) {
	char tmp[30];
	sprintf(tmp,"mode con cols=%d lines=%d",a,b);
	system(tmp);
	CWidth=a,CHeight=b;
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
	return "";
}
void ReportError() {
	printf("Error Code:%d\n",WSAGetLastError());
	system("pause>nul");
	exit(0);
}
bool Running=0;
bool Sending=0;
inline void ParseCmd() {
	char type=Global::MsgBuf[0];
	char* con=Global::MsgBuf+1;
	switch (type) {
		case Type::JOIN:{
			printC("[System] %s joined the room",con);
			break;
		}
		case Type::MSG:{
			short s;
			memcpy((char*)&s,con,2);
			con[s+2]='\0';
			sprintf(_Tmp,"\n[%s] %s",con+2,con+s+3);
			Content->append(_Tmp);
			if (Sending) Edit->clear(),Sending=0;
			break;
		}
		case Type::RENAME:{
			short s;
			memcpy((char*)&s,con,2);
			con[s+2]='\0';
			sprintf(_Tmp,"\n[System] %s has renamed to %s",con+2,con+s+3);
			Content->append(_Tmp);
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
	int len=Global::UserLength+msg.length()+5;
	char* i=new char[len];
	i[0]=Type::MSG;
	memcpy(i+1,(char*)&Global::UserLength,2);
	memcpy(i+3,Global::UserName,Global::UserLength);
	i[Global::UserLength+3]=' ';
	memcpy(i+Global::UserLength+4,msg.data(),msg.length()+1);
	Sending=1;
	Global::sendR(i,len);
	delete[] i;
}
void CSplit() {
	__TMP.clear();
	__TMP+='\n';
	for (int i=0;i<CWidth;i++) __TMP+='=';
	Content->append(__TMP);
}
void Main();
void Disconnect() {
	closesocket(Global::UDPSocket);
	UI::Main();
}
inline void ExecCmd(const char* s) {
	Edit->clear();
	if (memcmp(s,"exit",4)==0) {
		UI::Disconnect();
		return;
	} else if (memcmp(s,"name",4)==0) {
		char *ns=(char*) s+4;
		int len=strlen(ns);
		int i=0;
		while (ns[i]==' '&&i<len) i++;
		if (i>=len-1) {
			Content->append("\n[System] syntax error, expected \"name [string]\"");
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
	}
	printC("[System] Undefined Command");
}
void SimpleListener(VField* arg) {
	const char* s=arg->getText().c_str();
	if (strlen(s)==0) return;
	if (s[0]=='/')
		ExecCmd(s+1);
	else
		sendMsg(arg->getText());
}
void Chat() {
	cls();
	setConsoleSize(70,20);
	VConsole::setBufferSize(70,25);
	Content=new VField(0,0,CWidth-1,CHeight-2);
	Content->setSingleLine(0);
	ContentEmpty=1;
	printCCenter("=====================");
	printCCenter("= Welcome to VRoom! =");
	printCCenter("=====================");
	printCCenter("UserName:%s RoomID:%d",Global::UserName,Global::Port);
	CSplit();
	Edit=new VField(0,CHeight-1,CWidth-1,CHeight-1);
	Edit->enableInput();
	Edit->setSingleLine(1);
	Edit->setAutoRedraw(1);
	Edit->setTarget(Content);
	Edit->setEnterListener(SimpleListener);
	Edit->focus();
	char c[Global::UserLength+2];
	c[0]=Type::JOIN;
	memcpy(c+1,Global::UserName,Global::UserLength+1);
	Global::sendR(c,Global::UserLength+2);
	while (true) {Sleep(2000);}
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
	printCenter("Welcome to VRoom!");
	printCenter("VER %s",VERSION);
	Split();
	printf("UserName:");
	string tmp;
	getline(cin,tmp);
	if (!(Global::UserLength=tmp.length())) Main();
	Global::UserName=(char*) tmp.data();
	VConsole::moveCursor(0,VConsole::getCursorY()-1);
	printCenter("UserName:%s",Global::UserName);
	printf("RoomID:");
	cin>>Global::Port;
	VConsole::moveCursor(0,VConsole::getCursorY()-1);
	printCenter("RoomID:%d",Global::Port);
	Connect();
}
};
int main() {
	system("title VRoom - By Xs.JIONG");
	Global::ConsoleWindow=GetForegroundWindow();
	VConsole::Init();
	UI::Main();
	return 0;
}
