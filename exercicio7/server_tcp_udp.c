#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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

  setsockopt(stcp, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
 
  if ((bind(stcp, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("simplex-talk: bind TCP");
    exit(1);
  }

  listen(stcp, MAX_PENDING);
  
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

              if ( (childpid = fork()) == 0) { 
                  close(stcp);    
                  len = recv(stcp, buf, sizeof(buf), 0);
		  fputs(buf, stdout);
		  send(stcp, buf, len, 0);  
                  exit(0);
               }
               close(connfd);
	 }

         if (FD_ISSET(sudp, &rset)) {
	   len = recvfrom(sudp, buf, sizeof(buf), 0, (struct sockaddr *) &client_info, &addr_size);
	   fputs(buf, stdout);
           sendto(sudp, buf, len, 0, (struct sockaddr *) &client_info, addr_size);
          }     
     }
    	close(stcp);
	close(sudp);
}

