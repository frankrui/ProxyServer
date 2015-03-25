#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SERVER_TCP_PORT 8080
#define BUFLEN 256

char* firstLine(char* request, char* temp) {
  int i = 0;
  memset(temp, 0, sizeof(temp));
  while(request[i] != '\r' && request[i+1] != '\n') {
    temp[i] = request[i];
    i++;
  }
  printf("%s", temp);
  return temp;
}

int main(int argc, char **argv) {

    int n, bytes_to_read;
    int sd, new_sd, client_len, port, hostPort, host_sd;
    struct sockaddr_in server, client,host;
    struct hostent *hostent;
    char *bp, buf[BUFLEN], outbuf[BUFLEN];
    char* strptr;
    char* strptr2;
    char* strptr3;
    char requestType[4];
    char hostAddr[256];
    char absolutePath[256];
    char portNum[6];
    char temp[256];
    
    switch (argc) {
        case 1:
            port = SERVER_TCP_PORT;
            break;
        case 2:
            port = atoi(argv[1]);
            break;
        default:
            fprintf(stderr, "Usage: %s [port]\n",
                    argv[0]);
            exit(1);
    }
    
    /* Create a stream socket. */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket.\n");
        exit(1);
    }
    
    /* Bind an address to the socket */
    memset((char *) &server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (struct sockaddr *)&server, sizeof(server)) ==-1) {
        fprintf(stderr, "Can't bind name to socket.\n");
        exit(1);
    }
    
    /* Receive from the client. */
    listen(sd, 5);

    while (1) {
        memset(buf, 0, sizeof(buf));
	memset(portNum,0,sizeof(portNum));
	memset(hostAddr, 0, sizeof(hostAddr));
	memset(requestType,0,sizeof(requestType));
	memset(absolutePath,0,sizeof(absolutePath));
	memset(temp,0,sizeof(temp));
        if ((new_sd = accept(sd, (struct sockaddr *)&client, &client_len)) == -1) {
            fprintf(stderr, "Can't accept client.\n");
            exit(1);
        }
        
        bp = buf;
        bytes_to_read = BUFLEN;
        while((n = read(new_sd, bp, bytes_to_read)) > 0) {
	  printf("%s", bp);
	  printf("%d", n);
	  if(*(bp += (n-1)) == '\n') {
	    break;
	  }
	  bp += n;
	  bytes_to_read -= n;
       	}

	char* first = firstLine(buf,temp);
	
        strptr = strtok(first, " ");
        strcpy(requestType, strptr);
        printf("request type: %s\n", requestType);

	/* Check if Method is a GET method */
	if(strcmp(requestType,"GET") != 0) {
	  memset(outbuf,0,sizeof(outbuf));
	  strcpy(outbuf,"405 Method Not Allowed (Only a GET method is allowed)\n");
	  write(new_sd, outbuf, strlen(outbuf));
	  printf("Sent: %s\n", outbuf);
	  close(new_sd);
	  continue;
	}

	strptr = strtok(NULL, ":");
	strptr2 = strtok(NULL, ":");
	strptr3 = strtok(NULL, " ");
        if(strptr3 != NULL) {
	  char buffer[256];
	  
	  strcpy(buffer, strptr2 + 2);

	  strptr = strtok(buffer, "/");
	  strcpy(hostAddr, strptr);
	  strptr = strtok(NULL, " ");
	  strcpy(absolutePath, strptr);
	  strcpy (portNum, strptr3);
        } else {
	  char buffer[256];
	  
	  strptr = strtok(strptr2, " ");
	  strcpy(buffer, strptr + 2);
	  strptr = strtok(buffer, "/");
	  strcpy(hostAddr,strptr);
	  strptr = strtok(NULL, " ");
	  strcpy(absolutePath,strptr);
	  strncpy(portNum,"80",2);
        }
        printf("host: %s\n", hostAddr);
        printf("port: %s\n", portNum);
	printf("path: %s\n", absolutePath);

    	hostent = gethostbyname(hostAddr);
    	bzero((char *) &host, sizeof(host));
    	host.sin_family = AF_INET;
    	hostPort = atoi(portNum);
    	host.sin_port = htons(hostPort);
    	bcopy((char *) hostent->h_addr, (char *) &host.sin_addr.s_addr, hostent->h_length);
	//printf("%d",hostent->h_addr);
	//printf("%d",hostent->h_length);

    	if ((host_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	  fprintf(stderr, "Can't create a socket.\n");
	  exit(1);
    	}

    	if (connect(host_sd,(struct sockaddr*) &host, sizeof(host)) < 0) {
	  printf("Connect failed (on port %d,  %s).\n", host.sin_port, inet_ntoa(host.sin_addr));
	  exit(1);
    	} else {
	  printf("Connection succeeded\n");
    	}

	close(new_sd);
    }

    //close
    close(sd);
    return 0;

}


