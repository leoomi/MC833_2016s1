#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_PORT 31472
#define MAX_LINE 256

int main(int argc, char * argv[]){
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[MAX_LINE];
  int s;
  int len;
  
  if (argc==2) {
    host = argv[1];
  }
  else {
    fprintf(stderr, "usage: ./client host\n");
    exit(1);
  }

  /* translate host name into peerâ€™s IP address */
  hp = gethostbyname(host);
  if (!hp) {
    fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
    exit(1);
  }

  /* build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(SERVER_PORT);

  /* active open */
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
  }

  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) { 
     perror("simplex-talk: connect"); 
     close(s);
     exit(1); 
   } 
  
  int i;
  char msg[17] = "teste de tempo";
  len = strlen(msg) + 1;
  for(i = 0; i < 10000; i++){
    printf("%d ",i);
    send(s, msg, len, 0);
    len = recv(s, buf, sizeof(buf), 0);
    fputs(buf, stdout);
    printf("\n");
  }
}
