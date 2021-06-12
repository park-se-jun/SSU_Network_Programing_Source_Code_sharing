#include "client.h"

int main(int argc, char *argv[])
{
  int sock;
  struct sockaddr_in server_address;

  //쓰레드 송신 , 쓰레드 수신
  pthread_t snd_thread, rcv_thread;
  void* thread_return;

  if(argc != 3){
    printf("usage : %s <ip> <port>\n" , argv[0]);
    exit(1);
  }
  sem_init(&semaphore,0,3);
  initscr();
  init_window();

  //IPv4, TCP 소켓 생성
  sock = socket(PF_INET, SOCK_STREAM,0);
  
  //서버 주소정보 초기화
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family=AF_INET;
  server_address.sin_addr.s_addr=inet_addr(argv[1]);
  server_address.sin_port=htons(atoi(argv[2]));


  //서버 주소 정보를 기반으로 연결요청, 이때 비로소 클라이언트 소켓이됨.
  connect(sock, (struct sockaddr*)&server_address, sizeof(server_address));

  //쓰레드 생성 및 실행
  pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
  pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);

  //쓰레드 종료까지 대기
  pthread_join(snd_thread, &thread_return);
  pthread_join(rcv_thread, &thread_return);

  //클라이언트 소켓 연결 종료
  close(sock);
  endwin();
  return 0;
}
