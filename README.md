# Chat-Room-v0.1
## 用法
> git clone https://github.com/hackeryard/Chat-Room-v0.1.git

> cd Chat-Room-v0.1/ChatRoom

> make && make sernew

## 客户端
> ./client 本地IP 本地端口

## 服务器
> ./sernew

## 说明
服务器ip port在宏定义里面，修改后编译即可

客户端输入的IP是局域网IP，端口是需要用作TCP/UDP bind的端口

## 调试
- gdb调试，尤其用指针比较多的时候
- gdb attach， 可以动态调试服务器和客户端
- 出现段错误，调试core文件
- tcpdump打印网络流量
- 善用errno，打印错误信息
