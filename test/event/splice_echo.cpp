//
// Created by weiwei on 2020/10/30.
//

#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<fcntl.h>
//#include <bits/fcntl-linux.h>

#define LEN 655

int main(int argc,char *argv[])
{
    if(argc < 3)
    {
        printf("usage: %s ip port\n",argv[0]);
        exit(1);
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd,connfd;
    struct sockaddr_in sockaddr,connaddr;
    socklen_t connaddr_len = sizeof(connaddr);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    inet_pton(AF_INET,ip,&sockaddr.sin_addr);

    sockfd = socket(AF_INET,SOCK_STREAM,0);

    int ret = bind(sockfd,(struct sockaddr*)&sockaddr,sizeof(sockaddr));
    if(ret == -1)
    {
        perror("bind error");
        exit(1);
    }

    listen(sockfd,15);

    connfd = accept(sockfd,(struct sockaddr*)&connaddr,&connaddr_len);
    if(connfd == -1)
    {
        perror("accept error");
        exit(1);
    }

    int pipefd[2];
    pipe(pipefd);
    while(true)
    {
        //用splice函数的回射服务

        int n = splice(connfd,NULL,pipefd[1],NULL,LEN,SPLICE_F_MORE);
        if(n > 0)
        {
            splice(pipefd[0],NULL,connfd,NULL,n,SPLICE_F_MORE);
        }
        else if(n == 0)
        {
            printf("client close\n");
            close(pipefd[0]);
            close(pipefd[1]);
            close(connfd);
            close(sockfd);
            break;
        }
        else{
            perror("splice error");
            exit(1);
        }

        /*
        //用read和write函数的回射服务
        char buf[BUFSIZ];
        int n = read(connfd,buf,sizeof(buf));
        if(n > 0)
        {
            write(connfd,buf,n);
        }
        else if(n == 0)
        {
            printf("client close\n");
            close(connfd);
            close(sockfd);
            break;
        }
        else{
            perror("read error");
            exit(1);
        }
        */
    }
    return 0;
}