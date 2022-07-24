#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <iostream>
#define MAXCLINE 10
int clinet_fd_list[MAXCLINE]; //连接队列
int current_client_fd_nums = 0;
#define BUDDERSIZE 65536
char buf[BUDDERSIZE];
void showclient()
{
    int i;
    printf("client amount:%d\n", current_client_fd_nums);
    for(i=0;i<MAXCLINE;i++)
    {
        printf("[%d]:%d ",i,clinet_fd_list[i]);
    }
    printf("\n\n");
}
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

    int ret = 0; 

    int maxsock = listenfd;
    struct timeval tv;      
    int new_fd = -1;
    struct sockaddr_in client_addr; 
    socklen_t sin_size = sizeof(client_addr);
    //（文件描述符0、1、2），（stdin、stdout、stderr），（终端设备）这三者之间的关系？？？
    // clinet_fd_list[0] = 1;
    while(1)
    {
        // 必须每次select前设置下
        FD_ZERO(&fdsr);
        FD_SET(listenfd, &fdsr);
        //将所有的连接全部加到这个这个集合中，可以监测客户端是否有数据到来
        for(int i = 0; i < MAXCLINE; i++) 
        {
            if(clinet_fd_list[i] != 0)
            {
                // 必须每次select前设置下
                FD_SET(clinet_fd_list[i], &fdsr);
            }
        }    
        //超时的设定，这里也可以不需要设置时间，将这个参数设置为NULL,表明此时select为阻塞模式
        // 必须每次select前设置下
        tv.tv_sec = 30;
        tv.tv_usec =0;
        // int select(int maxfd, fd_set *readfds, fd_set *writefds,
        // fd_set *exceptfds, struct timeval *timeout);
        //如果文件描述符中有连接请求 会做相应的处理，实现I/O的复用 多用户的连接通讯
        //这里指定了想要接收的fd
        ret = select(maxsock + 1, &fdsr, NULL,NULL, &tv);
        if(ret < 0) //没有找到有效的连接 失败
        {
            perror("select error!\n");
            break;
        }
        else if(ret ==0)// 指定的时间到
        {
            printf("timeout \n");
            continue;
        }
        //下面这个循环是非常必要的，因为你并不知道是哪个连接发过来的数据，所以只有一个一个去找。
        for(int i=0; i < MAXCLINE; i++) {
            if(FD_ISSET(clinet_fd_list[i],&fdsr))
            {
                ret = recv(clinet_fd_list[i],buf,sizeof(buf),0);
                // 解阻塞与非阻塞recv返回值没有区分，都是
                // <0 出错
                // =0 连接关闭
                // >0 接收到数据大小，
                //如果客户端主动断开连接，会进行四次挥手，会出发一个信号，此时相应的套接字会有数据返回，告诉select，我的客户断开了，你返回-1
                
                if(ret <=0) //客户端连接关闭，清除文件描述符集中的相应的位
                {
                    printf("client[%d] close\n",i);
                    close(clinet_fd_list[i]);
                    current_client_fd_nums--;
                    FD_CLR(clinet_fd_list[i], &fdsr);
                    clinet_fd_list[i] = 0;                
                }
                //否则有相应的数据发送过来 ，进行相应的处理
                else
                {
                    if(ret < BUDDERSIZE)
                        memset(&buf[ret],'\0', 1);
                    printf("client[%d] send:%s\n",i,buf);
                }
            }
        } 
        if(FD_ISSET(listenfd, &fdsr)) { //代表listen有连接过来了 
            new_fd = accept(listenfd,(struct sockaddr *)&client_addr, &sin_size);
            if(new_fd <=0)
            {
                perror("accept error\n");
                continue;
            }
            if (current_client_fd_nums < MAXCLINE) {
                for(int i = 0; i < MAXCLINE; i++)
                {
                    if(clinet_fd_list[i]==0)
                    {
                        clinet_fd_list[i] = new_fd;
                        current_client_fd_nums++;
                        printf("new connection client[%d]%s:%d\n",current_client_fd_nums,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
                        if(new_fd > maxsock)
                        {
                            maxsock = new_fd;
                        }
                        break;
                    }
                }
            }
            else
            {
                printf("max connections arrive ,exit\n");
                close(new_fd);
                continue;
            }
        }
        showclient();
    }
    for(int i=0;i<MAXCLINE;i++)
    {
        if(clinet_fd_list[i]!=0)
        {
            close(clinet_fd_list[i]);
        }
    }  
    close(listenfd);
    exit(0);
}