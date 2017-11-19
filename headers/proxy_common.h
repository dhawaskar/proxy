#include <iostream>
#include <string.h>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <openssl/md5.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h> 
#define MAXLINE 1024
#define MAXCLIENTS 1024
#define MAX 100

using namespace std;
extern int sfd,cfd,port;
extern struct sockaddr_in servaddr,cliaddr;
socklen_t client_len=sizeof(cliaddr);



void proxy_check_arg(int count,char **arg){

	if(count<2){
		cout<<"Usage :"<<arg[0]<<" <port_num>"<<endl;
		exit(0);
	}
	else{
		cout<<"Welcome to web proxy server"<<endl;
		port=atoi(arg[1]);
	}

} 

void proxy_create_socket(){
	cout<<"Creating the TCP socket to listen to clients"<<endl;
	sfd=socket(AF_INET,SOCK_STREAM, 0);
	if(sfd<0){
		cout<<"I am in "<<__FUNCTION__<<endl;
		perror("socket");
		exit(1);
	}
	else{
		cout<<"Scoket is created:"<<sfd<<endl;
	}	
}

void proxy_initialise_socket(){
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr =inet_addr("127.0.0.1");
	servaddr.sin_port = htons(port);
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (struct sockaddr *) &servaddr, sizeof(servaddr));
}

void proxy_bind_socket(){
	cout<<"Binding the socket to the local address"<<endl;
	if(bind(sfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0){
		cout<<"I am in "<<__FUNCTION__<<endl;
		perror("bind:");
		exit(1);
	}	
	else{
		cout<<"Binding is done"<<endl;	
	}
}

void proxy_listen_socket(){
	if((listen(sfd,MAXCLIENTS))==-1){
		cout<<"I am in "<<__FUNCTION__<<endl;
		perror("Listening of server failed");
		exit(1);
	}
	else{
		cout<<"Listen is done"<<endl;
	}
}

void proxy_accept_socket(){
	cfd=accept(sfd,(struct sockaddr *) &cliaddr,&client_len);
	if(cfd==-1){
		cout<<"I am in "<<__FUNCTION__<<endl;
		perror("accept:");
	}
	else{
		cout<<"client is accepted"<<endl;
	}
}
