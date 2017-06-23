/*-lmysqlclient -I/usr/include/mysql*/
#include <my_global.h>
#include <mysql.h>
#include <string.h>

#include "sql.h"

char Auser[20][12];
char MsgOff[10][1024];

int addMsg(char *sid, char *rid, char *msg){

  MYSQL *conn;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int num_fields;
  int i;
  int count = 0;
  //define 20 users array

  char query[200];

  bzero(query, sizeof(query));
  strcpy(query, "INSERT INTO message VALUES('");
  strcat(query, sid);
  strcat(query, "','");
  strcat(query, rid);
  strcat(query, "','");
  strcat(query, msg);
  strcat(query, "',");
  strcat(query, "now()");
  strcat(query, ")");

  conn = mysql_init(NULL);
  mysql_real_connect(conn, "localhost", "rick", "secret", "chatdb", 0, NULL, 0);

  mysql_query(conn, query);

}

int getMsg(char *rid){
    printf("---Get Msg Begin!!---\n");
    //return msg number
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    int num_fields;
    int i;
    int count = 0;
    //define 20 users array

    char query[200];
    strcpy(query, "SELECT sid,msg,time FROM message WHERE rid='");
    strcat(query, rid);
    strcat(query, "'");

    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "rick", "secret", "chatdb", 0, NULL, 0);

    mysql_query(conn, query);

    result = mysql_store_result(conn);

    num_fields = mysql_num_fields(result);//1 

    bzero(MsgOff, 10240);//init message box

    while ((row = mysql_fetch_row(result))){

        // strcpy(MsgOff[count], row[0]);
        memcpy(MsgOff[count], row[0], 12); //sid
        memcpy(MsgOff[count]+12, row[2], 20); //time
        memcpy(MsgOff[count]+32, row[1], 992); //msg

        printf("---in getMsg ---> %s\n", MsgOff[count]+12);

        count++;

    }

    bzero(query, sizeof(query));
    strcpy(query, "DELETE FROM message WHERE rid='");
    strcat(query, rid);
    strcat(query, "'");
    mysql_query(conn, query);


    mysql_free_result(result);
    mysql_close(conn);
    return count;
  //delete msg
}

int Alluser(){
  MYSQL *conn;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int num_fields;
  int i;
  int count = 0;
  //define 20 users array

  char query[200];
  strcpy(query, "SELECT uid FROM user");

  conn = mysql_init(NULL);
  mysql_real_connect(conn, "localhost", "rick", "secret", "chatdb", 0, NULL, 0);

  mysql_query(conn, query);

  result = mysql_store_result(conn);

  num_fields = mysql_num_fields(result);//1 


  while ((row = mysql_fetch_row(result))){

      strcpy(Auser[count], row[0]);
      count++;

  }
      mysql_free_result(result);
      mysql_close(conn);
      return count;

}
//uid and passwd
int Login(users *a){

  MYSQL *conn;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int num_fields;
  int i;

  char query[200];
  strcpy(query, "SELECT * FROM user WHERE uid ='");
  strcat(query, a->userid);
  strcat(query, "' AND passwd = '");
  strcat(query, a->passwd);
  strcat(query, "'");

  conn = mysql_init(NULL);
  mysql_real_connect(conn, "localhost", "rick", "secret", "chatdb", 0, NULL, 0);

  mysql_query(conn, query);

  result = mysql_store_result(conn);

  num_fields = mysql_num_fields(result);

//has some record
  if ((row = mysql_fetch_row(result)) != NULL){
      mysql_free_result(result);
      mysql_close(conn);
      return 1;

  }else{
      mysql_free_result(result);
      mysql_close(conn);
      return 0;
  }

}

int Register(users *a){

  MYSQL *conn;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int num_fields;
  int i;

  char query[200];
  strcpy(query, "SELECT * FROM user WHERE uid ='");
  strcat(query, a->userid);
  strcat(query, "'");

  conn = mysql_init(NULL);
  mysql_real_connect(conn, "localhost", "rick", "secret", "chatdb", 0, NULL, 0);

  mysql_query(conn, query);

  result = mysql_store_result(conn);

  num_fields = mysql_num_fields(result);

  if ((row = mysql_fetch_row(result)) != NULL){
      //already has some record 
      mysql_free_result(result);
      mysql_close(conn);
      return 0;

  }else{
    //add the reg info
      bzero(query, sizeof(query));
      strcpy(query, "INSERT INTO user VALUES('");
      strcat(query, a->userid);
      strcat(query, "','");
      strcat(query, a->passwd);
      strcat(query, "')");

      mysql_query(conn, query);

      mysql_free_result(result);
      mysql_close(conn);
      return 1;
  }

}
