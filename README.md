# Chat Room
基于Linux socket的局域网聊天软件

## 设计思想
项目设计之初，选择把server端作为聊天消息的转发端，但这样做会限制server端的性能，且增加了聊天的延时。为了降低server的工作量，我们采用P2P聊天的思想，把server端的逻辑只限定在用户管理（注册和登录）和离线消息上，在线聊天和文件传输逻辑，都放在client端。

## 用法
```bash
git clone https://github.com/hackeryard/ChatRoom.git  
cd ChatRoom/ChatRoom  
make && make server
```

## 客户端
```bash
./client 本地IP 本地端口
```

## 服务器
```bash
./server
```

## 说明
服务器IP Port在宏定义里面，修改后编译即可

客户端输入的IP是局域网IP，端口是需要用作TCP/UDP bind的端口

## 调试
- gdb调试，尤其用指针比较多的时候
- gdb attach， 可以动态调试服务器和客户端
- 出现段错误，调试core文件
- tcpdump打印网络流量
- 善用errno，打印错误信息
