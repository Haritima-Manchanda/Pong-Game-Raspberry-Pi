//NAME: HARITIMA MANCHANDA CISC210(HONORS)
//LAB 8
//file main.c
//DESCRIPTION: The program creates the server side of a socket
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

//error: sends an error with the message passed during function call
//	exits the program
void error(char *msg)
{
	perror(msg);
	exit(0);
}

//socketCreate: creates the socket using socket()
int socketCreate()
{
	int server_socket;
	printf("Create the socket\n");

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	return server_socket;
}

//bindCreatedSocket(): binds the socket
int bindCreatedSocket(int server_socket, int portno)
{
	int n = -1;
	struct sockaddr_in serv_addr;

	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	n = bind(server_socket,(struct sockaddr*)&serv_addr,sizeof(serv_addr));

	return n;

}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno,clilen;
	char buffer[256];
	char newBuffer[1024];

	struct sockaddr_in serv_addr, cli_addr;
	int n;
	
	if(argc == 1)
	{
		portno = 8080;
	}
	else if(argc == 2)
	{
		portno = atoi(argv[1]);	
	}
	else
	{
		error("Wrong number of arguments provided");
	}

	//CREATE SOCKET
	sockfd = socketCreate();
	if(sockfd < 0)
	{
		error("Error in Creating Socket");
	}
	printf("Socket Created\n");

	//BIND
	if(bindCreatedSocket(sockfd, portno)< 0)
	{
		error("Error Bind Failed");
	}
	printf("Bind Done\n");

	//LISTEN
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);

	printf("NewSocket Created\n");
	if(newsockfd < 0)
	{
		error("Error in Accepting");
	}

	bzero(buffer, 256);
	n = recv(newsockfd, buffer, 255, 0);
	if(n<0)
	{
		error("ERROR reading from socket");
	}

	printf("Here is the message: %s\n", buffer);

	//CHECKING IF ITS A GET REQUEST
	if(strncmp(buffer,"GET",3)== 0)
	{
		strcpy(newBuffer,"<html>body style='color:white;background:url(https://www.eecis.udel.edu/~silber/sw.png)'><div style='width:400px;padding:10px;margin:100px auto;background:black;'><h1>May the4<sup>th</sup>be with you...</h1><h2>...Always</h2></div></body></html>");

		n = send(newsockfd,newBuffer,strlen(newBuffer),0);
	}
	else
	{
		n = send(newsockfd, "I got your message",18,0);
	}


	if(n < 0)
	{
		error("ERROR writing to socket");
	}
	printf("\nMessage Sent");
	close(newsockfd);
	close(sockfd);
	return 0;
}
