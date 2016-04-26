#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 31472
#define MAX_PENDING 5
#define MAX_LINE 256

int main()
{
  struct sockaddr_in sin, remote_info;
  char buf[MAX_LINE];
  int len;
  int s, new_s;
  int re_len;

  /* build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(SERVER_PORT);

  /* setup passive open */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
  }
  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("simplex-talk: bind");
    exit(1);
  }
  listen(s, MAX_PENDING);

  /* wait for connection, then receive and print text */
  while(1) {
    if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
      perror("simplex-talk: accept");
      exit(1);
    }

          re_len = sizeof(typeof(struct sockaddr_in));
      if(getpeername(new_s, &remote_info, &re_len) < 0){
	perror("getpeername() failed");
	close(new_s);
	exit(1);
      }
      
      printf("teste");
      printf("Endereco IP do cliente: %s\n", inet_ntoa(remote_info.sin_addr));
      printf("Porta do cliente: %s\n", ntohs(remote_info.sin_port));
   
    while (len = recv(new_s, buf, sizeof(buf), 0)){
      fputs(buf, stdout);
      send(new_s, buf, len, 0);
    }
    close(new_s);
  }
}
