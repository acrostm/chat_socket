#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include "socket_c.h"


// 创建监套接字
int createSocket()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        perror("socket");
        return -1;
    }
    printf("|Socket create success, fd=%d|\n", fd);
    return fd;
}

// 绑定本地的IP和端口
int bindSocket(int lfd, unsigned short port)
{
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = INADDR_ANY;  // 0 = 0.0.0.0
    int ret = bind(lfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if(ret == -1)
    {
        perror("bind");
        return -1;
    }
    printf("|Socket bind success, ip: %s, port: %d|\n",
           inet_ntoa(saddr.sin_addr), port);
    return ret;
}

// 设置监听
int setListen(int lfd)
{
    int ret = listen(lfd, 128);
    if(ret == -1)
    {
        perror("listen");
        return -1;
    }
    printf("|Listening to port...|\n");
    return ret;
}

// 阻塞并等待客户端的连接
int acceptConn(int lfd, struct sockaddr_in *addr)
{
    int cfd = -1;
    if(addr == NULL)
    {
        cfd = accept(lfd, NULL, NULL);
    }
    else
    {
        unsigned int addrlen = sizeof(struct sockaddr_in);
        cfd = accept(lfd, (struct sockaddr*)addr, &addrlen);
    }
    if(cfd == -1)
    {
        perror("accept");
        return -1;
    }       
    printf("|Connected to client...|\n");
    return cfd; 
}

// 接收数据
int recvMsg(int cfd, char** msg)
{
    if(msg == NULL || cfd <= 0)
    {
        return -1;
    }
    // 接收数据
    // 1. 读数据头
    int len = 0;
    readn(cfd, (char*)&len, 4);
    len = ntohl(len);
    printf("|Data Size: %d|\n", len);

    // 根据读出的长度分配内存
    char *buf = (char*)malloc(len+1);
    int ret = readn(cfd, buf, len);
    if(ret != len)
    {
        return -1;
    }
    buf[len] = '\0';
    *msg = buf;

    return ret;
}

// 发送数据
int sendMsg(int cfd, const char* msg, int len)
{
   if(msg == NULL || len <= 0)
   {
       return -1;
   }
   // 申请内存空间: 数据长度 + 包头4字节(存储数据长度)
   char* data = (char*)malloc(len+4);
   int bigLen = htonl(len);
   memcpy(data, &bigLen, 4);
   memcpy(data+4, msg, len);
   // 发送数据
   int ret = writen(cfd, data, len+4);
   return ret;
}

// 连接服务器
int connectToHost(int fd, const char* ip, unsigned short port)
{
    // 2. 连接服务器IP port
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &saddr.sin_addr.s_addr);
    int ret = connect(fd, (struct sockaddr*)&saddr, sizeof(saddr));
    if(ret == -1)
    {
        perror("connect");
        return -1;
    }
    printf("|Connected to server...|\n");
    return ret;
}

// 关闭套接字
int closeSocket(int fd)
{
    int ret = close(fd);
    if(ret == -1)
    {
        perror("close");
    }
    return ret;
}

// 接收指定的字节数
// 函数调用成功返回 size
int readn(int fd, char* buf, int size)
{
    int nread = 0;
    int left = size;
    char* p = buf;

    while(left > 0)
    {
        if((nread = read(fd, p, left)) > 0)
        {
            p += nread;
            left -= nread;
        }
        else if(nread == -1)
        {
            return -1;
        }
    }
    return size;
}

// 发送指定的字节数
// 函数调用成功返回 size
int writen(int fd, const char* msg, int size)
{
    int left = size;
    int nwrite = 0;
    const char* p = msg;

    while(left > 0)
    {
        if((nwrite = write(fd, msg, left)) > 0)
        {
            p += nwrite;
            left -= nwrite;
        }
        else if(nwrite == -1)
        {
            return -1;
        }
    }
    return size;
}
