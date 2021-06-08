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
}user_info;


void print_error();
int connect_source(user_info *user , SOCKET client_socket);
int input_text(int client_socket);

void print_error(){
	fprintf(stderr,"Error: %s\n", strerror(errno));
	exit(errno);
}

int connect_source(user_info * user, SOCKET client_socket){	//서버 연결 후, 소스코드 이름 작성
	char rBuff[10];
	char* filename = user->filename;
	int file_length = strlen(filename);

	write(client_socket, filename , file_length);
	printf("LOADING...\n");
	read(client_socket, rBuff, 10);

	if(!strcmp(rBuff, "SUCESS")){
		printf("SUCESS: connect to %s\n",filename);
		return 0;
	}

	else{
		printf("FAIL: connect to %s\n",filename);
		return -1;
	}
	return 0;
}

int input_text(int client_socket){	//소스코드 내용 작성 함수
	char wBuff[BUFSIZ], rBuff[BUFSIZ];

	printf("input text : ");
	fgets(wBuff, BUFSIZ - 1, stdin);
	int write_length = strlen(wBuff);
	write(client_socket, wBuff, write_length - 1);

	return 0;
}

int print_source(int client_socket){
	char rBuff[BUFSIZ];
	system("clear");
	read(client_socket, rBuff, sizeof(rBuff) - 1);
	printf("\n%s", rBuff);

	return 1;
}

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

void user_info_init(user_info** user_ptr,SOCKET client_socket){
	
	user_info* user = malloc(sizeof(user_info));
	__init_filename(user);
	__init_nickname(user);
	__init_currentLine(user);
	*user_ptr = user;
}

void user_info_print(user_info* user){
	printf("filename: %s \nnickname: %s \ncurrentLine: %d \n",user->filename,user->nickname,user->now_line);
}