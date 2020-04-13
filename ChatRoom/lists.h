#ifndef LISTS_H
#define LISTS_H

#include <stdbool.h>
#include "msg.h"
typedef struct node{
	struct cinfos info;
	struct node * PNext;
}*PNode, Node;

PNode create_list();
void traverse(PNode pHead);
bool isempty(PNode pHead);
struct cinfos get_item(PNode pHead, char * username);
int remove_item(PNode pHead, char * username);
int push_item(PNode pHead, struct cinfos newItem);

#endif