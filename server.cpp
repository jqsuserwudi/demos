
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include "prefix.h"

static const std::string i_host = "0.0.0.0";
static const int         i_port = 1120;

static int m_fd = 0;
static struct sockaddr_in client;

int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    printf("udp_output start send data \n");
    ::sendto(m_fd, buf, len, 0, (struct sockaddr*)&client, sizeof(client));
    return 0;
}

int main(int argc, char* argv[]){
    //int m_fd = 0;
    if ((m_fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("error_create socket \n");
        return -1;
    }
    
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(i_host.c_str());
    server.sin_port = htons(i_port);
    
    if (::bind(m_fd, (struct sockaddr*)&server, sizeof(server))<0){
        printf("error_bind port: %d \n", i_port);
        return -2;
    }
    //设置堵塞模式
    int flags;
    if ((flags = fcntl(m_fd, F_GETFL, NULL)) < 0) {
        return -3;
    }
    if (fcntl(m_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return -4;
    }
    //kcp init
    ikcpcb *kcp = ikcp_create(0x11223344, NULL);
    kcp->output = udp_output;
    // 配置窗口大小：平均延迟200ms，每20ms发送一个包，
    // 而考虑到丢包重发，设置最大收发窗口为128
    ikcp_wndsize(kcp, 128, 128);
    // 默认模式
    ikcp_nodelay(kcp, 0, 10, 0, 0);
    
    printf("waiting data fron client \n");
    char buff[2048] = {0};
    //struct sockaddr_in client;
    while(1){
        isleep(1);
        
        ikcp_update(kcp, iclock());
        int hr = ikcp_recv(kcp, buff, 10);
        if (hr>0){
            buff[hr] = 0;
            printf("received data from kcp = %s, len = %d \n", buff, hr);
            
            std::string back = "_backup";
            back = buff+back;
            printf("back client data = %s \n", back.c_str());
            //::sendto(m_fd, back.c_str(), back.length(), 0, (struct sockaddr*)&client, sizeof(client));
            ikcp_send(kcp, back.c_str(), back.length());
        }
        
        socklen_t len = sizeof(client);
        ssize_t r_len = ::recvfrom(m_fd, buff, sizeof(buff)-1, 0, (struct sockaddr*)&client, &len);
        if (r_len>0){
            buff[r_len] = 0;
            printf("[%s:%d] client data from udp = %s, %d \n",inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff, r_len);
            ikcp_input(kcp, buff, r_len);
            
            printf("\n");
        }
    }
    return 0;
}
