#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct sockaddr *sockaddrp;

struct sockaddr_in src_addr[50];
socklen_t src_len = sizeof(src_addr[0]);

int confd[50] = {};
int count = 0;

void *broadcast(void *indexp)
{
    int index = *(int *)indexp;
    char buf_rcv[255] = {};
    char buf_snd[255] = {};
    char name[20] = {};
    int ret = recv(confd[index], name, sizeof(name), 0);
    if (ret == -1)
    {
        perror("recv");
        close(confd[index]);
        return NULL;
    }

    while (1)
    {
        bzero(buf_rcv, sizeof(buf_rcv));
        recv(confd[index], buf_rcv, sizeof(buf_rcv), 0);

        if (0 == strcmp("quit", buf_rcv))
        {
            sprintf(buf_snd, "%s已经退出悟空聊天室", name);
            for (int i = 0; i <= count; i++)
            {
                if (i == index || 0 == confd[i])
                {
                    continue;
                }

                send(confd[i], buf_snd, strlen(buf_snd), 0);
            }
            confd[index] = -1;
            pthread_exit(0);
        }

        // sprintf(buf_snd, "%s:%s", name, buf_rcv);
        snprintf(buf_snd, sizeof(buf_snd), "%s:%s", name, buf_rcv);
        printf("%s\n", buf_snd);
        for (int i = 0; i <= count; i++)
        {
            if (i == index || 0 == confd[i])
            {
                continue;
            }

            send(confd[i], buf_snd, sizeof(buf_snd), 0);
        }
    }
}

int main(int argc, char **argv)
{
    if(argc != 3 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
    {
        printf("Usage: ./%s <ip address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("chat server running!\n");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket descriptor creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {AF_INET};
    socklen_t addr_len = sizeof(addr);

    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    int ret = bind(sockfd, (sockaddrp)&addr, addr_len);
    if (ret == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 50);

    int index = 0;

    while (count <= 50)
    {
        confd[count] = accept(sockfd, (sockaddrp)&src_addr[count], &src_len);
        ++count;
        index = count - 1;

        pthread_t tid;
        int ret = pthread_create(&tid, NULL, broadcast, &index);
        if (ret == -1)
        {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }
}
