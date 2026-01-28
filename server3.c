/*  SERVER 3 - concurrent clinets using select() 
 *  */
#include<sys/socket.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<limits.h>
#include<sys/select.h>

#define BACKLOG 1
#define MAX_BUF_SIZE 200

int main(int argc,char *argv[]){
  if(argc!=2){
    fprintf(stderr, "usage: ./server <port>\n");
    exit(1);
  }
  unsigned short port = atoi(argv[1]);
  struct sockaddr_in sock_addr;  // contains socket info. similar to struct sockaddr, but easier to set address & port.
  sock_addr.sin_family = AF_INET; // ipv4
  sock_addr.sin_port = htons(port);  // port 
  sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // ip of localhost
  memset(sock_addr.sin_zero,0,sizeof sock_addr.sin_zero); // padding to make compatible with struct sockaddr.

  // create a socket on localhost, domain:ipv4,socket type: stream, protocol: tcp
  int sockfd = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP); 
  if(sockfd == -1){
    perror("socket error");
    exit(1);
  }
  //bind socket to a port, 
  if( bind(sockfd,(struct sockaddr*)&sock_addr,sizeof sock_addr) == -1 ){
    perror("bind error");
    exit(1);
  }
  // start listening on socket, backlog determines how many connection requests can be in queue
  int listen_fd;
    if(listen(sockfd,BACKLOG) == -1){
      perror("listen error");
      exit(1);
    }
    printf("server listening on port: %d\n",port);
  /* server setup end*/
  /* setting file descriptor sets for select START*/
  fd_set master;
  int max_fd = sockfd;
  FD_ZERO(&master); // to clear random garbage on that was on memory
  FD_SET(sockfd,&master); // adding listener socket to fd set

  /* setting file descriptor sets for select END*/
  while(1){
    fd_set recvfds = master; // copy all current clients that may or may not be reaady
    if(select(max_fd+1,&recvfds,NULL,NULL,NULL)==-1){
      perror("select error");
      exit(1);
    }
    for(int i=0;i<=max_fd;i++){
      if(FD_ISSET(i,&recvfds)){
        if(i == sockfd){ // if listener is ready, accept a client
          struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int new_fd;
    if( (new_fd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len)) == -1){
      perror("accept failed");
      continue;
    }
    char client_ip[INET_ADDRSTRLEN]; 
    inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,sizeof client_ip);
    printf("connected to %s:%d\n",client_ip,ntohs(client_addr.sin_port));
    // send client input format 
    char *exp_str = "enter expression: <num> <op> <num> or parenthesized expression\n";
    int exp_len = strlen(exp_str); int bytes_sent;
    if ( (bytes_sent = send(new_fd,exp_str,exp_len,0)) == -1){
      perror("send error");
      close(new_fd);
      continue;
    }
          FD_SET(new_fd,&master);
          if(new_fd>max_fd) max_fd = new_fd;
        }
        else{ // if a client is ready, process its requests
    struct sockaddr_in client_info;
    socklen_t client_info_len = sizeof(client_info);
        getpeername(i,(struct sockaddr*)&client_info,&client_info_len);
          char client_ip[INET_ADDRSTRLEN];
          inet_ntop(AF_INET,&client_info.sin_addr,client_ip,sizeof client_ip);
    int bytes_recv; char recv_buf[MAX_BUF_SIZE];
     bytes_recv = recv(i,recv_buf,MAX_BUF_SIZE-1,0);
      if(bytes_recv <=0){
            if(bytes_recv == 0){
        fprintf(stderr, "client %s:%d closed the connection\n",client_ip,client_info.sin_port);
            }
            else{
        fprintf(stderr, "failed to receive\n");
            }
        close(i);
        FD_CLR(i,&master);
        continue;
      }
      recv_buf[bytes_recv] = '\0';
      printf("-> received from client %s:%d = %s\n",client_ip,client_info.sin_port,recv_buf);
      double a,b,res; char op;
      sscanf(recv_buf, "%lf %c %lf",&a,&op,&b);
      char send_buf[MAX_BUF_SIZE];
      switch (op) {
        case '+':{res = a+b;break;}
        case '-':{res = a-b;break;}
        case '*':{res = a*b;break;}
        case '/':{res = a/b;break;}
        default: {res = INT_MIN;break;}
      }
      if(res!=INT_MIN){
        snprintf(send_buf,MAX_BUF_SIZE, "%lf\n",res);
      }
      else sprintf(send_buf, "operator not supported\n");
    if (send(i,send_buf,strlen(send_buf),0) == -1){
      perror("send error");
      close(i);
      continue;
    }
        }
      }
    }
  }
}
