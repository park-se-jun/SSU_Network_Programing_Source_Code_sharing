#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#define SCREEN_SIZE 21
#define SOCKET int
#define MAX_USER 5
typedef struct user_info{
	char* nickname;
	int now_line;
	char* filename;
	SOCKET socket;
}user_info;

void show_code(int fd, char* nickname){ // now_line은 현재 몇번째 라인인지
	int fd_des = dup(fd);
	char** lineptr;
    char** tmp_lineptr = lineptr ;
	
    size_t size[SCREEN_SIZE] ={0,};

	FILE *fp = fdopen(fd_des,"r+");
	for(int i = 0; i <SCREEN_SIZE ;i++){
		getline(tmp_lineptr,&size[i],fp);

        tmp_lineptr = lineptr + size[i];
	}
	// printf("%s",lineptr);
	// for(int ){

    // }		
}
void show_user_pos(user_info* users, int length){
	printf("==============================================\n");
	printf("<현재 접속중인 유저 리스트 입니다>\n");
	for(int i = 0; i<length; i++){
		printf("|유저 이름 %10s ", (users[i].nickname));
		printf("|커서 위치 %10d |\n",(users[i].now_line));
	}
}
void show_commandline(){

	printf("==============================================\n");
	printf("edit 모드에서 위 아래 방향키를 클릭시 커서가 움직입니다.\n");
	printf("edit 모드에서 Enter 클릭시 입력이 완료됩니다.\n");
	printf("F1을 누르면 로컬에 저장이 됩니다.\n");
	printf("<edit mode> :");
	return ;
}
void get_command_line(){

}
void __init_nickname(user_info* user){
	size_t size = 0;
	ssize_t ret;
	printf("공백 없는 닉네임을 입력해주세요: ");
	ret = getline(&(user->nickname),&size,stdin);
	int newline = strlen(user->nickname)-1;
	(user->nickname)[newline] ='\0';
	return ;
}
void __init_filename(user_info* user){
	size_t size = 0;
	ssize_t ret;
	printf("파일 이름을 입력해주세요: ");
	ret = getline(&(user->filename),&size,stdin);
	int newline = strlen(user->filename)-1;
	(user->filename)[newline] ='\0';
	
}
void __init_currentLine(user_info* user){
	user->now_line = 0;
}
void __init_socket(user_info* user){
	user->socket = -1;
}
void user_info_init(user_info** user_ptr){
	
	user_info* user = malloc(sizeof(user_info));
	__init_filename(user);
	__init_nickname(user);
	__init_currentLine(user);
	__init_socket(user);
	*user_ptr = user;
}
void user_info_set_socker(user_info* user, SOCKET sd){
	user->socket = sd;
}
void user_info_print(user_info* user){
	printf("filename: %s \nnickname: %s \ncurrentLine: %d \nsocket: %d \n",user->filename,user->nickname,user->now_line,user->socket);
}