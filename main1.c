// NAME: HARITIMA MANCHANDA CISC 210(HONORS) LAB 7
//FILE: main1.c
// DESCRIPTION: The program creates a socket to a web server.
// Tested with www.google.com 80 /

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//SIZE: size of buffer
#define SIZE 4096

// error: Displays the error message when called and exits the program.
void error(char* msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[SIZE];

	if(argc != 4)
	{
		fprintf(stderr, "Usage:\n  myprog %s \n  hostname %s\n  port %s\n  serverpath\n",argv[0],argv[1], argv[2],argv[3]);
		exit(0);
	}

	portno = atoi(argv[2]);

	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockfd < 0)
	{
		error("ERROR opening socket");
	}

	server = gethostbyname(argv[1]);

	if(server == NULL)
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	bzero((char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);

	serv_addr.sin_port = htons(portno);

	if(connect(sockfd,(const struct sockaddr *)&serv_addr, sizeof(serv_addr))< 0)
	{
		error("ERROR connecting");
	}
	
	strcpy(buffer,"GET /\r\n\r\n"); // GET REQUEST
	strcat(buffer,argv[3]);

	n = send(sockfd, buffer, strlen(buffer),0);

	if(n < 0)
	{
		error("ERROR WRITING TO SOCKET");
	}

	bzero(buffer,4096);

	n = recv(sockfd,buffer,SIZE,0);
	if(n < 0 || strlen(buffer) > SIZE)
	{
		error("ERROR reading from socket");
	}

	fprintf(stdout,buffer);
	printf("\n");

	close(sockfd);
	return 0;

}
