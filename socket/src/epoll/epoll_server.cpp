#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h> /* poll function */
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <iostream>
#include <fcntl.h>
#include <sys/epoll.h>
#define MAXLISTEN 256
void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}
int main(int argc, char *argv[])
{
    int listenfd = 0;
    struct sockaddr_in serv_addr; 

    char recvBuff[1025];
    time_t ticks; 
    // SOCK_STREAM is reliable, applicable to tcp
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(recvBuff, '0', sizeof(recvBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // socket INADDR_ANY 监听0.0.0.0地址 socket只绑定端口让路由表决定传到哪个ip

    serv_addr.sin_port = htons(5000);  //端口5000

    // 将serv_addr信息绑定到listenfd
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); //开启listenfd的监听，等待其他连接
    setnonblocking(listenfd);
    struct sockaddr_in client_addr; 
    socklen_t sin_size = sizeof(client_addr);
    //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
    int epfd;
    epfd = epoll_create(MAXLISTEN);//监听数目为256
    struct epoll_event ev;
    ev.data.fd = listenfd;
    ev.events = EPOLLIN|EPOLLET; //listenfd特别适合边沿触发：状态变化的时候
    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
    int nfds;
    int i;
    int connfd;
    int ret;
    struct epoll_event client_events[20];
    while(1) {
        nfds = epoll_wait(epfd, client_events,20,500);     
        for(i=0;i < nfds;++i) {
            if(client_events[i].data.fd == listenfd)//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            {
                connfd = accept(listenfd,(sockaddr *)&client_addr, &sin_size);
                if(connfd<0){
                    perror("connfd<0");
                    exit(1);
                }
                setnonblocking(connfd);
                ev.data.fd=connfd;
                //设置用于注测的读操作事件

                ev.events=EPOLLIN;
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
            }
            else if(client_events[i].events & EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。
            {
                ret = recv(client_events[i].data.fd, recvBuff,sizeof(recvBuff),0);
                // 解阻塞与非阻塞recv返回值没有区分，都是
                // <0 出错
                // =0 连接关闭
                // >0 接收到数据大小，
                //如果客户端主动断开连接，会进行四次挥手，会出发一个信号，此时相应的套接字会有数据返回，告诉select，我的客户断开了，你返回-1
                
                if(ret <=0) //客户端连接关闭，清除文件描述符集中的相应的位
                {
                    printf("client[%d] close\n",i);
                    close(client_events[i].data.fd);
                }
                //否则有相应的数据发送过来 ，进行相应的处理
                else
                {
                    if(ret < 1025)
                        memset(&recvBuff[ret],'\0', 1);
                    printf("client[%d] send:%s\n",i, recvBuff);
                }
            }
        }
    }
    close(listenfd);
    exit(0);
}