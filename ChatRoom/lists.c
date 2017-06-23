#include<stdio.h>
#include<malloc.h>
#include<string.h> 
#include "lists.h"
/*
//client_info
struct cinfos{
	char userid[12];//12

	int ip;		//4

	unsigned short int port;	//2
};
*/

PNode create_list()
{

    PNode PHead=malloc(sizeof(Node));
    PHead->PNext=NULL;

    return PHead;

}

/**
**对链表进行遍历 ///list items
*/
void traverse(PNode pHead)
{
   PNode p=pHead->PNext;
   while(p!=NULL)
   {
       printf("username:%s ip:%d port:%d  \n",p->info.userid, p->info.ip, p->info.port);
       p=p->PNext;
   }
   printf("\n");
}
/**
*判断链表是否为空
*/

bool isempty(PNode pHead)
{
    if(NULL==pHead->PNext)
    {
            return true;
    }else{
    return false;
    }
}

///using username  to search
struct cinfos get_item(PNode pHead, char * username){
	PNode p = pHead;
	while(p!= NULL){
		p = p->PNext;
		if(strcmp(username, p->info.userid) == 0){
			return p->info;
		}
	} 
}
///using username to remove item
int remove_item(PNode pHead, char * username){
	PNode p = pHead;
	while(p!= NULL){
		PNode temp = p;
		p = p->PNext;
		if(strcmp(username, p->info.userid) == 0){
			temp->PNext = p->PNext;
			return 0;
		}
	}
	return -1;//not containter this user
}
///push_back
int push_item(PNode pHead, struct cinfos newItem){
	
	PNode p = pHead;
	while(p->PNext != NULL){
		p = p->PNext;
	} 
	PNode newNode = malloc(sizeof(Node));
	p->PNext = newNode;
	newNode->info = newItem;
	newNode->PNext = NULL;
}

/*
int main()
{

	PNode PHead= create_list();
	if(isempty(PHead))
		printf("链表为空\n");
		
	struct cinfos hacker;
	hacker.ip = 123;
	hacker.port = 34;
	strcpy((char *)hacker.userid, "hacking");
	
	struct cinfos jomaker;
	jomaker.ip = 999;
	jomaker.port = 3224;
	strcpy((char *)jomaker.userid, "jomaker");

	struct cinfos toughgame;
	toughgame.ip = 342;
	toughgame.port = 666;
	strcpy((char *)toughgame.userid, "toughgame");	
	
	push_item(PHead, hacker);
	push_item(PHead, jomaker);
	push_item(PHead, toughgame);
	traverse(PHead);
	
	char * username = "jomaker";
	struct cinfos temp;
	temp = get_item(PHead, username);
	printf("username:%s ip:%d port:%d  \n",temp.userid, temp.ip, temp.port);
	
	remove_item(PHead, username);
	traverse(PHead);
	
}
*/