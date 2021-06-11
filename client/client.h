#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <ncurses.h>
#define BUF_SIZE 8192
#define EDIT_MODE 0
#define COMMAND_MODE 1

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_print(char *msg);
void print_help();
void handle_command(char *msg, int sock);
void compile_code(char *code_name, char *exe_name);
void print_source(char *code, int length);

char source[BUF_SIZE];
int command_mode_flag = COMMAND_MODE;
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

//ncurses  관련 함수
WINDOW  *send_window, *recv_window;

WINDOW* create_new_win(const int height, const int width,const int start_y,const int start_x){
  WINDOW* local_win = newwin(height,width,start_y,start_x);
  // box(local_win,0,0);
  wrefresh(local_win);
  return local_win;
}

void init_window(){
  int max_x,max_y,command_x,command_y,sorce_x,sorce_y;
  getmaxyx(stdscr,max_y,max_x);
  sorce_x = max_x;
  command_x = max_x;
  if(max_y < 14){
    printf("terminal is too small\n");
    exit(1);
  }
  if(max_y/3<=7){
    command_y = 7;
    sorce_y = max_y - command_y;
  }else{
    command_y = max_y/3;
    sorce_y = max_y - command_y;
  }
  recv_window = create_new_win(sorce_y,sorce_x,0,0);
  send_window = create_new_win(command_y,command_x,sorce_y,0);

}
void update_sorce_window(int pos,char* code){
}
void update_command_window(int mode){
  switch (mode)
  {
  case EDIT_MODE:
    /* code */
    break;
  case COMMAND_MODE:
  default:
    break;
  }
}
void edit_mode(char* buff){
  noraw();
  echo();
  keypad(stdscr,FALSE);
  wclear(send_window);
  mvwprintw(send_window,0,0,"[edit mode]");
  mvwprintw(send_window,1,1,"<Input text> : ");
  box(send_window,0,0);
  return;
}
void command_mode(){
  raw();
  noecho();
  keypad(stdscr,TRUE);
  char ch=getch();
  switch (ch)
  {
  case KEY_F(1):
    /* code */
    break;
  
  default:
    break;
  }
}