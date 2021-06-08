#include "client.h"

int main(int argc, char** argv)
{
	int clntSd;
	struct sockaddr_in clntAddr;

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
	}

	close(clntSd);

	return 0;
}
