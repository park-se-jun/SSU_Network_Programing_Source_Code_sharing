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

int client_count = 0; 
int client_sockets[MAX_CLIENT]; 
pthread_mutex_t mutex;
char *source; 
int source_fd, curr_src = 0, curr_low = 0;


void* service_client(void* arg){
  int client_socket = *((int*)arg);
  int str_len = 0, i;
  char msg[BUF_SIZE];
  memset(msg, 0, BUF_SIZE);

  printf("Guest %d - Service_client run\n" , client_socket);

  
  while ( (str_len = read(client_socket , msg , sizeof(msg))) != 0) {
    printf("Guest %d send text %d byte\n", client_socket, str_len);


    if(!strcmp(msg, "&CLEAR&")){
      printf("Guest %d request to source code clean\n", client_socket);
      memset(source, 0, BUF_SIZE);
      curr_src = 0;
      curr_low = 0;
      continue;
    }


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
    send_msg(source, curr_src);  
  }

  printf("Guest %d is disconnected\n", client_socket);
  pthread_mutex_lock(&mutex); 

  for(i = 0; i < client_count; i++){
    if(client_socket == client_sockets[i]){
        while (i < client_count -1){
          client_sockets[i] = client_sockets[i+1];
          i++;
        }
        break;
    }
  }

  client_count--; 
  pthread_mutex_unlock(&mutex); 
  close(client_socket); 
  printf("Current client : %d\n" , client_count);
  return NULL;
}

void send_msg(char *msg, int len){
  int i;

  pthread_mutex_lock(&mutex);     
  for (i = 0; i < client_count; i++) {
 
      write(client_sockets[i], msg, len);
  }
  pthread_mutex_unlock(&mutex);    

}


void error_print(char *message){
  fputs(message , stderr);
  fputc('\n' , stderr);
  exit(-1);
}

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

void modify_source(char *msg){ 
  int line = atoi(msg); 
  int i = 0, modify_next;
  char modify_buff[BUF_SIZE];
  memset(modify_buff, 0, BUF_SIZE);

  while(1){ 
    if(msg[i] == ' '){
      i++;
      break;
    }
    i++;
  }

  modify_next = strlen(&msg[i]);

  memmove(modify_buff, &msg[i], modify_next);

  if(line == 1){ 
    int i = 0;
    char temp[BUF_SIZE];

    while(1){
      if(source[i] == '\n'){
        i++;
        break;
      }
        i++;
    }

    memmove(temp, &source[i], BUF_SIZE - modify_next);
 
    memmove(&modify_buff[modify_next], temp, BUF_SIZE - modify_next);

    memmove(source, modify_buff, BUF_SIZE);
  }

  else if(line == curr_low){ 
    int i = curr_src-2;

    while(1){
      if(source[i] == '\n')
        break;
      i--;
    }

    memmove(&source[i + 1], modify_buff, modify_next);
  }

  else{ 
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

    memmove(head, source, BUF_SIZE);
    memset(source, 0, BUF_SIZE);

    memmove(source, head, BUF_SIZE - i);
    memmove(&source[i + 1], modify_buff, modify_next);
    tail_size = BUF_SIZE - (i + modify_next + 1);
    memmove(&source[i + modify_next + 1], tail, tail_size);
  }

}
