
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include "prefix.h"

static const std::string i_host = "192.168.1.8";
static const int         i_port = 1120;

static int m_fd = 0;
static struct sockaddr_in server;

int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    printf("udp_output start send data \n");
    ::sendto(m_fd, buf, len, 0, (struct sockaddr*)&server, sizeof(server));
    return 0;
}

int main(int argc, char* argv[]){
    //int m_fd = 0;
    if ((m_fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("error_create socket \n");
        return -1;
    }
    struct hostent* he = gethostbyname(i_host.c_str());
    if (he==NULL){
        printf("error_analyzing host = %s  \n", i_host.c_str());
        return -2;
    }
    char ip[32];
    inet_ntop(he->h_addrtype, he->h_addr, ip, sizeof(ip));
    
    //设置堵塞模式
    int flags;
    if ((flags = fcntl(m_fd, F_GETFL, NULL)) < 0) {
        return -3;
    }
    if (fcntl(m_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return -4;
    }
    
    //struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    inet_aton(ip, &server.sin_addr);
    server.sin_port = htons(i_port);
    
    //kcp init
    ikcpcb *kcp = ikcp_create(0x11223344, NULL);
    kcp->output = udp_output;
    // 配置窗口大小：平均延迟200ms，每20ms发送一个包，
    // 而考虑到丢包重发，设置最大收发窗口为128
    ikcp_wndsize(kcp, 128, 128);
    // 默认模式
    ikcp_nodelay(kcp, 0, 10, 0, 0);
    
    /*
    // 启动快速模式
    // 第二个参数 nodelay-启用以后若干常规加速将启动
    // 第三个参数 interval为内部处理时钟，默认设置为 10ms
    // 第四个参数 resend为快速重传指标，设置为2
    // 第五个参数 为是否禁用常规流控，这里禁止
    ikcp_nodelay(kcp, 1, 10, 2, 1);
    kcp->rx_minrto = 10;
    kcp->fastresend = 1;
    */
    
    char buff[2048];
    memcpy(buff, "aaa", sizeof("aaa"));
    struct sockaddr_in peer;
    int count = 0;
    while(1){
        isleep(1);
        while(1){
            socklen_t len = sizeof(peer);
            ssize_t r_len = ::recvfrom(m_fd, buff, sizeof(buff)-1, 0, (struct sockaddr*)&peer, &len);
            if (r_len>0){
                buff[r_len] = 0;
                ikcp_input(kcp, buff, r_len);
                printf("received data from udp = %s \n", buff);
            }
            else{
                break;
            }
        }
        while(1){
            ikcp_update(kcp, iclock());
            int hr = ikcp_recv(kcp, buff, 10);
            if (hr>0){
                buff[hr] = 0;
                printf("received data from kcp = %s \n\n", buff);
            }
            else{
                break;
            }
        }
        
        
        count += 1;
        if (count>=10000){
            count = 0;
            memcpy(buff, "aaa", sizeof("aaa"));
            printf("start data to send:");
            ikcp_send(kcp, buff, (int)strlen(buff));
        }
    }
    return 0;
}



