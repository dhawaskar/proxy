#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include "headers/proxy_common.h"

using namespace std;

int sfd,cfd,mfd,port,server_port;
struct sockaddr_in servaddr,cliaddr,main_addr;
socklen_t client_len=sizeof(cliaddr);
vector <string> sv; 
char header[MAXLINE],server_name[MAX],header_server[MAXLINE],header_client[MAXLINE],ip_address[MAX];


char * header_generation(){
	cout<<"Header generation"<<endl;
	int len;
	bzero(header,MAXLINE);
	strncpy(header,"GET ",strlen("GET "));
	strncat(header,server_name,strlen(server_name));
	strncat(header," ",1);
	strncat(header,sv[2].c_str(),strlen(sv[2].c_str()));
	/*strncat(header,"\r\n",2);
	strncat(header,"Host: ",strlen("Host: "));
	strncat(header,server_name,strlen(server_name));
	strncat(header,"\r\n",2);
	strncat(header,"Connection: Close",strlen("Connection: Close"));
	strncat(header,"\r\n",2);*/
	len=strlen(header);
	header[len]='\0';
	cout<<"Header length is \t"<<len<<endl;
	return header;
}


int hostname_to_ip(char hostname[MAX])
{
	struct hostent *he;
    struct in_addr **addr_list;
    int i,len;
	strcpy(hostname,strtok(hostname,"/"));
	len=strlen(hostname);
	hostname[len]='\0';
    if ( (he = gethostbyname( hostname ) ) == NULL)
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

	bzero(ip_address,MAX);
    for(i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcpy(ip_address , inet_ntoa(*addr_list[i]) );
        return 0;
    }

    return 1;
}

void get_server_process(){
	char *temp,buf[MAXLINE];
	int len,n;
	fstream fd;
	cout<<"Lets talk to main server:\t"<<server_name<<"\tat port:\t"<<server_port<<endl;
	hostname_to_ip(strstr(server_name,"www."));
	cout<<"IP address is \t"<<ip_address<<endl;
	main_addr.sin_family = AF_INET;
	main_addr.sin_addr.s_addr =inet_addr(ip_address);
	main_addr.sin_port = htons(server_port); 
	mfd=proxy_create_socket();
	if(connect(mfd,(struct sockaddr *)&main_addr,sizeof(main_addr))<0){
		cout<<"I am in \t"<<__FUNCTION__<<endl;
		perror("connect:");
		return;
	}
	else{
		cout<<"Connection is establised with\t"<<server_name<<endl;
		temp=header_generation();
		len=strlen(temp);
		strncpy(header_server,temp,len);
		header_server[len]='\0';
		cout<<"generated header to send to main server:\n"<<header_server<<endl;
		if(n=sendto(mfd,header_server,len,0,(struct sockaddr *)&main_addr,sizeof(main_addr))<0)
			perror("sendto");	
		else{
			cout<<"sent the request to main server"<<endl;
		}
		cout<<"Receive the response from main server"<<endl;
		bzero(buf,MAXLINE);
		fd.open("main_server_resp",fstream::out);
		n=recv(mfd,buf,MAXLINE,0);
		if(n<0){
			perror("recv");
		}
		cout<<"Received from main server\n"<<buf<<endl;
		fd.write(buf,n);	
		fd.close();
	}		
}

void get_process(){
	int len;
	char server_sock[MAX],temp_port[MAX];
	len=strlen(sv[1].c_str());
	bzero(server_sock,MAX);
	bzero(server_name,MAX);
	bzero(temp_port,MAX);
	strncpy(server_sock,sv[1].c_str(),len);
	server_sock[len]='\0';
	cout<<"The client wants connection with :"<<server_sock<<endl;
	if(strstr(server_sock,"http")==NULL){
		cout<<"Port is given"<<endl;
		strncpy(server_name,"http://",strlen("http://"));
		strcat(server_name,strtok(server_sock,":"));
		strcat(server_name,"/");
		//strcpy(temp_port,strtok(NULL,"NULL"));
		strcpy(temp_port,"80");
	}
	else{
		cout<<"port is not given"<<endl;
		strcpy(server_name,sv[1].c_str());
                strcpy(temp_port,"80");
	}
	server_port=atoi(temp_port);
	cout<<"Server name is\t"<<server_name<<"\tport is \t"<<server_port<<endl;
	get_server_process();
}

void client_handle(){
	int n;
	char buf[MAXLINE];
	string command,temp;
	fstream fd,fd1;
	fd.open("temp",fstream::out);
	bzero(buf,MAXLINE);
	n=recv(cfd,buf,MAXLINE,0);
	cout<<"Received from client:"<<buf<<endl;
	fd.write(buf,strlen(buf)-2);
	fd.close();
	fd1.open("temp",fstream::in);
	bzero(buf,MAXLINE);
	fd1.getline(buf,MAXLINE);
	fd1.close();
	command=buf;
	stringstream s(command);
	while(s>>temp)
		sv.push_back(temp);
	if((strncmp(sv[0].c_str(),"GET",3)==0)||(strncmp(sv[0].c_str(),"CONNECT",7)==0)){
		cout<<"This is GET command process for client"<<endl;
		get_process();
	}
	else{
		cout<<"Undefined function"<<endl;
	}	
}

int main(int argc,char **argv){
	int childpid=0;
	proxy_check_arg(argc,argv);
	sfd=proxy_create_socket();
	proxy_initialise_socket();
	proxy_bind_socket();
	proxy_listen_socket();
	while(1){
		proxy_accept_socket();		
                if ( (childpid = fork ()) == 0 )
                {
                        close(sfd);
                        client_handle();
                        printf("\nDone handling the client\n");
                        exit(0);
                }
                close(cfd);
        }
	close(sfd);
	return 0;

}
