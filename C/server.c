#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include "socket_c.h"

int main(int argc, char *argv[])
{
    if (argc != 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 创建套接字
    int lfd = createSocket();
    if (lfd == -1)
    {
        fprintf(stderr, "Failed to create socket.\n");
        exit(EXIT_FAILURE);
    }

    // 绑定套接字到指定端口
    unsigned short port = (unsigned short)atoi(argv[1]);
    if (bindSocket(lfd, port) == -1)
    {
        fprintf(stderr, "Failed to bind socket.\n");
        closeSocket(lfd);
        exit(EXIT_FAILURE);
    }

    // 设置监听
    if (setListen(lfd) == -1)
    {
        fprintf(stderr, "Failed to set listening.\n");
        closeSocket(lfd);
        exit(EXIT_FAILURE);
    }

    // 等待客户端连接
    struct sockaddr_in client_addr;
    int cfd = acceptConn(lfd, &client_addr);
    if (cfd == -1)
    {
        fprintf(stderr, "Failed to accept connection.\n");
        closeSocket(lfd);
        exit(EXIT_FAILURE);
    }

    // 接收消息
    char *msg;
    if (recvMsg(cfd, &msg) == -1)
    {
        fprintf(stderr, "Failed to receive message.\n");
        closeSocket(cfd);
        closeSocket(lfd);
        exit(EXIT_FAILURE);
    }
    printf("Received message from client: %s\n", msg);
    free(msg);

    // 发送文件
    FILE *file = fopen("example.txt", "r");
    if (!file)
    {
        fprintf(stderr, "Failed to open file.\n");
        closeSocket(cfd);
        closeSocket(lfd);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_buffer = (char *)malloc(file_size);
    fread(file_buffer, 1, file_size, file);

    if (sendMsg(cfd, file_buffer, file_size) == -1)
    {
        fprintf(stderr, "Failed to send file.\n");
        closeSocket(cfd);
        closeSocket(lfd);
        free(file_buffer);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    printf("File sent successfully.\n");

    // 关闭连接和套接字
    closeSocket(cfd);
    closeSocket(lfd);
    free(file_buffer);
    fclose(file);

    exit(EXIT_SUCCESS);
}
