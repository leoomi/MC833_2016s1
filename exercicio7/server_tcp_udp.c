#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>
#include <unistd.h>

#define SERVER_PORT 31472
#define MAX_PENDING 5
#define MAX_LINE 256

int main()
{
  struct sockaddr_in sin;
  char buf[MAX_LINE];
  int len;
  int i, nready, connfd, maxfd;
  int stcp, sudp;
  int optval = 1;
  pid_t childpid;
  fd_set rset;
  ssize_t n;
  struct sockaddr_in client_info;
  socklen_t addr_size = sizeof client_info;

  /*TCP*/
  
  /* build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(SERVER_PORT);

  /* setup passive open */
  if ((stcp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
  }

  if(setsockopt(stcp, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0){
    perror("setsockopt error");
    exit(1);
  }
 
  if ((bind(stcp, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("simplex-talk: bind TCP");
    exit(1);
  }

  if(listen(stcp, MAX_PENDING) < 0){
    perror("listen error");
    exit(1);
  }
  
  /*UDP*/
  if ((sudp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
  }

  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(SERVER_PORT);
  
  if ((bind(sudp, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("simplex-talk: bind UDP");
    exit(1);
  }
  
  /* Receive frm socket and print text */
  FD_ZERO(&rset);
  maxfd = sudp + 1;
  for ( ; ; ) {
    FD_SET(stcp, &rset);
    FD_SET(sudp, &rset);
    if ( (nready = select(maxfd, &rset, NULL, NULL, NULL)) < 0) {
      perror("Select() error");
      exit(1);
    }

    if (FD_ISSET(stcp, &rset)) {
      connfd = accept(stcp, (struct sockaddr *) &client_info,&addr_size);
      printf("New TCP client IP: %s:%d\n", inet_ntoa(client_info.sin_addr), (int)ntohs(client_info.sin_port));

      if ( (childpid = fork()) == 0) { 
	while(1){
	  len = recv(connfd, buf, sizeof(buf), 0);
	  send(connfd, buf, len, 0);
	}
	close(connfd);
	exit(0);
      }
    }

    if (FD_ISSET(sudp, &rset)) {
      len = recvfrom(sudp, buf, sizeof(buf), 0, (struct sockaddr *) &client_info, &addr_size);
      printf("From IP: %s:%d\n", inet_ntoa(client_info.sin_addr), (int)ntohs(client_info.sin_port));
      fputs(buf, stdout);
      addr_size = sizeof client_info;
      sendto(sudp, buf, len, 0, (struct sockaddr *) &client_info, addr_size);
    }     
  }
  close(connfd);
  close(stcp);
  close(sudp);
}
