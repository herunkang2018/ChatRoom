extern char Auser[20][12];
extern char MsgOff[10][1024];

typedef struct {
	char userid[12];
	char passwd[8];
}users;

int Register(users * a);
int Login(users * a);
int Alluser();