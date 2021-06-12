#include "server.h"

int main(int argc, char * argv[]){
  int server_socket, client_socket;
  struct sockaddr_in server_address, client_address;
  socklen_t client_address_size;
  pthread_t thread;

  if(argc != 2){ 
    printf("Usage : %s <port>\n" , argv[0]);
    exit(1);
  }

  system("clear");
  printf("Port is %s, Server Start\n", argv[1]);

  pthread_mutex_init(&mutex, NULL); 

  if(init_source() == -1){  
    printf("File create error!\n");
    exit(0);
  }


  server_socket = socket(PF_INET, SOCK_STREAM,0);

 
  memset(&server_address , 0 , sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(atoi(argv[1]));



  bind(server_socket,(struct sockaddr*)&server_address, sizeof(server_address));


  listen(server_socket , MAX_CLIENT);

  while (1) {
    client_address_size = sizeof(client_address);

  
    client_socket = accept(server_socket , (struct sockaddr*)&client_address, &client_address_size);

    printf("connected client ip : %s\n" , inet_ntoa(client_address.sin_addr));


    pthread_mutex_lock(&mutex);
    client_sockets[client_count++] = client_socket;
    pthread_mutex_unlock(&mutex);

    pthread_create(&thread , NULL, service_client, (void*)&client_socket);

    pthread_detach(thread);
  }

  return 0;
}
