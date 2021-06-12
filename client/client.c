#include "client.h"

int main(int argc, char *argv[])
{
  int sock;
  struct sockaddr_in server_address;

  
  pthread_t snd_thread, rcv_thread;
  void* thread_return;

  if(argc != 3){
    printf("usage : %s <ip> <port>\n" , argv[0]);
    exit(1);
  }
  sem_init(&semaphore,0,4);
  initscr();
  init_window();

  sock = socket(PF_INET, SOCK_STREAM,0);
  
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family=AF_INET;
  server_address.sin_addr.s_addr=inet_addr(argv[1]);
  server_address.sin_port=htons(atoi(argv[2]));

  connect(sock, (struct sockaddr*)&server_address, sizeof(server_address));
  pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
  pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);

  pthread_join(snd_thread, &thread_return);
  pthread_join(rcv_thread, &thread_return);

  close(sock);
  endwin();
  return 0;
}
