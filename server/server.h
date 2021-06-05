#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

void *client_connect(void *data);
int init_source(int connectSd);
int write_source(int connectSd);

struct stat sb;
char *source;
int source_fd, curr_src = 0;

void *client_connect(void * data) //클라이언트가 연결되면, 파일 입출력 작업을 한다.
{
	int connectSd = *((int *) data);

	while(1){	//소스 코드의 이름을 입력받음.
		if(init_source(connectSd) == 0){	//성공적인 입력
			write(connectSd, "SUCESS", 7);
			break;
		}
	}

	while(1)	//클라이언트로부터 데이터를 읽어온다.
	{
		if(write_source(connectSd) == -1)
			break;

	}

	fprintf(stderr, "The client is disconnected.\n");
	close(connectSd);
}

int init_source(int connectSd){
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
	source_fd = shm_open(source_name, O_RDWR|O_CREAT, 0777);
	ftruncate(source_fd, BUFSIZ);

	if(source_fd == -1){
		printf("File create error!\n");
		write(connectSd, "FAIL", 5);
		close(connectSd);
		exit(1);
	}

	source = (char *) mmap(NULL, BUFSIZ, PROT_READ|PROT_WRITE, MAP_SHARED, source_fd, 0);
	memset(source, 0, BUFSIZ);
	close(source_fd);
	return 0;
}

int write_source(int connectSd){	//클라이언트로부터 읽어서 파일에 쓴다.
	char rBuff[BUFSIZ];
	memset(rBuff, 0, BUFSIZ);
	int read_length = read(connectSd, rBuff, sizeof(rBuff) - 1);

	rBuff[read_length] = '\n';
	rBuff[++read_length] = '\0';

	if(read_length <= 0)
		return -1;

	memcpy(source + curr_src, rBuff, read_length);
	curr_src += read_length;
	printf("current source code is %d Byte\n", curr_src);
	write(connectSd, source, curr_src);
}
