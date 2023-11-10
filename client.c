#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

typedef struct sockaddr *sockaddrp;
int sockfd;

#define TRUE 1
#define FALSE 0

void *recv_other(void *arg)
{
    char buf[255] = {};
    while (TRUE)
    {
        int ret = recv(sockfd, buf, sizeof(buf), 0);
        if (ret == -1)
        {
            perror("recv error");
            return NULL;
        }

        // 使用 ANSI escape codes 清除当前行
        // printf("\033[2K"); 
        // printf("\033[1G"); // 光标回到行首
        printf("%s\n", buf);
        printf("< "); // 重新打印 <
        fflush(stdout); // 立即刷新输出缓冲区
    }
}

int main(int argc, char **argv)
{
    if (argc != 3 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        printf("Usage: ./%s <ip address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {AF_INET};
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    socklen_t addr_len = sizeof(addr);

    int ret = connect(sockfd, (sockaddrp)&addr, addr_len);
    if (ret == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char buf[255] = {};
    char name[255] = {};
    printf("Set your nickname: ");
    scanf("%s", name);
    printf("------------------------------------\n");
    printf("|||Welcome to chat room [%d] |||\n|||Type after \"< \" to seng message|||\n|||Type \"quit\" to leave room\t|||\n", getpid());
    printf("------------------------------------\n");
    ret = send(sockfd, name, strlen(name), 0);
    if (ret == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    pthread_t tid;
    ret = pthread_create(&tid, NULL, recv_other, NULL);
    if (ret == -1)
    {
        perror("pthread create failed");
        exit(EXIT_FAILURE);
    }

    while (TRUE)
    {
        printf("< ");
        scanf("%s", buf);
        int ret = send(sockfd, buf, strlen(buf), 0);
        if (ret == -1)
        {
            perror("send error");
            exit(EXIT_FAILURE);
        }

        if (strcmp("quit", buf) == 0)
        {
            printf("[%s] 已经退出了聊天室\n", name);
            exit(EXIT_SUCCESS);
        }
    }
}
