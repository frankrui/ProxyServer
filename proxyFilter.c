#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#define SERVER_TCP_PORT 8080
#define BUFLEN 256

int main(int argc, char **argv) {

    int n, bytes_to_read;
    int sd, new_sd, client_len, port;
    struct sockaddr_in server, client;
    char *bp, buf[BUFLEN], outbuf[BUFLEN];
    char* strptr;
    char* strptr2;
    char requestType[4];
    char host[256];
    char portNum[6];
    
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
        if ((new_sd = accept(sd, (struct sockaddr *)&client, &client_len)) == -1) {
            fprintf(stderr, "Can't accept client.\n");
            exit(1);
        }
        
        bp = buf;
        bytes_to_read = BUFLEN;
        int i = 0;
        while((n = read(new_sd, bp, bytes_to_read)) > 0) {
        	printf("%s", "hi");
        	printf("%s", bp);
            bp += n;
            bytes_to_read -= n;
       }
        strptr = strtok(buf, " ");
        strcpy(requestType, strptr);
        printf("request type: %s\n", requestType);

        strptr = strtok(NULL, ":");
        strptr2 = strtok(NULL, ":");
        if(strptr2 != NULL) {
        	strcpy(host, strptr2);
        	strptr = strtok(NULL, " ");
        	strcpy (portNum, strptr);
        } else {
        	strptr = strtok (NULL, " ");
        	strcpy(host, strptr);
        	strncpy(portNum,"80",2);
        }
        printf("host: %s\n", host);
        printf("port: %s\n", portNum);
      }

    //close
    close(sd);
    return 0;

}
