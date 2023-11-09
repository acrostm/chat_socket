#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

typedef struct sockaddr *sockaddrp;
int sockfd;

void *recv_other(void *arg)
{
    char buf[255] = {};
    while (1)
    {
        int ret = recv(sockfd, buf, sizeof(buf), 0);
        if (ret == -1)
        {
            perror("recv error");
            return NULL;
        }
        printf("receive: %s\n", buf);
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

    while (1)
    {
        printf("input> ");
        scanf("%s", buf);
        int ret = send(sockfd, buf, strlen(buf), 0);
        if (ret == -1)
        {
            perror("send error");
            exit(EXIT_FAILURE);
        }

        if (0 == strcmp("quit", buf))
        {
            printf("[%s] 已经退出了聊天室\n", name);
            exit(EXIT_SUCCESS);
        }
    }
}
