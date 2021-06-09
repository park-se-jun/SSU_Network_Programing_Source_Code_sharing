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


int client_count = 0; //서버에 접속한 클라이언트 수
int client_sockets[MAX_CLIENT]; //서버에 접속한 클라이언트들의 Socket FD를 저장한 배열
pthread_mutex_t mutex;
char *source; //쓰레드들이 공유할 메모리
int source_fd, curr_src = 0;

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

    if(!strcmp(msg, "&CLEAR&")){
      printf("Guest %d request to source code clean\n", client_socket);
      memset(source, 0, BUF_SIZE);
      curr_src = 0;
      continue;
    }
    /*
    else if(!strcmp(msg, "&MODIFY&")){
      memset(msg, 0, BUF_SIZE);
      str_len = read(client_socket, msg, sizeof(msg));
      msg[str_len] = '\0';
      curr_src = strlen(msg);
      msg[str_len] = '\n';
      memcpy(source, msg, str_len);
      send_msg(source, curr_src);
      continue;
    }
    */
    msg[str_len] = '\n';
    memcpy(source + curr_src, msg, str_len);
    curr_src += str_len;
    printf("Current source code is %d byte\n", curr_src);
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
