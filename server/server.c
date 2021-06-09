#include "server.h"

int main(int argc, char * argv[]){
  int server_socket, client_socket;
  struct sockaddr_in server_address, client_address;
  socklen_t client_address_size;
  pthread_t thread;

  if(argc != 2){ //매개인자 갯수를 만족하지 못할 경우
    printf("Usage : %s <port> \n" , argv[0]);
    exit(1);
  }
  
  system("clear");
  printf("\nServer Start\n");

  pthread_mutex_init(&mutex, NULL); //뮤텍스 생성

  if(init_source() == -1){  //공유 메모리 생성
    printf("File create error!\n");
    exit(0);
  }

  //IPv4, TCP 소캣 생성
  server_socket = socket(PF_INET, SOCK_STREAM,0);

  //서버주소에 관련한 구조체 정보 초기화
  memset(&server_address , 0 , sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(atoi(argv[1]));


  //서버주소 할당
  bind(server_socket,(struct sockaddr*)&server_address, sizeof(server_address));

  //클라이언트의 요청 대기
  listen(server_socket , MAX_CLIENT);

  while (1) {
    client_address_size = sizeof(client_address);

    //클라이언트의 연결요청이 들어오면 클라이언트와의 송수신을 위한 소켓을 생성
    client_socket = accept(server_socket , (struct sockaddr*)&client_address, &client_address_size);
    //클라이언트의 ip 정보를 문자열로 변환 후 출력
    printf("\nconnected client ip : %s\n" , inet_ntoa(client_address.sin_addr));

    /*
      클라이언트 접속 수를 카운팅하고, 클라이언트 소켓들의 배열에 저장.
      이 때, mutex를 통한 Lock을 통해, Data Race를 방지한다.
    */
    pthread_mutex_lock(&mutex);
    client_sockets[client_count++] = client_socket;
    pthread_mutex_unlock(&mutex);

    printf("Guest %d connected\n" , client_count);

    //새로 들어온 클라이언트를 대상으로 하는 쓰레드 생성 및 실행
    pthread_create(&thread , NULL, service_client, (void*)&client_socket);

    //쓰레드가 종료되면 소멸시킴
    pthread_detach(thread);
  }

  return 0;
}
