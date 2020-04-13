/*IP:192.168.40.128*/
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "msg.h"

#define SER_TCP_PORT "192.168.40.128"

void udp_io_handler(int);
struct packet buf;
struct packet buffer;

char suser[12];
char loguser[12];
unsigned short logport;

//用数组存储待打包的用户信息
struct payload_lists gList;

//点对点在线聊天的消息格式
struct chatMsg chatMsg;

//udp server socket
int usockfd;

//存储文件名
char FileName[NAMESIZE];

void writefile(int sockfd, FILE *fp)
{
    ssize_t n; //每次接受数据数量
    char buff[MAX_LINE] = {0}; //数据缓存
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0) 
    {
        if (n == -1)
        {
            perror("Receive File Error");
            exit(1);
        }
        
        //将接受的数据写入文件
        if (fwrite(buff, sizeof(char), n, fp) != n)
        {
            perror("Write File Error");
            exit(1);
        }
        memset(buff, 0, MAX_LINE); //清空缓存
    }
}

void sendfile(FILE *fp, int sockfd) 
{
    int n; //每次读取数据数量
    char sendline[MAX_LINE] = {0}; //暂存每次读取的数据
    while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0) 
    {
        if (n != MAX_LINE && ferror(fp)) //读取出错并且没有到达文件结尾
        {
            perror("Read File Error");
            exit(1);
        }
        
        //将读取的数据发送到TCP发送缓冲区
        if (send(sockfd, sendline, n, 0) == -1)
        {
            perror("Can't send file");
            exit(1);
        }
        memset(sendline, 0, MAX_LINE); //清空暂存字符串
    }
}

unsigned long get_file_size(const char *path)  
{  
    unsigned long filesize = -1;  
    FILE *fp;  
    fp = fopen(path, "r");  
    if(fp == NULL)  
        return filesize;  
    fseek(fp, 0L, SEEK_END);  
    filesize = ftell(fp);  
    fclose(fp);  
    return filesize;  
}  

int con2server(){//no signup or login

	int csock = socket(AF_INET,SOCK_STREAM,0);
	if(csock == -1){
		perror("sock() error!");
		exit(1);
	}
	
	//server addr
	struct sockaddr_in  cliaddr;
	bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(PORT);
	cliaddr.sin_addr.s_addr = inet_addr(SER_TCP_PORT);//modify
		
	if(connect(csock,(struct sockaddr *)&cliaddr,sizeof(cliaddr)) == -1){
		perror("connect() error!");
		close(csock);
		exit(1);
	}
	return csock;
}

void handle_log(struct payload_log *pload, char *userid, char *ip, unsigned short int port, char *passwd){

	bzero(pload, sizeof(pload));

	strcpy((char *)pload->info.userid, userid);
	pload->info.ip.s_addr = inet_addr(ip);
	pload->info.port = htons(port);//packet using network byte REMEMBER!!
	strcpy((char *)pload->passwd, passwd);
}

int signup2server(int csock, struct payload_log *pload){

	bzero(&buf,sizeof(buf));
	buf.meta = SIGNUP;
	memcpy(buf.data, (char *)pload, sizeof(struct payload_log));

	write(csock, &buf,sizeof(buf));
	read(csock, &buf, sizeof(buf));
	if(buf.meta == SIGFAIL){
		return 0;
	}else if(buf.meta == SIGSUSS){
		return 1;
	}else{
		return 0;
	}
}

int login2server(int csock, struct payload_log *pload){

	bzero(&buf,sizeof(buf));
	buf.meta = LOGIN;
	memcpy(buf.data, (char *)pload, sizeof(buf.data));
	
	write(csock, &buf, sizeof(buf));
	read(csock, &buf, sizeof(buf));
	if(buf.meta == LOGFAIL){
		return 0;
	}else if(buf.meta == LOGSUSS){
		return 1;
	}else{
		return 0;//default return
	}
}

//recv chat
void udp_io_handler(int signum){
	//debug:
	printf("\n[INFO]	sigio is coming\n");
	//buffer recv global
	int addrsize = sizeof(struct sockaddr_in);
	struct sockaddr_in dest_addr;

	int num = recvfrom(usockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&dest_addr, &addrsize);
	if(num < 0){
		perror("[ERROR]	 recv error!");
		exit(-1);
	}
	if(buffer.meta == LIST){
		//update lists
		memcpy((char *)&(gList), buffer.data, sizeof(gList));
		showList();

	}else if(buffer.meta == C2C_CHAT){

		//debug:
		printf("[INFO]	接收在线消息\n");
		char * p = (char *)&(buffer.data);
		p = p + 12;
		printf("[From]: %s > %s\n", buffer.data, p);

	}else if(buffer.meta == C2C_FILE){
		printf("[有发往您的文件]:\n");
		char *p = (char *)&(buffer.data);
		//store the filename at RECVer
		memcpy(FileName, p, NAMESIZE);
		p = p + NAMESIZE;
		unsigned long size;
		memcpy(&size, p, sizeof(unsigned long));
		p = p + 8;
		printf("[INFO]	发送方: %s \t 文件名: %s \t 大小: %luB\n", p, buffer.data, size);
		//record peer server info <- user gList
		char tempuser[12];
	    memcpy(tempuser, p, 12);



		printf("[HELP]	接收(1) or 拒绝(0):\n>");
		int get;
		scanf("%d", &get);
		printf("\n");
		if(get == 1){

			//rename the new file
			char fname[NAMESIZE];
			printf("[Rename]: ");
			scanf("%s", fname);
			//store the filename at RECVer
			memcpy(FileName, fname, NAMESIZE);
			//send file_ready
			bzero(&buffer, sizeof(buffer));
			buffer.meta = FILE_READY;

			//send local user
			memcpy(buffer.data, loguser, 12);

			//scokaddr process
		    struct sockaddr_in dest_addr;
		    memset(&dest_addr, 0, sizeof(dest_addr));
		    dest_addr.sin_family = AF_INET;


		    //search the user info(ip/port)
		    int unum;
			for(unum = 0; unum < gList.usernum; unum++){
				if(strcmp(gList.info[unum].userid, tempuser) == 0){
		    		dest_addr.sin_port = gList.info[unum].port;
		    		dest_addr.sin_addr.s_addr = gList.info[unum].ip.s_addr;
		    		printf("[INFO]	查找用户成功\n");
		    		break;
				}
			}
			int addrsize = sizeof(dest_addr);
			sendto(usockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&dest_addr, addrsize);

			//start file tcp server
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		    if (sockfd == -1) 
		    {
		        perror("[ERROR]	 Can't allocate sockfd");
		        exit(1);
		    }
		    
			int flag = 1;
			int flag_len = sizeof(int);
			if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, flag_len) == -1){
				perror("[ERROR]	 ser socket option error!");
			}

		    //配置 服务器套接字地址
		    struct sockaddr_in clientaddr, serveraddr;
		    memset(&serveraddr, 0, sizeof(serveraddr));
		    serveraddr.sin_family = AF_INET;
		    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		    serveraddr.sin_port = htons(logport);////

		    //绑定套接字与地址
		    if (bind(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) == -1) 
		    {
		        perror("[ERROR]	 Bind Error");
		        exit(1);
		    }

		    //转换为监听套接字
		    if (listen(sockfd, BACKLOG) == -1) 
		    {
		        perror("[ERROR]	 Listen Error");
		        exit(1);
		    }

		    //等待连接完成
		    socklen_t addrlen = sizeof(clientaddr);
		    int connfd = accept(sockfd, (struct sockaddr *) &clientaddr, &addrlen); //已连接套接字
		    if (connfd == -1) 
		    {
		        perror("[ERROR]	 Connect Error");
		        exit(1);
		    }
		    close(sockfd); //关闭监听套接字

			//accept the connector and begin transfer file

		    //创建文件
		    FILE *fp = fopen(fname, "wb");
		    if (fp == NULL) 
		    {
		        perror("[ERROR]	 Can't open file");
		        exit(1);
		    }
		    
		    //把数据写入文件 ////
		    //wait for data coming
		    writefile(connfd, fp);
		    puts("[INFO]  Receive Success");

		    //关闭文件和已连接套接字
		    fclose(fp);
		    close(connfd);

		}else if(get == 0){
			//printf("I choose 0\n");
			//buffer process
			bzero(&buffer, sizeof(buffer));
			buffer.meta = FILE_REJ;


			//scokaddr process
		    struct sockaddr_in dest_addr;
		    memset(&dest_addr, 0, sizeof(dest_addr));
		    dest_addr.sin_family = AF_INET;


		    //search the user info(ip/port)
		    int unum;
			for(unum = 0; unum < gList.usernum; unum++){
				if(strcmp(gList.info[unum].userid, tempuser) == 0){
		    		dest_addr.sin_port = gList.info[unum].port;
		    		dest_addr.sin_addr.s_addr = gList.info[unum].ip.s_addr;
		    		printf("[INFO]	查找用户成功\n");
		    		break;
				}
			}
			int addrsize = sizeof(dest_addr);
			sendto(usockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&dest_addr, addrsize);

		}

	}else if(buffer.meta == FILE_READY){
			//I am SENDer
			//connect the tcp file server
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		    if (sockfd < 0) 
		    {
		        perror("[ERROR]	 Can't allocate sockfd");
		        exit(1);
		    }
		    unsigned short temport;
		    char tempuser[12];
		    memcpy(tempuser, buffer.data, 12);

		    //设置传输对端套接字地址 // lists
		    struct sockaddr_in serveraddr;
		    memset(&serveraddr, 0, sizeof(serveraddr));
		    serveraddr.sin_family = AF_INET;


		    //查找user info(ip/port)
		    int unum;
			for(unum = 0; unum < gList.usernum; unum++){
				if(strcmp(gList.info[unum].userid, tempuser) == 0){
		    		serveraddr.sin_port = gList.info[unum].port;
		    		serveraddr.sin_addr.s_addr = gList.info[unum].ip.s_addr;
				}
			}


		    //连接文件服务器
		    if (connect(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
		    {
		        perror("[ERROR]	 Connect Error");
		        exit(1);
		    }
		    
		    //获取文件名
		    char *filename = FileName; //文件名
		    if (filename == NULL)
		    {
		        perror("[ERROR]	 Can't get filename");
		        exit(1);
		    }
		    
		    
		    //打开要发送的文件
		    FILE *fp = fopen(FileName, "rb");
		    if (fp == NULL) 
		    {
		        perror("[ERROR]	 Can't open file");
		        exit(1);
		    }

		    //读取并发送文件
		    sendfile(fp, sockfd);
		    puts("[INFO]  发送成功！");

		    //关闭文件和套接字
		    fclose(fp);
		    close(sockfd);

	}else if(buffer.meta == FILE_REJ){
		//return
		printf("[SORRY]	您的朋友拒绝了你的文件\n");
	}else if(buffer.meta == LIST_OFFLINE){
		//recv the lists and print, you can select to send message
		int offcount;
		char AUser[20][12];
		memcpy(&offcount, buffer.data, 4);
		char *p = buffer.data;
		p = p + 4;
		memcpy(AUser, p, offcount*12);

		int index;
		printf("[离线用户]:\n");
		for(index = 0; index < offcount; index++){
			printf("%d:%s ", index, AUser[index]);
		}
		printf("\n");

		int choose;//0 1 2
		printf("[HELP]	请选择一个离线用户\n>");
		scanf("%d", &choose);
		getchar();

		char tempbuf[1012];
		//glist.info[choose] -> userinfo to send
		printf("[To]: %s >", AUser[choose]);
		//debug:
		//fflush(stdout);

		fgets(tempbuf, sizeof(tempbuf), stdin);
		////scanf("%s", tempbuf);
		//printf("tempbuf is:%s:\n", tempbuf);
		//debug
		//fflush(stdout);

		bzero(&buffer, sizeof(buffer));
		buffer.meta = CHAT_OFFLINE;
		
		p = buffer.data;
		memcpy(p, loguser, 12);
		p = p + 12;
		memcpy(p, AUser[choose], 12);
		p = p + 12;
		memcpy(p, tempbuf, 1000);


		struct sockaddr_in usockaddr;
		usockaddr.sin_family = AF_INET;
		usockaddr.sin_port = htons(SER_UDP_PORT);//server
		usockaddr.sin_addr.s_addr = htonl(INADDR_ANY);//server test

		int addrsize = sizeof(usockaddr);
		sendto(usockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&usockaddr, addrsize);
		printf("[NOTE]	您发送了一条离线消息\n");

	}else if(buffer.meta == CHAT_OFFLINE){
		printf("[离线消息]\n\t[FROM %s %s] > %s\n", buffer.data, buffer.data+12, buffer.data+32);
	}


	else{
		printf("[ERROR]	unknown msg type!\n");  
	}

}

//通过gList中的信息定位选择的用户（信息）
//在这里通过信号异步（覆盖ctrl+c）的方式 通知程序执行任务
int send_chat(){
	printf("[INFO]	显示在线用户列表:\n");
	showList();
	printf("[HELP]	请选择聊天对象, 系统帮助请输入: '66'\n>");

	int choose;//0 1 2
	scanf("%d", &choose);
	getchar();

	if(choose == 88){
		printf("[INFO]	程序正在退出...\n");
		exit(0);
	}else if(choose == 55){
		int a;
		printf("[HELP]	请选择发送的用户\n>");
		scanf("%d", &a);
		getchar();

		bzero(&buffer, sizeof(buffer));
		buffer.meta = C2C_FILE;

		char fname[NAMESIZE];
		printf("[HELP]	请选择一个本地文件\n>");
		scanf("%s", fname);
		//store the filename at SENDer
		memcpy(FileName, fname, NAMESIZE);
		unsigned long fsize = get_file_size(fname);
		memcpy(buffer.data, fname, NAMESIZE);
		//use temptr to packet buffer
		char * temptr = (char *)&(buffer.data);
		temptr = temptr + NAMESIZE;
		memcpy(temptr, &fsize, sizeof(fsize));
		temptr = temptr + 8;
		memcpy(temptr, loguser, 12);//always send local info

		struct sockaddr_in usockaddr;
		usockaddr.sin_family = AF_INET;
		usockaddr.sin_port = gList.info[a].port;//gList.info[choose].port
		////usockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		usockaddr.sin_addr.s_addr = gList.info[a].ip.s_addr;//struct sin_addr -> contains s_addr

		int addrsize = sizeof(usockaddr);
		sendto(usockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&usockaddr, addrsize);

		return;
	}else if(choose == 11){
		//请求离线列表
		bzero(&buffer, sizeof(buffer));
		buffer.meta = LIST_OFFLINE;

		struct sockaddr_in usockaddr;
		usockaddr.sin_family = AF_INET;
		usockaddr.sin_port = htons(SER_UDP_PORT);
		usockaddr.sin_addr.s_addr = htonl(INADDR_ANY);//local test server

		int addrsize = sizeof(usockaddr);
		sendto(usockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&usockaddr, addrsize);
		return;

	}else if(choose == 66){
		printf("[HELP]  1, CTRL+C 发送在线消息\n\t2, '11' 发送离线消息\n\t3, '55' 发送在线文件\n\t4, '66' 显示系统帮助\n\t5, '88' to Exit the Program\n");
		return;
	}
	char tempbuf[1012];
	//glist.info[choose] -> userinfo to send
	printf("[To]: %s >", gList.info[choose].userid);
	//debug:
	//fflush(stdout);

	fgets(tempbuf, sizeof(tempbuf), stdin);
	////scanf("%s", tempbuf);
	//printf("tempbuf is:%s:\n", tempbuf);
	//debug
	//fflush(stdout);

	buffer.meta = C2C_CHAT;
	bzero(&chatMsg, sizeof(chatMsg));
	//chatMsg.userid = gList.info[choose].userid;
	strcpy(chatMsg.userid, gList.info[choose].userid);

	memcpy(chatMsg.msg, tempbuf, sizeof(tempbuf));
	memcpy(buffer.data, &chatMsg, sizeof(buffer.data));


	struct sockaddr_in usockaddr;
	usockaddr.sin_family = AF_INET;
	usockaddr.sin_port = gList.info[choose].port;//gList.info[choose].port
	////usockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	usockaddr.sin_addr.s_addr = gList.info[choose].ip.s_addr;//struct sin_addr -> contains s_addr

	int addrsize = sizeof(usockaddr);
	sendto(usockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&usockaddr, addrsize);
	printf("\n[NOTE]  您已经发送一条消息\n");
	fflush(stdout);


}


//SIGINT ctrl+c （覆盖SIGINT的信号处理函数，转而执行聊天等任务）
void SignHandler(int iSigNum)
{
	printf("\n-----捕获到SIGIO信号:%d-----\n", iSigNum);
	send_chat();
}

//通过TCP先向登录/注册用户发送在线List
int recvList(int sockfd){
	bzero(&buf, sizeof(buf));
	int ret = read(sockfd, &buf, sizeof(buf));
	//printf("ret bytes is %d\n", ret);
	if(ret < 0){
		perror("read list error!");
	}else{
		printf("%d\n", buf.meta);
		if(buf.meta == LIST){
			memcpy((char *)&(gList), buf.data, sizeof(gList));
		}
	}
}

int showList(){
	int usernum = gList.usernum;
	int i;
	printf("[ON LINE]: 总计: %d\n", usernum);
	for(i = 0; i < usernum; i++){
		printf("\t %d:%s ", i, gList.info[i].userid);
	}
	printf("列表结束\n");
}

int main(int argc, char * argv[]){
	if(argc != 3){
		printf("[Usage]: %s [你的ip] [你的port]\n", argv[0]);
		exit(0);
	}

	//signal
	signal(SIGINT,SignHandler);

	//connect to server
	printf("登录:1 注册:2 退出:e 请选择 > ");
	char *ip = argv[1];
	unsigned short int port = atoi(argv[2]);
	logport = port;
	char username[12];
	char passwd[8];

	struct payload_log payload;
	
	int csock = con2server();

	char choose = getchar();

	if(choose == '1'){
		printf("用户名 (less than %d) >", NAME_MAX-1);
		scanf("%s", username);
		if(strlen(username) > NAME_MAX-1){
			//printf("%d\n", strlen(namein));
			perror("[ERROR] name size is out of length!");
			exit(0);
		}
		printf("密码 >");
		scanf("%s", passwd);
		printf("[NOTE] loading...\n");
		//func login with return LOGSUSS or LOGFAIL

		handle_log(&payload, username, ip, port, passwd);
		if(login2server(csock, &payload)){
			memcpy(loguser, username, 12);
			//printf("My :%s\n", loguser);

			//start the udp server
			usockfd = socket(AF_INET, SOCK_DGRAM, 0);
			if(usockfd < 0){
				perror("udp server socket() error");
			}

			int flags = 1;
			int flags_len = sizeof(int);
			if(setsockopt(usockfd, SOL_SOCKET, SO_REUSEADDR, &flags, flags_len) == -1){
				perror("ser socket option error!");
			}

			struct sockaddr_in usaddr;
			usaddr.sin_family = AF_INET;
			usaddr.sin_addr.s_addr = htonl(INADDR_ANY);//
			usaddr.sin_port = htons(port);

			//udp server bind addr
			if(bind(usockfd, (struct sockaddr *)&usaddr, sizeof(usaddr)) < 0){
				perror("udp server bind error!");
				exit(1);
			}

			signal(SIGIO, udp_io_handler);

			if (fcntl(usockfd, F_SETOWN, getpid()) < 0){
				perror("fcntl F_SETOWN");
				exit(1);
			}

			if (fcntl(usockfd, F_SETFL, FASYNC) <0 ){
				perror("fcntl F_SETFL, FASYNC");
				exit(1);
			}

			printf("登录成功!\n");
			
			//wait the list
			recvList(csock);
			showList();

		}else{
			printf("登录失败!\n");
		}

	}else if(choose == '2'){
	
		printf("用户名 (less than %d) >", NAME_MAX-1);
		scanf("%s", username);
		printf("密码 >");
		scanf("%s", passwd);

		//func signup with return SIGSUSS or SIGFAIL
		handle_log(&payload, username, ip, port, passwd);
		if(signup2server(csock, &payload)){
			//store the current login user
			memcpy(loguser, username, 12);

			//start the udp server
			usockfd = socket(AF_INET, SOCK_DGRAM, 0);
			if(usockfd < 0){
				perror("udp server socket() error");
			}

			int flags = 1;
			int flags_len = sizeof(int);
			if(setsockopt(usockfd, SOL_SOCKET, SO_REUSEADDR, &flags, flags_len) == -1){
				perror("ser socket option error!");
			}

			struct sockaddr_in usaddr;
			usaddr.sin_family = AF_INET;
			usaddr.sin_addr.s_addr = htonl(INADDR_ANY);//
			usaddr.sin_port = htons(port);

			//udp server bind addr
			if(bind(usockfd, (struct sockaddr *)&usaddr, sizeof(usaddr)) < 0){
				perror("udp server bind error!");
				exit(1);
			}

			signal(SIGIO, udp_io_handler);

			if (fcntl(usockfd, F_SETOWN, getpid()) < 0){
				perror("fcntl F_SETOWN");
				exit(1);
			}

			if (fcntl(usockfd, F_SETFL, FASYNC) <0 ){
				perror("fcntl F_SETFL, FASYNC");
				exit(1);
			}
			//wait the list
			printf("注册成功，现在你已经登录\n");

			recvList(csock);
			showList();

		}else{
			printf("注册失败，请检查自己的信息!\n");
		}

	}else{
		perror("输入错误!");		
	}

	//wait sianal coming
	while(1){
	}

}