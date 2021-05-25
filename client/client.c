#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

void err_proc();
int main(int argc, char** argv)
{
	int clntSd;
	struct sockaddr_in clntAddr;
	int clntAddrLen, readLen, recvByte, maxBuff;
	char wBuff[BUFSIZ];
	char rBuff[BUFSIZ];

	if(argc != 3){
		printf("Usage: %s [IP Address] [Port]\n", argv[0]);
		return -1;
	}
	//클라이언트 소켓 생성. (IPv4, TCP를 사용함.)
	clntSd = socket(AF_INET, SOCK_STREAM, 0);

	if(clntSd == -1)
		err_proc();

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
	if(connect(clntSd, (struct sockaddr *) &clntAddr, sizeof(clntAddr)) == -1)
	{
		close(clntSd);
		err_proc();
	}

	//서버와 연결되고 난 이후
	while(1)
	{
		//키보드 입력을 받아 메모리 버퍼인 wBuff에 저장.
		fgets(wBuff, BUFSIZ-1, stdin);
		readLen = strlen(wBuff);

		if(readLen < 2)
			continue;

		write(clntSd, wBuff, readLen-1);
		recvByte = 0;
		maxBuff = BUFSIZ-1;


		do{	//wBuff에 쓴 만큼, 서버로부터 데이터를 기다렸다가 출력.
			recvByte += read(clntSd,rBuff,maxBuff);
			maxBuff -= recvByte;
		}while(recvByte < (readLen-1));

		rBuff[recvByte] = '\0';
		printf("\nFile : %s\n", rBuff);
		wBuff[readLen-1]='\0';

		//입력이 END 였다면, 반복문 탈출.
		if(!strcmp(wBuff,"END"))
			break;
	}
	printf("END ^^\n");
	close(clntSd);

	return 0;
}

void err_proc()
{
	fprintf(stderr,"Error: %s\n", strerror(errno));
	exit(errno);
}
