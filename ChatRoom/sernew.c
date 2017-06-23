#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include <fcntl.h>
#include <signal.h>

#include "haha.h"
#include "sql.h"
#include "lists.h"

//PORT USED
struct packet buf; //tcp buffer
struct packet buffer;//udp buffer

int usockfd;

//define global var
#define BUF_SIZE 1028

//store the user info array
struct payload_lists gList;

//handle offline message or send offline lists
void udp_io_handler(int signum){
	//debug:
	printf("---sigio is coming---\n");
	//buffer recv global
	int addrsize;
	addrsize = sizeof(struct sockaddr_in);
	struct sockaddr_in dest_addr;
	bzero(&buffer, sizeof(buffer));

	int num = recvfrom(usockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&dest_addr, &addrsize);
	if(num < 0){
		perror("recv error!");
		exit(-1);
	}

	if(buffer.meta == LIST_OFFLINE){
		//send offline lists
		printf("LIST OFFLINE\n");
		//sql query count offline usernum
		int ucount = Alluser();
		int gcount = gList.usernum;
		int offcount = ucount - gcount;
		char AUser[20][12];

		int i, j, Flag;
		int ui = 0;
		for(i = 0; i < ucount; i++){
			Flag = 0;
			for(j = 0; j < gcount; j++){
				if(strcmp(gList.info[j].userid, Auser[i]) == 0){
					Flag = 1;
					break;
				}
			}
			if(Flag == 0){
				strcpy(AUser[ui], Auser[i]);
				ui++;
			}

		}
		printf("ui is: %d\n", ui);
		printf("%d\n", gList.usernum);
		for(i = 0; i < ui; i++){
			printf("%s \n", AUser[i]);
		}

		//packet the offline users
		bzero(&buffer, sizeof(buffer));
		buffer.meta = LIST_OFFLINE;
		memcpy(buffer.data, &ui, 4);

		char *p = buffer.data;
		p = p + 4;
		memcpy(p, AUser, ui*12);

		//sendto(reuse the recvform addr)
		sendto(usockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&dest_addr, addrsize);

	}else if(buffer.meta == CHAT_OFFLINE){
		//store the offline message using chatList{sender, recver, message}
		printf("CHAT_OFFLINE!!\n");
		printf("%s -> %s: %s\n", buffer.data, buffer.data + 12, buffer.data + 24);

		printf("Storin....\n");
		addMsg(buffer.data, buffer.data+12, buffer.data+24);

	}
}



///local pack to array gList
int packetList(PNode pHead){
	bzero(&gList, sizeof(gList));
	int usernum = 0;
	if(isempty(pHead)){
		usernum = 0;
	}else{
	   PNode p=pHead->PNext;
	   char * ptr = (char *)&(gList.info);	   
	   while(p!=NULL)
	   {
	   		char * pptr = (char *)&(p->info);
	   		usernum++;
	   		memcpy(ptr, pptr, sizeof(struct cinfos));
	   		ptr = ptr + sizeof(struct cinfos);
	        printf("packList >> username:%s ip:%x port:%d  \n",p->info.userid, p->info.ip, ntohs(p->info.port));
	        p=p->PNext;
	   }
	}
	gList.usernum = usernum;
	//send gList
} 

char urec[20][12];
       
int fd_A[BACKLOG];    // accepted connection fd  //store the lists
int conn_amount;    // current connection amount  

int sock_fd, new_fd;  // listen on sock_fd, new connection on new_fd  
struct sockaddr_in client_addr; // connector's address information   
socklen_t sin_size;  

int serstart(){
	bzero(&buf,sizeof(buf)); 
	
	int ssock = socket(AF_INET,SOCK_STREAM,0);
	if(ssock == -1){
		perror("sock() error!");
		exit(1);
	}
	
	int flag = 1;
	int flag_len = sizeof(int);
	if(setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &flag, flag_len) == -1){
		perror("ser socket option error!");
	}

	struct sockaddr_in  servaddr;
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(ssock,(struct sockaddr *)&servaddr,sizeof(servaddr)) == -1){
		perror("bind() error!");
		exit(1);
	}
	if(listen(ssock,BACKLOG) == -1){
		perror("listen() error!");
		exit(1);
	} 
	return ssock;

}

void showclient()  
{  
      int i;  
      printf("client amount: %d \n", conn_amount);  
      for (i = 0; i < BACKLOG; i++) {  
          printf("[%d]:%d  ", i, fd_A[i]);  
      }  
      printf("\n\n");  
}  

//send lists to all clients
int sendListAll(){

	int i;
	int usernum = gList.usernum;
	printf("[ON LINE]: totall: %d  BEGIN to send all\n", usernum);
	for(i = 0; i < usernum; i++){
		struct sockaddr_in dest_addr;
		bzero(&dest_addr, sizeof(struct sockaddr_in));
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = gList.info[i].port;
		dest_addr.sin_addr.s_addr = gList.info[i].ip.s_addr;

		bzero(&buffer, sizeof(buffer));
		buffer.meta = LIST;
		memcpy(buffer.data, (char *)&(gList), sizeof(gList));
		int addrsize = sizeof(dest_addr);
		//debug:
		//sleep(1);
		sendto(usockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&dest_addr, addrsize);

	}

}
//update link lists AND packet it AND send new lists
int updateList(PNode pHead, struct cinfos item, int flag){
	printf("update list~\n");
	if(flag == 1){
		//add item
		push_item(pHead, item);
		//check the item 
		printf("\t %s %d\n", item.userid, item.ip);
	}else if(flag == 2){
		remove_item(pHead, item.userid);
		//maybe using username to remove item
		printf("\tremove user item: %s\n", item.userid);
	}else{
		//debug:
		printf("updateList flag error!\n");
		return;
	}
	/*BUF:
		int meta;
		//usage: lists reply
		struct payload_lists{
			int usernum;
			struct cinfos info[20];
		};
	*/

	//packet the lists
	packetList(pHead);//update new ->gList

	bzero(&buf, sizeof(buf));
	buf.meta = LIST;
	memcpy(buf.data, (char *)&(gList), sizeof(gList));
	//send(userLists);
	write(new_fd, &buf, sizeof(buf));
}

//username -> record (search)
//all info (record) -> add to lists
//username -> remove record (search)

int selectfunc(PNode pHead){
	  fd_set fdsr;  //set 
      int maxsock;  
      struct timeval tv;  
       
      conn_amount = 0;  //init
      sin_size = sizeof(client_addr);  
     
      while (1) {  
            // initialize file descriptor set  
            FD_ZERO(&fdsr);  
            FD_SET(sock_fd, &fdsr);//add the listen fd
            maxsock = sock_fd;       
            
            // timeout setting  
            tv.tv_sec = 5;  
            tv.tv_usec = 0;  
       		
       		int i;
            // add active connection to fd set  
            for (i = 0; i < BACKLOG; i++) {  
                if (fd_A[i] != 0)
                    FD_SET(fd_A[i], &fdsr); //listen the local socket file ds
                if (fd_A[i]>maxsock) 
                    maxsock=fd_A[i]; //set the max fd to use select()
            	}  
       
       		int ret;
       		entry1:
            ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);  //has some fd can be read
            if (ret < 0 && errno == EINTR) {  
                printf("select is interrupted and go entry1 to restart func\n");
                goto entry1;
            } else if (ret == 0) {  
                printf("timeout\n");  
                continue;  //
            	}  
       			//select return > 0
            // check every fd in the set  (login? signup?)
            for (i = 0; i < BACKLOG; i++)
	      	 {  
              if(fd_A[i] != 0){
                if (FD_ISSET(fd_A[i], &fdsr)) {  
                		//debug:

                   	// ret = recv(fd_A[i], buf, sizeof(buf), 0);  //recv tthe type info
                   	bzero(&buf, sizeof(buf));
                	ret = read(fd_A[i], &buf, sizeof(buf));/////

                   	if (ret <= 0) {        // client close  //update lists
                    		printf("----------client[%d] close---------\n", i);  
                    		close(fd_A[i]);  
                    		FD_CLR(fd_A[i], &fdsr);  
     						printf("[close] user is: %s\n", urec[fd_A[i]]);

     						printf("urec5:%s urec6:%s\n",urec[5], urec[6]);

     						//debug:
     						// int i;
     						// for(i = 0; i < BACKLOG; i++){
     						// 	if(fd_A[i] != 0){
     						// 		printf("remain >>%d  %d > %s\n",i, fd_A[i], urec[fd_A[i]]);
     						// 	}
     						// }

                    		if(strlen(urec[fd_A[i]]) != 0){
		                		struct cinfos item;
		                		bzero(&item, sizeof(item));
		                		//item.userid = urec[fd_A[i]];
		                		strcpy(item.userid, urec[fd_A[i]]);

		                		//delete in link lists
		                		updateList(pHead, item, 2);

		                		//bcast to all (using all active fd) using udp
		                		sendListAll();

                    		}

                    		fd_A[i] = 0;
                    		//maxsock=0;  
                    		conn_amount--;  

                    } else{ // receive data  //case //handle_log 
                            if (ret <= BUF_SIZE){
                        	   if(buf.meta == SIGNUP){

									users signup;
									struct payload_log * temp = (struct payload_log *)&(buf.data);
									strcpy(signup.userid, temp->info.userid);
									strcpy(signup.passwd, temp->passwd);
									
									//store the info struct
									struct cinfos sinfo;
									memcpy(&sinfo, buf.data, sizeof(struct cinfos));

									//test
									char tempip[16];
									inet_ntop(AF_INET, &(temp->info.ip), tempip, 16);

									printf("ip is %s", tempip);
									unsigned short int temport = ntohs(temp->info.port);
									printf("port is %d", temport);

									if(Register(&signup)){
										//record fd
										//urec[fd_A[i]] = sinfo.userid;
										//printf("\n--------1 userid >> %s---------\n", sinfo.userid);
										memcpy(urec[fd_A[i]], sinfo.userid, 12);
										//printf("\n2 [urec] init :%s\n", urec[fd_A[i]]);
     									//printf("3 urec5:%s urec6:%s\n",urec[5], urec[6]);


										bzero(&buf, sizeof(buf));
										buf.meta = SIGSUSS;
										write(new_fd, &buf, sizeof(buf));

										//send lists and lists updates for all
										
										// bzero(&buf, sizeof(buf));
										// buf.meta = LIST;
										// packetList(pHead);
										// memcpy(buf.data, (char *)&(gList), sizeof(gList));//
										// write(new_fd, &buf, sizeof(buf));
										//start broadcast
										//fd_A[0..10];

										//update the lists
										updateList(pHead, sinfo, 1);
										sendListAll();

									}else{
										bzero(&buf, sizeof(buf));
										buf.meta = SIGFAIL;
										write(new_fd, &buf, sizeof(buf));
									}
								}else if(buf.meta == LOGIN){
									//debug:
									printf("I know you want to login\n");
									users login;
									struct payload_log * temp = (struct payload_log *)&(buf.data);
									strcpy(login.userid, temp->info.userid);
									strcpy(login.passwd, temp->passwd);

									//store the info struct
									struct cinfos sinfo;
									memcpy(&sinfo, buf.data, sizeof(struct cinfos));
									//test
									char tempip[16];
									inet_ntop(AF_INET, &(temp->info.ip), tempip, 16);

									printf("ip is %s", tempip);
									unsigned short int temport = ntohs(temp->info.port);
									printf("port is %d", temport);

									if(Login(&login)){
										//record fd and user
										printf("\n----------1 userid>>%s-----------\n", sinfo.userid);
										memcpy(urec[fd_A[i]], sinfo.userid, 12);
										printf("\n2 [urec] init :%s\n", urec[fd_A[i]]);
     									printf("3 urec5:%s urec6:%s\n",urec[5], urec[6]);







										bzero(&buf, sizeof(buf));
										buf.meta = LOGSUSS;
										write(new_fd, &buf, sizeof(buf));
										//lists updates

										//send lists and lists updates for all
										
										// bzero(&buf, sizeof(buf));
										// buf.meta = LIST;
										// packetList(pHead);
										// memcpy(buf.data, (char *)&(gList), sizeof(gList));//
										// write(new_fd, &buf, sizeof(buf));

										//update the lists
										//debug: temp is temp buz buf is zero
										updateList(pHead, sinfo, 1);//just to one (add item)
										sendListAll();

																			//get msg is there is some
										int msgnum = getMsg(sinfo.userid);
										printf("----The left off-line message is %d\n", msgnum);
										if(msgnum){
											//addr
											struct sockaddr_in usockaddr;
											usockaddr.sin_family = AF_INET;
											usockaddr.sin_port =  sinfo.port;
											usockaddr.sin_addr.s_addr = sinfo.ip.s_addr;

											int addrsize = sizeof(usockaddr);

											//sleep(5);
											int i;
											for(i = 0; i < msgnum; i++){
												//packet
												bzero(&buffer, sizeof(buffer));
												buffer.meta = CHAT_OFFLINE;
												//debug:
												sleep(1);
												printf("-----offline msg from %s Sending to dest-----\n", MsgOff[i]);
												memcpy(buffer.data, MsgOff[i], sizeof(buffer.data));
												sendto(usockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&usockaddr, addrsize);

											}
										}

									}else{
										bzero(&buf, sizeof(buf));
										buf.meta = LOGFAIL;
										write(new_fd, &buf, sizeof(buf));
									}//end  else
								}
                        	//debug:
                         	//memset(&buf[ret], '\0', 1);  
                        	//printf("client[%d] send:%s\n", i, buf);  
                    	}//end ret
            	    }//end else
		   		}//end fd_isset
            }  
        }
       
            // check whether a new connection comes  (listen fd)
            if (FD_ISSET(sock_fd, &fdsr)) {  
                new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size); 
                //debug:
                printf("accept after!");
                if (new_fd <= 0) {  
                    perror("accept");  
                    continue;  
                } 
       
                // add the new conncetion to fd queue  
                if (conn_amount < BACKLOG) {
                   for (i = 0; i < BACKLOG; i++)
					{
						if(fd_A[i] == 0){//check the empty fd element
                 		conn_amount++;
						fd_A[i] = new_fd;  //4--->1023
		           		printf("new connection client[%d] :%d\n", conn_amount,  
		                /*inet_ntoa(client_addr.sin_addr), */ntohs(client_addr.sin_port));  
			        		if (new_fd > maxsock)  
			            			maxsock = new_fd;  
									break;////
							}
					} 
                }  //max connection
                else {  
                    printf("max connections arrive, exit\n");  
                    send(new_fd, "bye", 4, 0);  ////
                    close(new_fd);  
                    break;  
                }  
          	}  //end if
			   showclient();
		  }  //end loop
}

int main(int argc, char * argv[]){
	//init the user lists
	PNode PHead= create_list();
	if(isempty(PHead)){
		printf("链表为空\n");
	}

	//start tcp server
	sock_fd = serstart();

	//start udp server
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
	usaddr.sin_port = htons(SER_UDP_PORT);

	//udp server bind addr
	if(bind(usockfd, (struct sockaddr *)&usaddr, sizeof(usaddr)) < 0){
		perror("udp server bind error!");
		exit(1);
	}

	//setown
	signal(SIGIO, udp_io_handler);

	if (fcntl(usockfd, F_SETOWN, getpid()) < 0){
		perror("fcntl F_SETOWN");
		exit(1);
	}

	if (fcntl(usockfd, F_SETFL, FASYNC) <0 ){
		perror("fcntl F_SETFL, FASYNC");
		exit(1);
	}

	selectfunc(PHead);
	close(sock_fd);
	
}