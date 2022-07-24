#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <iostream>
int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char recvBuff[1024] = {0};
    struct sockaddr_in serv_addr; 

    if(argc != 2)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    } 

    memset(recvBuff, 0,sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, 0, sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0) //写入地址
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 
    struct sockaddr_in client_addr; 
    socklen_t len = sizeof(client_addr);
    getsockname(sockfd, (struct sockaddr *)&client_addr, &len);
    snprintf(recvBuff, 1024, "client ip is %s, port is %d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    std::cout << recvBuff << std::endl;
    while ( (n = write(sockfd, recvBuff, strlen(recvBuff)) > 0))
    {
        sleep(1);
       // std::cout << "write n is " << n << std::endl;
    } 
    close(sockfd);
    if(n < 0)
    {
        printf("\n Read error \n");
    } 

    return 0;
}