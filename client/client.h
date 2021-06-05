#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

void print_error();
int create_source(int client_socket);
int input_text(int client_socket);

void print_error(){
	fprintf(stderr,"Error: %s\n", strerror(errno));
	exit(errno);
}

int create_source(int client_socket){	//서버 연결 후, 소스코드 이름 작성
	char rBuff[10], wBuff[BUFSIZ];

	printf("Source code name ?: ");

	fgets(wBuff, BUFSIZ - 1, stdin);
	int write_length = strlen(wBuff);
	write(client_socket, wBuff, write_length - 1);
	read(client_socket, rBuff, 10);

	if(!strcmp(rBuff, "SUCESS")){
		printf("Source code create sucess!\n");
		return 0;
	}

	else{
		printf("Source code create fail!\n");
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

	read(client_socket, rBuff, sizeof(rBuff) - 1);
	printf("\n%s\n", rBuff);
	return 0;
}
