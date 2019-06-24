
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <string.h>

static const std::string i_host = "0.0.0.0";
static const int         i_port = 1120;

int main(int argc, char* argv[]){
    int m_fd = 0;
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
    
    printf("waiting data fron client \n");
    char buff[2048] = {0};
    struct sockaddr_in client;
    while(1){
        socklen_t len = sizeof(client);
        ssize_t r_len = ::recvfrom(m_fd, buff, sizeof(buff)-1, 0, (struct sockaddr*)&client, &len);
        if (r_len>0){
            buff[r_len] = 0;
            printf("[%s:%d] client data = %s \n",inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);
            
            std::string back = "_backup";
            back = buff+back;
            printf("back client data = %s \n", back.c_str());
            ::sendto(m_fd, back.c_str(), back.length(), 0, (struct sockaddr*)&client, sizeof(client));
            
            printf("\n");
        }
    }
    return 0;
}
