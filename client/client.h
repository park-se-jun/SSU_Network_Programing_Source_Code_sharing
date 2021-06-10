#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 8192

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_print(char *msg);
void print_help();
void handle_command(char *msg, int sock);
void compile_code(char *code_name, char *exe_name);
void print_source(char *code, int length);

char source[BUF_SIZE];

//서버로 텍스트 송신
void* send_msg(void* arg){
  int sock = *((int*)arg);  //클라이언트 소켓 FD
  char msg[BUF_SIZE];
  system("clear");
  print_help();

  while(1){ //Command line에서 문자열을 입력받아 서버에 전송.
    fgets(msg, BUF_SIZE, stdin);
    handle_command(msg, sock);
  }

  return NULL;
}

//서버로부터 텍스트 수신
void *recv_msg(void *arg){
  int sock = *((int *)arg);  //클라이언트의 파일 디스크립터
  char msg[BUF_SIZE];
  int str_len;

  while (1){
    str_len = read(sock, msg, BUF_SIZE - 1);
    //read 실패시
    if(str_len == -1)
      return NULL;

    msg[str_len] = '\0';
    system("clear");
    printf("<current source code>\n\n");  //stdin 출력
    print_source(msg, str_len);
    putchar('\n');

    memset(source, 0, BUF_SIZE);
    memmove(source, msg, str_len - 1);
    memset(msg, 0, BUF_SIZE);
    print_help();
  }

  return NULL;
}

//에러 처리
void error_print(char *msg){
  fputs(msg, stderr);
  fputc('\n', stderr);
  exit(1);
}

void print_help(){
  fflush(stdout);
  printf("\n\n= = = = [command] = = = =");
  printf("\n[q or Q = exit]");
  printf("\n[exe or EXE = compile]");
  printf("\n[cls or CLS = Source code clean]");
  printf("\n[#n = row n modify]");
  printf("\n= = = = = = = = = = = = =");
  printf("\nInput text : ");
  fflush(stdout);
}

void handle_command(char *msg, int sock){
  char buff[10], code_name[20], exe_name[20];

  if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")){ //클라이언트 종료
      close(sock);
      exit(1);
  }

  else if(!strcmp(msg, "exe\n") || !strcmp(msg, "EXE\n")){ //소스코드 컴파일 후 종료
    printf("\nPlease enter source file name (example : hello.c) : ");
    scanf("%s", code_name);

    printf("\nPlease enter execute file name (example : hello) : ");
    scanf("%s", exe_name);

    compile_code(code_name, exe_name);
    close(sock);
    exit(1);
  }

  else if(!strcmp(msg, "cls\n") || !strcmp(msg, "CLS\n")){ //소스코드 클리어
    printf("\nDo you want source code clean ? (Y/N) : ");
    fgets(buff, 10, stdin);

    if(!strcmp(buff, "Y\n") || !strcmp(buff, "y\n")){
      write(sock, "&CLEAR&", 8);
      system("clear");
      printf("<current source code>\n\n");  //stdin 출력
      memset(source, 0, BUF_SIZE);
      print_source(source, 0);
      putchar('\n');
      print_help();
      return ;
    }

    else{
      system("clear");
      printf("<current source code>\n\n");  //stdin 출력
      memset(source, 0, BUF_SIZE);
      print_source(source, 0);
      putchar('\n');
      print_help();
      return ;
    }
  }

  else if(!strcmp(msg, "#n\n") || !strcmp(msg, "#N\n")){  //소스코드 수정
    char buff[BUF_SIZE];
    memset(source, 0, BUF_SIZE);
    write(sock, "&MODIFY&", 9);
    printf("\nPlease enter a modification line and text: ");
    fgets(buff, BUF_SIZE, stdin);
    write(sock, buff, sizeof(buff));
    return ;
  }

  else write(sock, msg , strlen(msg)); //null 문자 제외하고 서버로 문자열 보냄
}

void compile_code(char *code_name, char *exe_name){
  FILE *src = fopen(code_name, "w");
  char command[100];

  for(int i = 0; i < BUF_SIZE; i++){
    if(source[i] == '\0')
      source[i] = ' ';
    }

  fwrite(source, sizeof(char), BUF_SIZE - 1, src);
  fclose(src);
  sprintf(command, "%s %s %s", "gcc -o", exe_name, code_name);
  system(command);
}

void print_source(char *code, int length){
  int i = 0, line = 1;
  printf("%d. ", line++);

  while(code[i] != '\0'){
    if(code[i] == '\n'){
      putchar(code[i]);
      printf("%d. ", line++);
      i++;
      continue;
    }

    putchar(code[i]);
    i++;
  }
}
