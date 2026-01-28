/* SERVER 1 */

#include<sys/socket.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<limits.h>

// this value doesn't matter, but it represents how many clients can be in queue and wait for server to accept the connection.
#define BACKLOG 1 
#define MAX_BUF_SIZE 200 // max size of server buffer

int main(int argc,char *argv[]){
  if(argc!=2){
    fprintf(stderr, "usage: ./server <port>\n");
    exit(1);
  }
  unsigned short port = atoi(argv[1]);
  struct sockaddr_in sock_addr;  // contains socket info. similar to struct sockaddr, but easier to set address & port.
  sock_addr.sin_family = AF_INET; // ipv4
  sock_addr.sin_port = htons(port);  // port 
  sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // ip of localhost, create a socket on localhost
  memset(sock_addr.sin_zero,0,sizeof sock_addr.sin_zero); // optional - padding to make compatible with struct sockaddr.

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
    if( listen(sockfd,BACKLOG) == -1){
      perror("listen error");
      exit(1);
    }
    printf("server listening on port: %d\n",port);
  while(1){
    struct sockaddr_in client_addr; // contains info about client : ip,port
    socklen_t client_addr_len = sizeof(client_addr);
    int new_fd;
    // accept a new connection from client
    if( (new_fd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len)) == -1){
      perror("accept failed");
      continue;
    }
    // stop listening on the socket sockfd, this is used so that only one client can be connected at a time and all other clients will receive "connection refused" error.
    shutdown(sockfd,0);
    char client_ip[INET_ADDRSTRLEN]; 
    // function to get ip address of client in readable format
    inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,sizeof client_ip);
    printf("connected to %s:%d\n",client_ip,ntohs(client_addr.sin_port));
    // start send/recv 
    // sending details to client on how to format expressions
    char *exp_str = "enter expression: <num> <op> <num> or parenthesized expression\n";
    int exp_len = strlen(exp_str); int bytes_sent;
    if ( (bytes_sent = send(new_fd,exp_str,exp_len,0)) == -1){
      perror("send error");
      close(new_fd);
      continue;
    }
    // receive expression from the client and save it to a buffer
    int bytes_recv; char recv_buf[MAX_BUF_SIZE];
    while( (bytes_recv = recv(new_fd,recv_buf,MAX_BUF_SIZE-1,0))!=0){
      if(bytes_recv == -1){
        fprintf(stderr, "failed to receive\n");
        close(new_fd);
        continue;
      }
      recv_buf[bytes_recv] = '\0';
      // server logs
      printf("received from client %s\n",recv_buf);
      double a,b,res; char op;
      sscanf(recv_buf, "%lf %c %lf",&a,&op,&b);
      char send_buf[MAX_BUF_SIZE];
      // evaluate expression
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
      // send the evaluated result to the client
    if ( (bytes_sent = send(new_fd,send_buf,strlen(send_buf),0)) == -1){
      perror("send error");
      close(new_fd);
      continue;
    }
    }
    // end send/recv
    // close connection with the client and start listening for other clients
    printf("closed connection with client %s:%d\n",client_ip,ntohs(client_addr.sin_port));
    close(new_fd);
    listen(sockfd,BACKLOG);
  }
}
