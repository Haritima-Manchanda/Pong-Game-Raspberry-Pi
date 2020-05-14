//Names: Becky Ren Haritima Manchanda
//Final Project
//Description: This program creates a ping pong game to be played on two pis via client and server connection
#include <stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys.types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "sense.h"
int running = 1;
//function to stop program
void InterruptHandler(int sig){
	printf("Game stopped");
	running =0;
}
//function to print an error and exit program
void error(char *msg){
	perror(msg);
	exit(0);}

//server_socketCreate: creates the server socket using socket()
int server_socketCreate(){
	int server_socket;
	printf("Create the socket\n");
	server_socket = socket(AF_INET,SOCK_STREAM,0);
	return server_socket;}

//client_socketCreate()creates the client socket using socket()	
int client_socketCreate(){
	int client_socket;
	printf("Create the socket\n");
	client_socket = socket(AF_INET_SOCK_STREAM,IPROTO_TCP);
	return client_socket;}	
//bindCreatedSocket binds the socket
int bindCreatedSocket(int server_socket, int portno){
	int n = -1;
	struct sockaddr_in serv_addr;
	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	n = bind(server_socket,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
	return n;}

int main(int argc, char *argv[]){
	int sockfd, newsockfd,portno,clilen;
	struct sockaddr_in serv_addr,cli_addr;
	struct hostent *server;
	char buffer[256];
	char newbuffer[1024];
	//check to start as server or client
	if(argc==2){
		portno = atoi(argv[1]);}
	else if (argc==3){
		portno = atoi(argc[1]);
		server = gethostbyname(argv[2])
	}
	else{
		error("Invalid number of entries. Try again");
	}


}	
