#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Thread.h"
#include <time.h>
#include <stdbool.h>
#include <math.h>
#define SERVER_TCP_PORT 8080
#define BUFLEN 32000 //make it smaller for testing

struct Argument{
        FILE* fp;
        int sd;
};

/* Reads only the headers up to \r\n\r\n of the response */
int readHeaders(int sd, char* buffer) {
  printf("inside function readHeaders\n");
  char* strptr = buffer;
  char accumulator[4];
  int amountRead = 0;
  while(1) {
    int i = read(sd, strptr, 1);
    if(strlen(accumulator) < 4) {
      accumulator[amountRead] = buffer[amountRead];
    } else {
      accumulator[0] = accumulator[1];
      accumulator[1] = accumulator[2];
      accumulator[2] = accumulator[3];
      accumulator[3] = buffer[amountRead];
    }
    if(strcmp(accumulator, "\r\n\r\n") == 0) {
      return amountRead + 1;
    }
    strptr += 1;
    amountRead += i;
  }
}

// Read a line from socket.
int readSocketLine(int sd, char* buf) {
  printf("in function readSocketLine\n");
  int i = 0;
  int amountRead;
  char * ptr;
  ptr = buf;
  while((amountRead = read(sd,ptr,1)) > 0) {
    if(*ptr == '\n') {
      printf("socket line: %s\n", buf);
      return i+1;
    }
    i++;
    ptr++;
  }
}

// Returns true if num was an 1xx level code.
bool isHundredLevel(int num) {
  int temp = num/100;
  if(temp == 1)
    return true;
  return false;
}
  

// Gets the response code from given first line of HTTP response.
int getResponseCode(char* firstLine) {
  printf("in function getResponseCode\n");
  char* ptr = strtok(firstLine," ");
  ptr = strtok(NULL, " ");
  int responseCode = atoi(ptr);
  printf("response Code: %d\n", responseCode);
  return responseCode;
}

//Copies the string up to and not including the given character into returnString and returns number of chars
//copied.
int readUntil(char* buf, char* returnString, char character) {
  int i = 0;
  printf("in function readUntil\n");
  while(buf[i] != character) {
    returnString[i] = buf[i];
    i++;
  }
  printf("readUntil: %s\n",returnString);
  return i;
}

//Copies the line into returnString but does not include \r\n at the end and returns the number of chars copied.
int readLine(char* buf, char * returnString) {
  printf("in function readLine\n");
  char * pointer = buf;
  int i = 0;
  while(buf[i] != '\r' && buf[i+1] != '\n') { 
        returnString[i] = buf[i];
        i++;
  }
  printf("Line: %s\n",returnString);
  return i;
}

// Gets the header content and copied into returnString. Writes nothing into returnString if header not found.
// MAKE SURE the buffer passed to store the content is clean!
void getHeaderContent(char* buf, char* returnString, char* headerType) {
  int lineLength;
  int headerLength;
  char * bp = buf;
  char header[256];
  char temp[BUFLEN];
  char * hp = temp;

  printf("in function getHeaderContent\n");
  while(1) {
    memset(temp,0,sizeof(temp));
    memset(header,0,sizeof(header));
    lineLength = readLine(bp,temp);
    if(strcmp(temp,"") != 0) {
      headerLength = readUntil(temp,header,' ');
    } else {
      printf("reached the end of headers\n");
      return;
    }
    if(strcmp(header, headerType) == 0) {
        hp += (headerLength + 1);
	strncpy(returnString, hp, (lineLength - headerLength - 1));
	printf("header content: %s\n",returnString);
	return;
    }
    // go to next line
    bp += (lineLength + 2);
  }
}

int firstLine(char* request, char* returnString) {
  printf("inside firstLine function\n");
  int firstLine_len;
  int i = 0;
  while(request[i] != '\r' && request[i+1] != '\n') { 
        returnString[i] = request[i];
        i++;
  }
  firstLine_len = i;
  printf("length of first line is: %d\n", firstLine_len);
  printf("first line: %s\n", returnString);
  return firstLine_len;
}

/* Main Worker Thread Function */
void request_handler(void* args) {
  int firstLine_len;
  int bodyLength;
  int n, bytes_to_read;
  int server_sd, new_sd, client_len, port, hostPort, host_sd;
  struct sockaddr_in client,host, empty;
  struct hostent *hostent = malloc(sizeof(struct hostent));
  char *bp, buf[BUFLEN], outbuf[BUFLEN];
  char* strptr;
  char* strptr2;
  char* strptr3;
  char requestType[4];
  char hostAddr[BUFLEN];
  char absolutePath[BUFLEN];
  char portNum[6];
  char temp[BUFLEN];
  char tempHandler[BUFLEN];
  char line[100];
  
  struct Argument * ptr = (struct Argument*) args;
  FILE* fp = ptr->fp;
  server_sd = (int) ptr->sd;

  while (1) {
    
    memset(buf, 0, sizeof(buf));
    memset(portNum,0, sizeof(portNum));
    memset(hostAddr, 0, sizeof(hostAddr));
    memset(requestType,0, sizeof(requestType));
    memset(absolutePath,0, sizeof(absolutePath));
    memset(temp,0,sizeof(temp));
    memset(line,0,sizeof(line));
    memset(outbuf,0,sizeof(outbuf));
    memset(tempHandler,0,sizeof(tempHandler));
    client = empty;
    host = empty;

    printf("Waiting to accept a connection\n");
    if ((new_sd = accept(server_sd, (struct sockaddr *)&client, &client_len)) == -1) {
      fprintf(stderr, "Can't accept client.\n");
      exit(1);
    }   
    bp = buf;
    bytes_to_read = BUFLEN;
    while((n = read(new_sd, bp, bytes_to_read)) > 0) {
      printf("request:%s\n", bp);
      printf("length of request:%d\n", n);
	       /*if(*(bp + (n-1)) == '\n' && 
	       	  *(bp + (n-2)) == '\r' && 
		  *(bp + (n-3)) == '\n' && 
		  *(bp + (n-4)) == '\r') {
		 break;
		 }*/
      if(strcmp(bp + (n-4), "\r\n\r\n") == 0){
	break;
      }
      bp += n;
      bytes_to_read -= n;
    }
    
    if(n == 0 || n == -1) {
      strcpy(outbuf,"HTTP/1.1 400 Bad Request\r\n\r\n");
      write(new_sd,outbuf,strlen(outbuf));
      close(new_sd);
      printf("closed\n");
      continue;
    }
    
    firstLine_len = firstLine(buf,temp);
    strncpy(tempHandler,temp,sizeof(temp));
	
    strptr = strtok(tempHandler, " ");
    strcpy(requestType, strptr);
    printf("request type: %s\n", requestType);

    /* Check if Method is a GET method */
    if(strcmp(requestType,"GET") != 0) {
      memset(outbuf,0,sizeof(outbuf));
      strcpy(outbuf,"HTTP/1.1 405 Method Not Allowed (Only a GET method is allowed)\r\n\r\n");
      write(new_sd, outbuf, strlen(outbuf));
      printf("Sent: %s\n", outbuf);
      close(new_sd);
      continue;
    }

    strptr = strtok(NULL, ":");
    printf("strptr: %s\n",strptr);
    strptr += 7;
    printf("strptr: %s\n",strptr);
    strptr2 = strtok(strptr, "/");
    printf("strptr2: %s\n",strptr2);
    strptr3 = strtok(NULL, " ");
    printf("strptr3: %s\n",strptr3);
    
    int length = strlen(strptr2);
    strptr = strtok(strptr2,":");

    if(strlen(strptr) == length) { // No port
      strncpy(hostAddr,strptr,strlen(strptr));
      strncpy(portNum,"80",2);
      if(strcmp(strptr3,"HTTP/1.1") == 0){
	*absolutePath = '/';
      } else {
	*absolutePath = '/';
	strptr = absolutePath + 1;
	strncpy(strptr, strptr3, strlen(strptr3));
      }
    } else {                        // has port
      strncpy(hostAddr,strptr,strlen(strptr));
      strptr = strtok(NULL, " ");
      strncpy(portNum,strptr,strlen(strptr));
      if(strcmp(strptr3,"HTTP/1.1") == 0){
	*absolutePath = '/';
      } else {
	*absolutePath = '/';
	strptr = absolutePath + 1;
	strncpy(strptr, strptr3, strlen(strptr3));
      }
    }
    memset(tempHandler, 0, sizeof(tempHandler));

    printf("host: %s\n", hostAddr);
    printf("port: %s\n", portNum);
    printf("path: %s\n", absolutePath);

    //check blacklist before making connection
    if (fp != NULL) {
      do {
	if (fgets(line, 100, fp) != NULL) {
	  strptr = strtok(line, ".");
	  strptr = strtok(NULL, "."); //get host name of black list address
	  strptr2 = strtok(hostAddr, ".");
	  strptr2 = strtok(NULL, ".");
	  if (*strptr == *strptr2) {
	    printf("HTTP/1.1 403 Forbidden\r\n\r\n");
	    exit(1);
	  }
	  printf("black list: %s", line);
	} else {
	  break;
	}
      } while (1);
      fclose(fp);
    }

    hostent = gethostbyname(hostAddr);
    bzero((char *) &host, sizeof(host));
    bzero((char *) hostent, sizeof(hostent));

    host.sin_family = AF_INET;
    hostPort = atoi(portNum);
    host.sin_port = htons(hostPort);
    
    bcopy((char *) hostent->h_addr, (char *) &host.sin_addr.s_addr, hostent->h_length);

    if ((host_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      fprintf(stderr, "Can't create a socket.\n");
      exit(1);
    }

    //make new first line
    char newFirstLine[BUFLEN];
    memset(newFirstLine,0,sizeof(newFirstLine));
    strcpy(newFirstLine, "GET ");
    strcat(newFirstLine, absolutePath);
    strcat(newFirstLine, " HTTP/1.1");
    printf("new first line: %s\n", newFirstLine);

    //delete first line from buf
    while (firstLine_len > 0) {
      memmove(buf, buf+1, strlen(buf));
      firstLine_len--;
    }
    strcat(newFirstLine, buf);
    printf("new request: %s\n", newFirstLine);

    //connect to server
    if (connect(host_sd,(struct sockaddr*) &host, sizeof(host)) < 0) {
      printf("Connect failed (on port %d,  %s).\n", host.sin_port, inet_ntoa(host.sin_addr));
      exit(1);
    } else {
      printf("Connection succeeded\n");
    }

    /* Send request to host */
    n = write(host_sd, newFirstLine, strlen(newFirstLine));

    if (n < 0) {
      printf("cannot write to socket\n");
    } else {
      printf("write successful\n");
      int amountRead;
      bp = outbuf;
      memset(outbuf,0,sizeof(outbuf));
      
      /* only read the headers up to \r\n\r\n */
      amountRead = readHeaders(host_sd,bp);

      printf("server response: %s\n",bp);
      write(new_sd,outbuf,amountRead);

      int responseCode;
      memset(temp, 0, sizeof(temp));

      /* Get first line of response and response code.*/
      firstLine_len = firstLine(outbuf, temp);
      responseCode = getResponseCode(temp);

      /* Get Transfer-Encoding or Content-Length header if present */
      memset(temp, 0, sizeof(temp));
      memset(tempHandler, 0, sizeof(tempHandler));
      getHeaderContent(bp, temp, "Transfer-Encoding:");
      getHeaderContent(bp, tempHandler, "Content-Length:");
	      
      if (strcmp(tempHandler, "") != 0) { //Case 2: We are given Content-Length header
	printf("INSIDE CONTENT-LENGTH CASE\n");
	int contentLength = atoi(tempHandler);
	memset(outbuf,0,sizeof(outbuf));
	while(contentLength != 0) {
	  if(contentLength > BUFLEN){
	    bytes_to_read = BUFLEN;
	  } else {
	    bytes_to_read = contentLength;
	  }
	  printf("bytes_to_read is: %d\n",bytes_to_read);
	  if((amountRead = read(host_sd, outbuf, bytes_to_read)) > 0) {
	    printf("amountRead: %d\n",amountRead);
	    //printf("content body: %s\n",outbuf);
	    write(new_sd, outbuf, amountRead);
	    contentLength -= amountRead;
	  }
	  memset(outbuf,0,sizeof(outbuf));
	}
      } else if (strcmp(temp, "chunked") == 0) {  // Case 3: We are given Transfer-Encoding header
	printf("INSIDE CHUNKED CASE\n");

		
	memset(temp,0,sizeof(temp));
	int bytesRead = readSocketLine(host_sd,temp);
	write(new_sd,temp,bytesRead);
	strptr = strtok(temp,"\r");
	bytes_to_read = (int)strtol(strptr,NULL,16);
	printf("bytes_to_read: %d\n",bytes_to_read);

	while(bytes_to_read > 0) {
	  int toRead;
	  bp = outbuf;
	  memset(outbuf,0,sizeof(outbuf));
	  memset(temp,0,sizeof(temp));
	  while(bytes_to_read > 0) {
	    if(bytes_to_read > BUFLEN) {
	      toRead = BUFLEN;
	      bytes_to_read -= BUFLEN;
	    } else {
	      toRead = bytes_to_read;
	      bytes_to_read = 0;
	    }
	    printf("toRead: %d\n",toRead);
	    if((amountRead = read(host_sd, bp, toRead)) > 0) {
	      printf("amountRead: %d\n", amountRead);
	      //printf("chunk content: %s\n", outbuf);
	      int result = write(new_sd, outbuf, amountRead);
	      printf("result: %d\n",result);
	      memset(outbuf,0,sizeof(outbuf));
	      printf("toRead: %d\n",toRead);
	      if(amountRead < toRead) {
		toRead -= amountRead;
		while(toRead > 0) {
		  amountRead = read(host_sd, bp, toRead);
		  printf("amountRead: %d\n",amountRead);
		  write(new_sd, outbuf, amountRead);
		  toRead -= amountRead;
		  printf("toRead 2: %d\n",toRead);
		  memset(outbuf,0,sizeof(outbuf));
		}
	      }
	    }
	  }
	        
	  amountRead = read(host_sd, temp, 2);
	  printf("2 chars: %s\n",temp);
	  write(new_sd,temp,amountRead);
		
	  memset(temp,0,sizeof(temp));
	  bytesRead = readSocketLine(host_sd,temp);
	  //memset(tempHandler,0,sizeof(tempHandler));
	  //strncpy(tempHandler,temp,strlen(temp));
	  write(new_sd,temp,bytesRead);
	  
	  int len = strlen(temp);
	  strptr = strtok(temp, ";");
	  
		
	  if(strlen(strptr) == len){
	    printf("if case 1\n");
	    strptr = strtok(temp, "\r");
	    bytes_to_read = (int)strtol(strptr,NULL,16);
	    printf("bytes_to_read: %d\n",bytes_to_read);
	  } else {
	    printf("if case 2\n");
	    bytes_to_read = (int)strtol(strptr,NULL,16);
	    printf("bytes_to_read: %d\n",bytes_to_read);
	  }
	  /*if(bytes_to_read == 0) {
	    printf("end of response: %s\n",outbuf);
	    strptr = &tempHandler[3];
	    amountRead = read(host_sd,strptr,2);
	    printf("tempHandler: %s\n",tempHandler);
	    write(new_sd,tempHandler,5);
	    }*/
	}
	memset(outbuf,0,sizeof(outbuf));
	amountRead = read(host_sd, outbuf, 2);
	
	printf("end of response: %s\n",outbuf);
	write(new_sd, outbuf, amountRead);
      } else {
		
      }
    }
    // End of request so we close both sockets
    printf("Request complete, closing sockets\n");
    close(host_sd);
    close(new_sd);
  } // end of while loop
  //sleep(5000);
  printf("exiting\n");
  close(host_sd);
  close(new_sd);
}

int main(int argc, char **argv) {

    int n, bytes_to_read;
    int new_sd, client_len, port, hostPort, host_sd;
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
    char line[100];

    
    struct Argument args;    
    switch (argc) {
        case 1:
            port = SERVER_TCP_PORT;
            break;
        case 2:
            port = atoi(argv[1]);
            break;
        case 3: 
            port = atoi(argv[1]);
            args.fp = fopen(argv[2], "r");
            break;
        default:
            fprintf(stderr, "Usage: %s [port] [FilterFile] \n",
                    argv[0]);
            exit(1);
    }
    
    /* Create a stream socket. */
    if ((args.sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket.\n");
        exit(1);
    }
    
    /* Bind an address to the socket */
    memset((char *) &server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(args.sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "Can't bind name to socket.\n");
        exit(1);
    }
    
    /* Listen on socket. */
    listen(args.sd, 5);

    /* Create 4 worker threads. */
    struct Thread *threads[4];
    int error;
    int i = 0;

    while(i < 1) {
      threads[i] = (struct Thread*) createThread((void*) &request_handler, (void*) &args);
      error = runThread(threads[i],NULL);
      if (error != -10) {
	i++;
      }
    }
    int join0 = joinThread(threads[0],NULL);
    int join1 = joinThread(threads[1],NULL);
    int join2 = joinThread(threads[2],NULL);
    int join3 = joinThread(threads[3],NULL);
    
    close(args.sd);
    return 0;

}


