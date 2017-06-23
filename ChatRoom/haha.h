#ifndef HAHA_H
#define HAHA_H
#include<netinet/in.h>

#define C2C_FILE 1
#define C2C_CHAT      2
#define SIGNUP  	  3
#define LOGIN    	  4     
//#define c2c_contrans
#define LOGFAIL	5
#define SIGFAIL 6
#define LOGSUSS 7
#define SIGSUSS 8

#define LIST 9

//prev use for heartbeat packet
#define ECHO	10
#define REPLY	11

#define FILE_READY 	12
#define FILE_REJ 	13

#define LIST_OFFLINE 14
#define CHAT_OFFLINE 15

#define PORT 5000
#define SER_UDP_PORT 4999
#define BACKLOG  10
#define NAME_MAX 10
#define NAMESIZE 100

#define MAX_LINE 4096
//define online or offline
#define ON 1
#define OFF 0

//declare the packet that always send or receive
struct packet{
	int meta;
	char data[1024];
};

//client_info
struct cinfos{
	char userid[12];//12

	struct in_addr ip;	//4

	// peer chat server port
	unsigned short int port;	//2->4
};

//payload : data[1024]
//usage: reg/login
struct payload_log
{
	struct cinfos info; //20
	
	char passwd[8];	//8
};

//usage: lists reply
struct payload_lists{
	int usernum;
	struct cinfos info[20];
};

struct chatMsg{
	char userid[12];
	char msg[1012];
};

#endif