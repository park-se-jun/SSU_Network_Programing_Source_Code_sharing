#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <ncurses.h>
#include <semaphore.h>

#define BUF_SIZE 8192
#define INPUT_TEXT_MODE 0
#define QUIT 1
#define COMPILE 2
#define MODIFY 3
#define CLEAR 4
#define PAGE_UP 5
#define PAGE_DOWN 6

#define COMMAND_Y 9
void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_print(char *msg);
void handle_command(int mode, int sock);
void compile_code(char *code_name, char *exe_name);
void print_source(char *code, int start);

void set_input_posible(bool i);
void select_mode(int *mode);
void print_input_text_mode();
void print_select_mode();

WINDOW *send_window, *recv_window;
sem_t semaphore;
char source[BUF_SIZE];
int start_line = 1;
void *send_msg(void *arg)
{
  int sock = *((int *)arg); 
  int mode;
  while (1)
  {
    sem_wait(&semaphore);
    select_mode(&mode);
    handle_command(mode, sock);
    sem_post(&semaphore);
  }

  return NULL;
}
void *recv_msg(void *arg)
{
  int sock = *((int *)arg); 
  char msg[BUF_SIZE];
  int str_len;

  while (1)
  {
    sem_wait(&semaphore);
    str_len = read(sock, msg, BUF_SIZE - 1);
    if (str_len == -1)
      return NULL;

    msg[str_len] = '\0';
    
    memset(source, 0, BUF_SIZE);
    memmove(source, msg, str_len - 1);
    memset(msg, 0, BUF_SIZE);
    print_source(source, start_line);
    sem_post(&semaphore);
  }

  return NULL;
}

void handle_command(int mode, int sock)
{
  char buff[BUFSIZ], code_name[20], exe_name[20];
  if (mode == INPUT_TEXT_MODE)
  {
    print_input_text_mode();
    wscanw(send_window, "%[^\n]s", buff);
    int str_len = strlen(buff);
    buff[str_len] = '\n';
    buff[str_len + 1] = '\0';
    write(sock, buff, strlen(buff)); 
  }
  else if (mode == QUIT)
  { 
    endwin();
    close(sock);
    exit(1);
  }
  else if (mode == COMPILE)
  { 
    set_input_posible(true);
    wclear(send_window);
    box(send_window, 0, 0);
    mvwprintw(send_window, 1, 1, "Please enter source file name (example : hello.c) : ");
    wscanw(send_window, "%s", code_name);
    mvwprintw(send_window, 2, 1, "Please enter execute file name (example : hello) : ");
    wscanw(send_window, "%s", exe_name);
    endwin();
    system("clear");
    compile_code(code_name, exe_name);
    close(sock);
    exit(1);
  }
  else if (mode == CLEAR)
  { 
    set_input_posible(true);
    wclear(send_window);
    box(send_window, 0, 0);
    mvwprintw(send_window, 1, 1, "Do you want source code clean ? (Y/N) : ");

    wscanw(send_window, "%s", buff);

    if (!strcmp(buff, "Y") || !strcmp(buff, "y"))
    {
      write(sock, "&CLEAR&", 8);
      memset(source, 0, BUF_SIZE);
      start_line = 1;
      print_source(source, start_line);
      return;
    }
    else
    {
      return;
    }
  }
  else if (mode == MODIFY)
  {
    set_input_posible(true);
    memset(source, 0, BUF_SIZE);
    write(sock, "&MODIFY&", 9);
    wclear(send_window);
    box(send_window,0,0);
    mvwprintw(send_window, 1, 1, "Please enter a modification line and text: ");
    wscanw(send_window, "%[^\n]s", buff);
    write(sock, buff, sizeof(buff));
    return;
  }
  else if (mode == PAGE_UP)
  {
    if (start_line > 1)
      start_line--;
    print_source(source, start_line);
  }
  else if (mode == PAGE_DOWN)
  {
    start_line++;
    print_source(source, start_line);
  }
}

void compile_code(char *code_name, char *exe_name)
{
  FILE *src = fopen(code_name, "w");
  char command[100];

  for (int i = 0; i < BUF_SIZE; i++)
  {
    if (source[i] == '\0')
      source[i] = ' ';
  }

  fwrite(source, sizeof(char), BUF_SIZE - 1, src);
  fclose(src);
  sprintf(command, "%s %s %s", "gcc -o", exe_name, code_name);
  system(command);
}

void print_source(char *code, int start)
{
  sem_wait(&semaphore);
  int line = 0, j = 1;
  char *lines[BUFSIZ] = {
      0,
  };
  char strings[BUFSIZ];
  memcpy(strings, code, BUFSIZ);
  wclear(recv_window);
  char *ptr = strtok(strings, "\n");
  while (ptr != NULL)
  {
    line++;
    lines[line] = ptr;
    ptr = strtok(NULL, "\n");
  }
  for (int i = start; i <= line; i++)
  {

    mvwprintw(recv_window, j, 1, "%d %s\n", i, lines[i]);
    j++;
  }
  box(recv_window, 0, 0);
  mvwprintw(recv_window, 0, 0, "<current source code>");
  sem_post(&semaphore);
}


WINDOW *create_new_win(const int height, const int width, const int start_y, const int start_x)
{
  WINDOW *local_win = newwin(height, width, start_y, start_x);
  immedok(local_win, true);
  box(local_win, 0, 0);
  return local_win;
}

void init_window()
{
  int max_x, max_y, command_x, command_y, source_x, source_y;
  getmaxyx(stdscr, max_y, max_x);
  source_x = max_x;
  command_x = max_x;
  if (max_y < 18)
  {
    printf("terminal is too small\n");
    endwin();
    exit(1);
  }
  command_y = COMMAND_Y;
  source_y = max_y - command_y;
  recv_window = create_new_win(source_y, source_x, 0, 0);
  send_window = create_new_win(command_y, command_x, source_y, 0);
  mvwprintw(recv_window, 0, 0, "<current source code>");
}

void set_input_posible(bool i)
{
  if (i)
  {
    noraw();
    curs_set(2);
    echo();
    keypad(send_window, FALSE);
  }
  else
  {
    raw();
    curs_set(0);
    noecho();
    keypad(send_window, TRUE);
  }
}
void print_input_text_mode()
{
  wclear(send_window);
  box(send_window, 0, 0);
  mvwprintw(send_window, 0, 0, "[input mode]");
  mvwprintw(send_window, 1, 1, "<Input text> : ");
  set_input_posible(true);
  return;
}
void print_select_mode()
{
  wclear(send_window);

  mvwprintw(send_window, 1, 1, "[q = exit]\n");
  mvwprintw(send_window, 2, 1, "[F1  = compile]\n");
  mvwprintw(send_window, 3, 1, "[F2 = row n modify]\n");
  mvwprintw(send_window, 4, 1, "[F5 = Source code clean]\n");
  mvwprintw(send_window, 5, 1, "[i = change to input mode]\n");
  mvwprintw(send_window, 6, 1, "[press UP or DOWN = move page]");
  set_input_posible(false);
  box(send_window, 0, 0);
  mvwprintw(send_window, 0, 0, "[select mode]");
  return;
}
void select_mode(int *mode)
{
  print_select_mode();
  int ch;
  while (1)
  {
    ch = wgetch(send_window);
    switch (ch)
    {
    case 'q':
      *mode = QUIT;
      return;
    case KEY_F(1):
      *mode = COMPILE;
      return;
    case KEY_F(2):
      *mode = MODIFY;
      return;
    case KEY_F(5):
      *mode = CLEAR;
      return;
    case 'i':
      *mode = INPUT_TEXT_MODE;
      return;
    case KEY_UP:
      *mode = PAGE_UP;

      return;
    case KEY_DOWN:
      *mode = PAGE_DOWN;

      return;
    default:
      break;
    }
  }
}