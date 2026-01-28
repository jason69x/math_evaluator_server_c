/* CLIENT PROGRAM */

#include<stdio.h> // for io functions like printf 
#include<stdlib.h> // for exit() & memset()
#include<unistd.h> // for close()
#include<sys/socket.h> // for basic socket functions socket(), bind(), listen()
#include<string.h> // memset
#include<arpa/inet.h> // inet_addr()
#include<netinet/in.h> // IPPROTO_TCP,sockaddr_in 

#define MAX_BUF_LEN 200 // Maximum size of client buffer

int main(int argc,char *argv[]){
  if(argc !=3){
    //user needs to run ./client.exe and pass ip and port of server as arguments
    fprintf(stderr, "usage: ./client <ip> <port>\n");
    exit(1);
  }
  char *ip = argv[1];
  unsigned short port = atoi(argv[2]);

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;   // AF_INET sets family to ipv4
  server_addr.sin_port = htons(port);  // convert port from host(little endian) format to network format(big endian)
  server_addr.sin_addr.s_addr = inet_addr(ip);  // convert ip passed by user to bytes and set s_addr to it
  //memset(server_addr.sin_zero,0,sizeof server_addr.sin_zero); // this is optional

  // create socket to connect
  int sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); // SOCK_STREAM - TCP , IPPROTO_TCP - use TCP protocol
  if(sockfd == -1){
    perror("socket error");
    exit(1);
  }
  // connect to server using above details, binds a random port on client to the server port
  if( connect(sockfd,(struct sockaddr*)&server_addr,sizeof server_addr) == -1){
    perror("connect error");
    exit(1);
  }
  printf("client connected to %s:%s\n",argv[1],argv[2]);
  //client buffer
  char buf[MAX_BUF_LEN]; int buf_len;
  while(1){
  if( (buf_len = recv(sockfd,buf,MAX_BUF_LEN-1,0))<=0){
    perror("recv error");
    exit(1);
  }
  buf[buf_len] = '\0';
  printf("%s.\n",buf);
    char exp[MAX_BUF_LEN];
    scanf("%s",exp); // accept expression as a string from user
    if(strlen(exp)<=1){
      close(sockfd);
      exit(1);
    }
    // send expression entered by user to server
    send(sockfd,exp,strlen(exp),0);
  }
}
