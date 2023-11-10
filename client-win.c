#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>

#define MAX_NAME_LENGTH 255
#define MAX_BUF_LENGTH 255
#define TRUE 1
#define FALSE 0

SOCKET sockfd;
HANDLE hThread;

void *recv_other(void *arg)
{
    char buf[MAX_BUF_LENGTH] = {};
    while (TRUE)
    {
        int ret = recv(sockfd, buf, sizeof(buf), 0);
        if (ret == SOCKET_ERROR)
        {
            perror("recv error");
            return NULL;
        }

        printf("\r%s\n", buf);
        printf("< ");
        fflush(stdout);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: %s <ip address> <port>\n", argv[0]);
        return -1;
    }

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        perror("WSAStartup failed");
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET)
    {
        perror("socket");
        WSACleanup();
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    int ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR)
    {
        perror("connect");
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    char buf[MAX_BUF_LENGTH] = {};
    char name[MAX_NAME_LENGTH] = {};
    printf("Set your nickname: ");
    scanf("%s", name);
    printf("------------------------------------\n");
    printf("|||Welcome to chat room [%d] |||\n|||Type after \"< \" to send message|||\n|||Type \"quit\" to leave room \t |||\n", GetCurrentProcessId());
    printf("------------------------------------\n");
    ret = send(sockfd, name, strlen(name), 0);
    if (ret == SOCKET_ERROR)
    {
        perror("send error");
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recv_other, NULL, 0, NULL);
    if (hThread == NULL)
    {
        perror("CreateThread failed");
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    while (TRUE)
    {
        printf("< ");
        scanf("%s", buf);
        int ret = send(sockfd, buf, strlen(buf), 0);
        if (ret == SOCKET_ERROR)
        {
            perror("send error");
            closesocket(sockfd);
            WSACleanup();
            return -1;
        }

        // 在用户输入 "quit" 后设置标志，终止 recv_other 线程
        if (strcmp("quit", buf) == 0)
        {
            printf("[%s] has left the chat room\n", name);


            // 关闭线程句柄
            CloseHandle(hThread);

            closesocket(sockfd);
            WSACleanup();
            return 0;
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
