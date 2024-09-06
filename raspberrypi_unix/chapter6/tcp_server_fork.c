#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
//#include <sys/wait.h>
#include <arpa/inet.h>
#include <signal.h>

#define TCP_PORT 5100 /*서버의 포트 번호*/

static int g_noc = 0; // number of child

void sigfunc(int no)
{
    printf("Signal : %d(%d)\n", no, g_noc);
    if(--g_noc == 0) exit(0);
}

int main(int argc, char **argv)
{
    int ssock, portno, n;
    socklen_t clen;  /*소켓 디스크립터 정의 */
    pid_t pid;
    struct sockaddr_in servaddr, cliaddr; /*주소 구조체 정의 */
    char mesg[BUFSIZ]; 

    portno = (argc == 2)?atoi(argv[1]):TCP_PORT;
    signal(SIGCHLD, sigfunc);

    /*서버 소켓 생성*/
    if((ssock = socket(AF_INET, SOCK_STREAM, 0))<0) {
        perror("socket()");
        return -1;
    }

    /*주소 구조체에 주소 지정 */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port = htons(portno);

    if(bind(ssock,(struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind()");
        return -1;
    }
    /*대기하는 클라이언트 숫자 설정. */
    if(listen(ssock, 8) < 0){
        perror("listen()");
        return -1;
    }
    clen=sizeof(cliaddr);
    do {
        int n, csock = accept(ssock,(struct sockaddr *)&cliaddr, &clen);
        /*클라이언트를 문자열로 변경 */
        
        inet_ntop(AF_INET, &cliaddr.sin_addr,mesg,BUFSIZ);
        printf("Client is connected : %s\n",mesg);
        
        g_noc++;

        if((pid=fork())<0){
            perror("Error");

        }else if (pid==0){
            close(ssock); /*서버 소켓을 닫음*/  
            do{
                memset(mesg, 0, BUFSIZ);
                if ((n=read(csock, mesg, BUFSIZ))<=0)
                     perror("read()");
                     printf("Received data : %s",mesg);


                /*클라이언트로 buf에 있는 문자열 전송 */
                if(write(csock, mesg, n) <=0)
                    perror("write()");
            } while(strncmp(mesg, "q", 1));
    
        close(csock);/* 클라이언트 소켓을 닫음*/
        exit(0);
        
        }
        //wait();
        close(csock);
    }while(strncmp(mesg, "q", 1)); // 부모쪽엔 영향이 없음. 자식은 q로 인해 종료됨. 부모는 무한루프 

    close(ssock);/*서버 소켓을 닫음 */
    return 0;
}
