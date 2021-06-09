#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 8192

void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_print(char * msg);

char msg[BUF_SIZE];

//서버로 텍스트 송신
void* send_msg(void* arg){
  int sock = *((int*)arg);  //클라이언트 소켓 FD
  char msg[BUF_SIZE];

  while(1){ //Command line에서 문자열을 입력받아 서버에 전송.
    fgets(msg, BUF_SIZE, stdin);

    if(!strcmp(msg, "EXIT\n") || !strcmp(msg, "exit\n")){
        //클라이언트 소캣 종료
        close(sock);
        //프로그램 종료
        exit(1);
    }
    //null 문자 제외하고 서버로 문자열 보냄
    write(sock, msg , strlen(msg));
  }

  return NULL;
}

//서버로부터 텍스트 수신
void* recv_msg(void* arg){

  //클라이언트의 파일 디스크립터
  int sock = *((int*)arg);
  char msg[BUF_SIZE];
  int str_len;

  while (1) {
    str_len = read(sock, msg, BUF_SIZE - 1);
    //read 실패시
    if(str_len == -1)
      return NULL;

    msg[str_len] = '\0';
    printf("\n[current source code]");
    //콘솔에 출력
    system("clear");
    printf("\n%s\n", msg);
    memset(msg, 0, BUF_SIZE);
    printf("\nexit or EXIT = process exit\ninput text : ");
  }

  return NULL;
}


//에러 처리
void error_print(char *msg)
{
  fputs(msg, stderr);
  fputc('\n', stderr);
  exit(1);
}
