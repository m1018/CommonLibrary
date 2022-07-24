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
#define OPEN_MAX 20
int main(int argc, char *argv[])
{
    int listenfd = 0;
    struct sockaddr_in serv_addr; 
    struct pollfd  client[OPEN_MAX];

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
    char addr_p[1000] = {0};//用来保存客户端ip信息
    client[0].fd = listenfd;
    client[0].events = POLLIN;
    for (int i = 1; i < OPEN_MAX; i++)
        client[i].fd = -1;      /* -1 indicates available entry */
    struct sockaddr_in client_addr; 
    socklen_t sin_size = sizeof(client_addr);
    int maxi = 0;
    int nready;
    int new_fd;
    int i;
    int sockfd;
    int ret;
    while(1)
    {
        //timeout：是poll函数调用阻塞的时间，单位：毫秒；
        nready = poll(client, OPEN_MAX, 1000U);//maxi表示client数组大小
        std::cout << "nready is " << nready << std::endl; 
        if (client[0].revents & POLLIN) {  
            new_fd = accept(listenfd,(struct sockaddr *)&client_addr, &sin_size);
 
            for (i = 1; i < OPEN_MAX; i++)//监视connfd是否可读、可写
                if (client[i].fd < 0) {
                    client[i].fd = new_fd;  /* save descriptor */
                    break;
                }
            if (i == OPEN_MAX) {
                printf("too many clients\n");
                close(new_fd);
            } else {
                client[i].events = POLLIN;//检测connfd是否可读
                if (i > maxi)
                    maxi = i;               /* max index in client[] array */
            }

            if (--nready <= 0)/*如果除了listen的client[0]被激活，其他事件没有没有被激活则nready是1
                *自减1后，为0，表示此次处理poll结束。继续下次监视。
                */
                continue;               /* no more readable descriptors */
        }
        for (i = 1; i <= maxi; i++) {    /* 第0个元素是处理listen的，处理其余accept的所有可读的connfd */
            if ( (sockfd = client[i].fd) < 0)//无效的fd
                continue;
            if (client[i].revents & (POLLIN)) {//处理可读的connfd
                ret = recv(client[i].fd, recvBuff,sizeof(recvBuff),0);
                // 解阻塞与非阻塞recv返回值没有区分，都是
                // <0 出错
                // =0 连接关闭
                // >0 接收到数据大小，
                //如果客户端主动断开连接，会进行四次挥手，会出发一个信号，此时相应的套接字会有数据返回，告诉select，我的客户断开了，你返回-1
                
                if(ret <=0) //客户端连接关闭，清除文件描述符集中的相应的位
                {
                    printf("client[%d] close\n",i);
                    close(client[i].fd);
                    client[i].fd = -1;                
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
    for(int i=0;i<OPEN_MAX;i++)
    {
        if(client[i].fd!=-1)
        {
            close(client[i].fd);
        }
    }  
    close(listenfd);
    exit(0);
}