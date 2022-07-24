#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <iostream>
int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1025];
    time_t ticks; 
    // SOCK_STREAM is reliable, applicable to tcp
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // socket INADDR_ANY 监听0.0.0.0地址 socket只绑定端口让路由表决定传到哪个ip

    serv_addr.sin_port = htons(5000);  //端口5000

    // 将serv_addr信息绑定到listenfd
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); //开启listenfd的监听，等待其他连接
    char addr_p[1000] = {0};//用来保存客户端ip信息
    fd_set fdsr; //文件描述符集的定义
    while(1)
    {
        FD_ZERO(&fdsr);
        FD_SET(listenfd,&fdsr);
        
        struct sockaddr_in client_addr; 
        socklen_t len = sizeof(client_addr);
        std::cout << "wait connect" << std::endl;
        connfd = accept(listenfd, (struct sockaddr*)&client_addr, &len); //得到连接
        inet_ntop(AF_INET,&client_addr.sin_addr,addr_p,sizeof(addr_p));
        std::cout << "get connect, client_addr ip is " << addr_p << std::endl;
        std::cout << "get connect, port ip is " << ntohs(client_addr.sin_port) << std::endl;
        snprintf(sendBuff, sizeof(sendBuff), "server to client is start");
        write(connfd, sendBuff, strlen(sendBuff));
        close(connfd);
        sleep(1);
     }
     close(listenfd);
}