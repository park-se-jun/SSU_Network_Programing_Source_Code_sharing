#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#define BUF_SIZE 8192
#define MAX_CLIENT 10

void *service_client(void* arg);
void send_msg(char* msg, int len);
void error_print(char * message);
int init_source();
void modify_source(char *msg);

int client_count = 0; //서버에 접속한 클라이언트 수
int client_sockets[MAX_CLIENT]; //서버에 접속한 클라이언트들의 Socket FD를 저장한 배열
pthread_mutex_t mutex;
char *source; //쓰레드들이 공유할 메모리
int source_fd, curr_src = 0, curr_low = 0;

//연결된 클라이언트에 대해 처리하는 쓰레드
void* service_client(void* arg){
  int client_socket = *((int*)arg);
  int str_len = 0, i;
  char msg[BUF_SIZE];
  memset(msg, 0, BUF_SIZE);

  printf("Guest %d - Service_client run\n" , client_socket);

  //클라이언트로부터 데이터를 수신받음. 루프를 빠져나가면 연결이 종료된 것으로 간주함.
  while ( (str_len = read(client_socket , msg , sizeof(msg))) != 0) {
    printf("Guest %d send text %d byte\n", client_socket, str_len);

    //소스코드 클린 명령
    if(!strcmp(msg, "&CLEAR&")){
      printf("Guest %d request to source code clean\n", client_socket);
      memset(source, 0, BUF_SIZE);
      curr_src = 0;
      curr_low = 0;
      continue;
    }

    //소스코드 수정 명령
    else if(!strcmp(msg, "&MODIFY&")){
      str_len = read(client_socket, msg, sizeof(msg));
      msg[str_len] = '\0';
      modify_source(msg);
      curr_src = strlen(source);
      printf("Guest %d modify source code\n", client_socket);
      printf("Current source code is %d byte, %d low\n", curr_src, curr_low);
      send_msg(source, curr_src);
      continue;
    }

    msg[str_len] = '\n';
    memcpy(source + curr_src, msg, str_len);
    curr_src += str_len;
    curr_low++;
    printf("Current source code is %d byte, %d lows\n", curr_src, curr_low);
    send_msg(source, curr_src);  //send_msg 함수 호출
  }

  printf("Guest %d is disconnected\n", client_socket);
  pthread_mutex_lock(&mutex); //뮤텍스 lock

  //disconnected 된 클라이언트 삭제
  for(i = 0; i < client_count; i++){
      //현재 해당하는 파일 디스크립터를 찾으면
    if(client_socket == client_sockets[i]){
        //클라이언트가 연결요청을 했으므로 해당 정보를 덮어씌워 삭제
        while (i < client_count -1){
          client_sockets[i] = client_sockets[i+1];
          i++;
        }
        break;
    }
  }

  client_count--; //클라이언트 수 감소
  pthread_mutex_unlock(&mutex); //뮤텍스 unlock
  close(client_socket); //클라이언트와의 송수신을 위한 생성했던 소켓종료
  printf("Current client : %d\n" , client_count);
  return NULL;
}

//연결되어 있는 모든 클라이언트들에게 소스코드의 내용을 전송.
void send_msg(char *msg, int len){
  int i;

  pthread_mutex_lock(&mutex);      //뮤텍스 lock
  for (i = 0; i < client_count; i++) {
      //현재 연결된 모든 클라이언트에게 메시지 전송
      write(client_sockets[i], msg, len);
  }
  pthread_mutex_unlock(&mutex);    //뮤텍스 unlock

}

//에러 메시지 전송
void error_print(char *message){
  fputs(message , stderr);
  fputc('\n' , stderr);
  exit(-1);
}

//공유 메모리 할당 및 초기화
int init_source(){
  source_fd = shm_open("/example", O_RDWR|O_CREAT|O_TRUNC, 0777);
  ftruncate(source_fd, BUFSIZ);

  if(source_fd == -1){
  	close(source_fd);
  	return -1;
  }

  source = (char *) mmap(NULL, BUFSIZ, PROT_READ|PROT_WRITE, MAP_SHARED, source_fd, 0);
  memset(source, 0, BUFSIZ);
  close(source_fd);
  return 0;
}

void modify_source(char *msg){ //소스코드 수정
  int line = atoi(msg); //몇 번째 라인을 수정할 것인지를 저장.
  int i = 0, modify_next;
  char modify_buff[BUF_SIZE];
  memset(modify_buff, 0, BUF_SIZE);

  while(1){ //msg에서 수정하려는 텍스트만 추출하기 위한 작업
    if(msg[i] == ' '){
      i++;
      break;
    }
    i++;
  }
  //수정할 텍스트의 크기를 저장.
  modify_next = strlen(&msg[i]);
  //modify_buff에 수정할 내용을 저장한다.
  memmove(modify_buff, &msg[i], modify_next);

  if(line == 1){ //첫 행을 수정하려는 경우
    int i = 0;
    char temp[BUF_SIZE];

    while(1){
      if(source[i] == '\n'){
        i++;
        break;
      }
        i++;
    }
    //temp에 source의 첫번째 행을 제외한 나머지 행들을 넣는다.
    memmove(temp, &source[i], BUF_SIZE - modify_next);
    //modify_buff에 temp를 붙인다.
    memmove(&modify_buff[modify_next], temp, BUF_SIZE - modify_next);
    //source에 수정사항을 덧붙인다.
    memmove(source, modify_buff, BUF_SIZE);
  }

  else if(line == curr_low){ //마지막 행을 수정하려는 경우
    int i = curr_src-2;

    while(1){
      if(source[i] == '\n')
        break;
      i--;
    }

    memmove(&source[i + 1], modify_buff, modify_next);
  }

  else{ //첫 행과 마지막 행 사이를 수정하려는 경우
    int line_count = 0, i = 0;
    int tail_size = 0;
    char head[BUF_SIZE], tail[BUF_SIZE];

    memset(head, 0, BUF_SIZE);
    memset(tail, 0, BUF_SIZE);

    while(1){
      if(source[i] == '\n')
        line_count++;

      if(line_count == line){
        i++;
        break;
      }
      i++;
    }
    //tail에 수정하려는 행의 이후 행들을 저장
    memmove(tail, &source[i], BUF_SIZE - i);
    line_count = 0;

    while(1){
      if(source[i] == '\n')
        line_count++;

      if(line_count == line){
        break;
      }
      i--;
    }
    //head에 수정하려는 행의 이전 행들을 저장
    memmove(head, source, BUF_SIZE);
    memset(source, 0, BUF_SIZE);
    //source에 head를 붙임.
    memmove(source, head, BUF_SIZE - i);
    memmove(&source[i + 1], modify_buff, modify_next);
    tail_size = BUF_SIZE - (i + modify_next + 1);
    memmove(&source[i + modify_next + 1], tail, tail_size);
  }

}
