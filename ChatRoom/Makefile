.PHONY: server
client:	client.c msg.h
	gcc -o client client.c msg.h -g

server:	
	gcc server.c msg.h sql.h sql.c lists.h lists.c -lmysqlclient -I/usr/include/mysql -o server -g