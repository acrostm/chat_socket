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
    int fd = createSocket();
    if (fd == -1)
    {
        fprintf(stderr, "Failed to create socket.\n");
        exit(EXIT_FAILURE);
    }

    // 连接服务器
    unsigned short port = (unsigned short)atoi(argv[1]);
    if (connectToHost(fd, "127.0.0.1", port) == -1)
    {
        fprintf(stderr, "Failed to connect to server.\n");
        closeSocket(fd);
        exit(EXIT_FAILURE);
    }

    // 发送消息
    if (sendMsg(fd, "Hello from client!", strlen("Hello from client!")) == -1)
    {
        fprintf(stderr, "Failed to send message.\n");
        closeSocket(fd);
        exit(EXIT_FAILURE);
    }
    printf("Message sent successfully.\n");

    // 接收文件
    char *file_buffer;
    if (recvMsg(fd, &file_buffer) == -1)
    {
        fprintf(stderr, "Failed to receive file.\n");
        closeSocket(fd);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen("received_file.txt", "w");
    if (!file)
    {
        fprintf(stderr, "Failed to open file for writing.\n");
        closeSocket(fd);
        exit(EXIT_FAILURE);
    }

    fwrite(file_buffer, 1, strlen(file_buffer), file);
    fclose(file);
    printf("File received successfully.\n");

    // 关闭套接字
    closeSocket(fd);
    free(file_buffer);

    exit(EXIT_SUCCESS);
}
