#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define TCP_PORT 5100


int main(int argc, char** argv)
{
    int sockfd;
    fd_set readfds;
    char mesg[BUFSIZ];
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(argc < 2){
        printf("Usage : %s IP_ADDRESS\n", argv[0]);
        return -1;
    }
  
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket()");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &(servaddr.sin_addr.s_addr));
    servaddr.sin_port = htons(TCP_PORT);

    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) > 0){
        perror("connect()");
        return -1;
    }

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(sockfd, &readfds);

        select(sockfd + 1, &readfds, NULL, NULL, NULL);
    
        if(FD_ISSET(0, &readfds)){ // 키보드 입력 
            int size = read(0, mesg, BUFSIZ);
            if(strncmp(mesg,"q", 1) == 0) 
                break;
            write(sockfd, mesg, size); // 서버로 보냄
        }

        if(FD_ISSET(sockfd, &readfds)){
            int size = read(sockfd, mesg, BUFSIZ);
            if(size == 0) break; // 서버가 끊어지면 0을 반환하기때문에 
            write(1, mesg, size); 
        }
    
    } 

    close(sockfd);
}

