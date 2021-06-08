#include "server.h"

int main(int argc, char** argv){
	int listenSd, connectSd;
	struct sockaddr_in srvAddr, clntAddr;
	int clntAddrLen;
	pthread_t thread;

	if(argc != 2){
		printf("Usage: %s [Port Number]\n", argv[0]);
		return -1;
	}

	printf("Server start...\n");
	listenSd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(atoi(argv[1]));

	bind(listenSd, (struct sockaddr *) &srvAddr, sizeof(srvAddr));
	listen(listenSd, 5);

	clntAddrLen = sizeof(clntAddr);

	while(1){	//accept이 될 때마다 쓰레드가 생성됨.

		connectSd = accept(listenSd, (struct sockaddr *) &clntAddr, (void *) &clntAddrLen);

		if(connectSd == -1)
			continue;

		else
			printf("A client is connected...\n");

		//Thread를 생성하여 클라이언트의 요청을 처리하도록 함.
		pthread_create(&thread, NULL, client_connect, (void *) &connectSd);
		pthread_detach(thread);
	}
	close(listenSd);
	return 0;
}
