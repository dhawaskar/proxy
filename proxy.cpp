#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "headers/proxy_common.h"

using namespace std;

int sfd,cfd,port;
struct sockaddr_in servaddr,cliaddr;

void client_handle(){


}

int main(int argc,char **argv){
	int childpid=0;
	proxy_check_arg(argc,argv);
	proxy_create_socket();
	proxy_initialise_socket();
	proxy_bind_socket();
	proxy_listen_socket();
	while(1){
		proxy_accept_socket();
		if((childpid = fork ()) == 0){
			close(sfd);
			client_handle();
			cout<<"Done handling the client"<<endl;
		}
		close(cfd);
	}		
	close(sfd);
	return 0;

}
