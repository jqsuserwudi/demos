
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <string.h>

static const std::string i_host = "192.168.1.8";
static const int         i_port = 1120;


int main(int argc, char* argv[]){
    int m_fd = 0;
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
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    inet_aton(ip, &server.sin_addr);
    server.sin_port = htons(i_port);
    
    char buff[2048] = {0};
    struct sockaddr_in peer;
    while(1){
        socklen_t len = sizeof(peer);
        printf("please enter data to send:");
        fflush(stdout);
        ssize_t s_len = ::read(0, buff, sizeof(buff)-1);
        
        if (s_len){
            buff[s_len-1] = 0;
            ::sendto(m_fd, buff, strlen(buff), 0, (struct sockaddr*)&server, sizeof(server));
            ssize_t r_len = ::recvfrom(m_fd, buff, sizeof(buff)-1, 0, (struct sockaddr*)&peer, &len);
            if (r_len>0){
                buff[r_len] = 0;
                printf("received data = %s \n", buff);
            }
        }
        printf("\n");
    }
    return 0;
}
