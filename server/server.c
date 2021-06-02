#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

void *client_connect(void *data);
int init_source(int connectSd, int *write_fd, int *read_fd);
int write_source(int connectSd, int *write_fd);
int send_source(int connectSd, int *read_fd);

/*
	단일 프로세스가 하나의 클라이언트 요청을 받는 단점을 보완하기 위해,
	여러 쓰레드로 여러 클라이언트의 요청을 받도록 한다.
*/
void *client_connect(void * data)
{
	int connectSd = *((int *) data);
	int write_fd, read_fd;

	while(1){
		if(init_source(connectSd, &write_fd, &read_fd) == 0){
			write(connectSd, "SUCESS", 7);
			break;
		}
	}

	while(1)	//클라이언트로부터 데이터를 읽어온다.
	{
		if(write_source(connectSd, &write_fd) == -1)
			break;

		if(send_source(connectSd, &write_fd) == -1)
			break;
	}

	fprintf(stderr, "The client is disconnected.\n");
	close(connectSd);
}

int init_source(int connectSd, int *write_fd, int *read_fd){
	char source_name[BUFSIZ];
	int name_length = read(connectSd, source_name, sizeof(source_name) - 1);

	if(name_length <= 0){
		fprintf(stderr, "Input error!\n");
		write(connectSd, "FAIL", 5);
		close(connectSd);
		exit(0);
	}

	source_name[name_length] = '\0';
	printf("Current working source code name is %s\n", source_name);
	*write_fd = open(source_name, O_WRONLY|O_CREAT, 0666);

	if(*write_fd == -1){
		printf("File create error!\n");
		write(connectSd, "FAIL", 5);
		close(connectSd);
		exit(1);
	}

	*read_fd = open(source_name, O_RDONLY, 0666);

	if(*read_fd == -1){
		printf("File create error!\n");
		write(connectSd, "FAIL", 5);
		close(connectSd);
		exit(1);
	}

	return 0;
}

int write_source(int connectSd, int *write_fd){	//클라이언트로부터 읽어서 파일에 쓴다.
	char rBuff[BUFSIZ];
	int read_length = read(connectSd, rBuff, sizeof(rBuff) - 1);

	if(read_length <= 0)
		return -1;

	//한 줄씩 개행이 들어가도록 맨 끝에 개행을 넣어줌.
	rBuff[read_length] = '\n';
	//rBuff[read_length + 1] = '\0';
	printf("Client(%d): %s\n",connectSd, rBuff);

	write(*write_fd, rBuff, strlen(rBuff));
}

int send_source(int connectSd, int *read_fd){	//클라이언트에게 파일의 내용을 전송한다.
	/*
	char wBuff[BUFSIZ];
	int cur_pos = lseek(*read_fd, 0, SEEK_CUR);
	lseek(*read_fd, 0, SEEK_SET);
	read(*read_fd, wBuff, BUFSIZ);
	printf("wBuff is %s\n", wBuff);
	lseek(*read_fd, cur_pos, SEEK_SET);
	*/
}

int main(int argc, char** argv){
	int listenSd, connectSd;
	struct sockaddr_in srvAddr, clntAddr;
	int clntAddrLen, strLen;
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

		connectSd = accept(listenSd,
			   	(struct sockaddr *) &clntAddr, &clntAddrLen);

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
