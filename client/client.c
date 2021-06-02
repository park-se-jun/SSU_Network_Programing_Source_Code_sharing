#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

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

	return 0;
}

int print_source(int client_socket){
	char rBuff[BUFSIZ];
	system("clear");
	read(client_socket, rBuff, sizeof(rBuff) - 1);
	printf("\n%s", rBuff);

	return 1;
}

int main(int argc, char** argv)
{
	int clntSd;
	struct sockaddr_in clntAddr;
	int clntAddrLen, readLen, recvByte, maxBuff;
	char rBuff[BUFSIZ], wBuff[BUFSIZ];

	if(argc != 3)
		printf("Usage: %s [IP Address] [Port]\n", argv[0]);

	//클라이언트 소켓 생성. (IPv4, TCP를 사용함.)
	clntSd = socket(AF_INET, SOCK_STREAM, 0);

	if(clntSd == -1)
		print_error();

	printf("==== client program =====\n");

	memset(&clntAddr, 0, sizeof(clntAddr));
	/*
		클라이언트 주소 초기화.
		IPv4를 사용하고, IP주소와 포트번호는 매개인자를 사용함.
	*/
	clntAddr.sin_family = AF_INET;
	clntAddr.sin_addr.s_addr = inet_addr(argv[1]);
	clntAddr.sin_port = htons(atoi(argv[2]));

	//서버와 연결을 시도함. 실패 시 예외처리.
	if(connect(clntSd, (struct sockaddr *) &clntAddr,
			    sizeof(clntAddr)) == -1)
	{
		close(clntSd);
		print_error();
	}

	if(create_source(clntSd) == -1){
		close(clntSd);
		printf("Source code create error!\n");
		exit(0);
	}

	while(1){
		if(input_text(clntSd) == -1)
			break;

		//while(print_source(clntSd))
			//break;
	}

	close(clntSd);

	return 0;
}
