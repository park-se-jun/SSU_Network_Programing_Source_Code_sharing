#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <arpa/inet.h>

int main(int argc, char** argv)
{
	int listenSd, connectSd;
	struct sockaddr_in srvAddr, clntAddr;
	int clntAddrLen, readLen, strLen;
	char rBuff[BUFSIZ];
	int maxFd = 0;
	fd_set defaultFds, rFds;
	int res, i;

	if(argc != 2)
	{
		printf("Usage: %s [Port Number]\n", argv[0]);
		return -1;
	}

	printf("Server start...\n");
	listenSd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(listenSd == -1 )
		printf("socket error");

	memset(&srvAddr, 0, sizeof(srvAddr));
	//서버 addr 구조체 초기화.
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(atoi(argv[1]));

	//소켓과 addr 구조체 바인딩
	if(bind(listenSd, (struct sockaddr *) &srvAddr,
				 sizeof(srvAddr)) == -1)
		printf("bind error\n");

	//5개의 연결 허용.
	if(listen(listenSd, 5) < 0)
		printf("listen error\n");

	//fdset의 모든 비트열 초기화.
	FD_ZERO(&defaultFds);
	//listenSd 에 해당하는 비트열을 1로 설정.
	FD_SET(listenSd, &defaultFds);
	//select 사용을 위해, 생성된 디스크립터의 숫자를 따로 저장.
	maxFd = listenSd;

	clntAddrLen = sizeof(clntAddr);
	while(1)
	{
		rFds = defaultFds;	//select 호출 전에 fdset을 복사해둠.

		//select으로 서버로 오는 이벤트를 감지.
		if((res = select(maxFd + 1, &rFds, 0, 0, NULL)) == -1)
			break;

		for(i=0; i<maxFd+1; i++)
		{
			if(FD_ISSET(i, &rFds))
			{
				if(i == listenSd) //accept a client
				{
					connectSd = accept(listenSd,
						 (struct sockaddr *) &clntAddr,
						 &clntAddrLen);
					if(connectSd == -1)
					{
						printf("Accept Error");
						continue;
					}
					printf("A client is connected...\n");
					//연결된 소켓을 defaultFds에 등록해둠.
					FD_SET(connectSd, &defaultFds);

					if(maxFd < connectSd){
						maxFd = connectSd;
					}
				}
				else //연결된 클라이언트의 요청
				{
					readLen = read(i, rBuff, sizeof(rBuff)-1);
					if(readLen <= 0)
					{
						printf("A client is disconnected...\n");
						//fdset에서 소켓을 해제함.
						FD_CLR(i, &defaultFds);
						close(i);
						continue;
					}
					rBuff[readLen] = '\0';
					printf("\nClient(%d): %s\n",i-3,rBuff);
					write(i,rBuff, strlen(rBuff));
				}
			}
		}
	}
	close(listenSd);
	return 0;
}
