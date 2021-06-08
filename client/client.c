#include "client.h"

int main(int argc, char** argv)
{
	int clntSd;
	struct sockaddr_in clntAddr;
	int clntAddrLen, readLen, recvByte, maxBuff;
	char rBuff[BUFSIZ], wBuff[BUFSIZ];


	user_info user_list[MAX_USER]; //서버에 연결되 있는 유저 정보들
	user_info* me; // 나의 정보
	

	if(argc != 3)
		printf("Usage: %s [IP Address] [Port]\n", argv[0]);

	//클라이언트 소켓 생성. (IPv4, TCP를 사용함.)

	clntSd = socket(AF_INET, SOCK_STREAM, 0);
	if(clntSd == -1)
		print_error();
	/*
		클라이언트 주소 초기화.
		IPv4를 사용하고, IP주소와 포트번호는 매개인자를 사용함.
	*/
	memset(&clntAddr, 0, sizeof(clntAddr));
	clntAddr.sin_family = AF_INET;
	clntAddr.sin_addr.s_addr = inet_addr(argv[1]);
	clntAddr.sin_port = htons(atoi(argv[2]));

	//서버와 연결을 시도함. 실패 시 예외처리.
	{
		if(connect(clntSd, (struct sockaddr *) &clntAddr,sizeof(clntAddr)) == -1)
		{
			close(clntSd);
			print_error();
		}
	}
	user_info_init(&me,clntSd);

	if(connect_source(me,clntSd) == -1){
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
