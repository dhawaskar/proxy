#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include "headers/proxy_common.h"

using namespace std;

int sfd,cfd,mfd=0,port,server_port=0;
struct sockaddr_in servaddr,cliaddr,main_addr;
socklen_t client_len=sizeof(cliaddr);
vector <string> sv;
vector <string> sv1; 
char header[MAXLINE],server_name[MAX],header_server[MAXLINE],header_client[MAXLINE],ip_address[MAX],cached_ip[MAX],file_path[MAX],content_type[MAX],content_len[MAX];

char * header_generation(){
	cout<<"Header generation"<<endl;
	int len;
	char server[MAX];
	bzero(server,MAX);
        strncpy(server,server_name,strlen(server_name));
        cout<<"The server name should be ****************\t"<<server<<endl;
	bzero(header,MAXLINE);
	strncpy(header,"GET ",strlen("GET "));
	if(strlen(file_path)>1)
		strncat(header,file_path,strlen(file_path));
	else	strncat(header,"/index.html",strlen("/index.html"));
	strncat(header," ",1);
	strncat(header,sv[2].c_str(),strlen(sv[2].c_str()));
	strncat(header,"\r\n",2);
	strncat(header,"HOST: ",strlen("HOST: "));
	strncat(header,server,strlen(server));
	strncat(header,"\r\n",2);
	strncat(header,"Connection: Close",strlen("Connection: Close"));
	strncat(header,"\r\n\r\n",4);
	len=strlen(header);
	header[len]='\0';
	return header;
}


void get_server_process(){
	char *temp,buf[MAXLINE],header_err[MAXLINE],serv_buff[MAX],file[MAX],filename[MAX];
	int len,n,childpid;
	fstream fd;
	cout<<"Lets talk to main server:\t"<<server_name<<"\tat port:\t"<<server_port<<endl;
	bzero(serv_buff,MAX);
	strcpy(serv_buff,server_name);
	serv_buff[strlen(serv_buff)]='\0';
	if(proxy_non_block(serv_buff)){
		if(proxy_is_cache_server(serv_buff)){
			cout<<"Server entry is already cached"<<endl;
			cout<<"Got the cached IP address"<<cached_ip<<endl;
			strcpy(ip_address,cached_ip);
		}
		else{
			cout<<"server entry is not cached yet and lets add it"<<endl;
			proxy_hostname_to_ip(serv_buff);
			cout<<"Got new IP address is \t"<<ip_address<<endl;
			proxy_cached_server(serv_buff,ip_address);
		}
		if(strlen(ip_address)>1){
			if(proxy_non_block(ip_address)){
				main_addr.sin_family = AF_INET;
				main_addr.sin_addr.s_addr =inet_addr(ip_address);
				main_addr.sin_port = htons(server_port);
				mfd=proxy_create_socket();
				cout<<"The socket created to talk to main server"<<mfd<<endl;
				if(connect(mfd,(struct sockaddr *)&main_addr,sizeof(main_addr))<0){
					cout<<"I am in \t"<<__FUNCTION__<<endl;
					perror("connect:");
				}
				else{
					cout<<"Connection is establised with\t"<<server_name<<endl;
					strcpy(header_server,header_generation());
					header_server[strlen(header_server)]='\0';
					cout<<"generated header to send to main server:\n"<<header_server<<endl;
					if(n=send(mfd,header_server,strlen(header_server),0)<0)
						perror("sendto");	
					else{
						cout<<"sent the request to main server"<<endl;
					}
					cout<<"Receive the response from main server"<<endl;
					bzero(buf,MAX);
					n=recv(mfd,buf,MAXLINE,0);
					send(cfd,buf,n,0);
					int flag=VERYLARGEMAX;
					while(flag--){
						bzero(buf,MAX);
						n=recv(mfd,buf,1,0);
						send(cfd,buf,n,0);
					}
					close(mfd);
				}
			}
			else{
				//add code here for error blocked IP
				bzero(header_err,MAXLINE);
                        	strcpy(header_err,"HTTP/1.1 200 OK\r\nContent-Length:46\r\nContent-Type: text/html\r\nConnection: Keepalive\r\n\r\n");
                        	cout<<"header to be sent to client is \n"<<header_err<<endl;
                        	send(cfd,header_err,strlen(header_err),0);
                        	bzero(buf,MAXLINE);
                        	strcpy(buf,"<html>\r\n<p>ERROR 403 Forbidden</p>\r\n</html>\r\n\r\n");
                        	send(cfd,buf,strlen(buf),0);
				return ;
			}		
		}
		else{
			cout<<"Sorry the IP is not obtained!!!"<<endl;
			//send the server not found
			bzero(header_err,MAXLINE);
		        strcpy(header_err,"HTTP/1.1 200 OK\r\nContent-Length:61\r\nContent-Type: text/html\r\nConnection: Keepalive\r\n\r\n");
		        cout<<"header to be sent to client is \n"<<header_err<<endl;
		        send(cfd,header_err,strlen(header_err),0);
		        bzero(buf,MAXLINE);
		        strcpy(buf,"<html>\r\n<p>IP not found, give valid hostname</p>\r\n</html>\r\n\r\n");
		        send(cfd,buf,strlen(buf),0);
			return;
		}	
	}
	else{
		//add code here for error: blocked host
		bzero(header_err,MAXLINE);
                strcpy(header_err,"HTTP/1.1 200 OK\r\nContent-Length:43\r\nContent-Type: text/html\r\nConnection: Keepalive\r\n\r\n");
                cout<<"header to be sent to client is \n"<<header_err<<endl;
                send(cfd,header_err,strlen(header_err),0);
                bzero(buf,MAXLINE);
                strcpy(buf,"<html>\r\n<p>ERROR 403 Forbidden\r\n</html>\r\n\r\n");
                send(cfd,buf,strlen(buf),0);
                return ;
	}
}

void get_process(){
	int len;
	vector <string> com;
	string segment;
	char server_sock[MAX],temp_port[MAX],temp[MAX];
	len=strlen(sv[1].c_str());
	bzero(server_sock,MAX);
	bzero(server_name,MAX);
	bzero(temp,MAX);
	bzero(temp_port,MAX);
	bzero(file_path,MAX);
	strncpy(server_sock,sv[1].c_str(),len);
	server_sock[len]='\0';
	stringstream s(server_sock);
	while(getline(s,segment,':')){
		com.push_back(segment);
	}
	strcpy(temp,com[1].c_str());
	strncpy(server_name,temp+2,strlen(temp)-2);
	bzero(temp,MAX);
	if(strstr(server_name,"/")){
		strcpy(temp,server_name);
		strcpy(server_name,strtok(server_name,"/"));
		server_name[strlen(server_name)]='\0';
		strncpy(file_path,temp+(strlen(server_name)),strlen(temp)-strlen(server_name));
	}
	cout<<"The client wants connection with :"<<server_name<<"and file is \t:"<<file_path<<endl;
	if(com.size()>=3){
		cout<<"Port is given"<<endl;
		//strncpy(server_name,"http://",strlen("http://"));
		//strcat(server_name,strtok(server_sock,":"));
		//strcat(server_name,"/");
		
		cout<<"server name *****************"<<server_name<<endl;
		//strcpy(temp_port,strtok(NULL,"NULL"));
		//strcpy(temp_port,"80");
		strcpy(temp_port,com[2].c_str());
	}
	else{
		cout<<"port is not given"<<endl;
		//strcpy(server_name,sv[1].c_str());
              	//strcpy(temp_port,"8001");
		if(strcmp(server_name,"localhost")==0)
			strcpy(temp_port,"8001");
		else
			strcpy(temp_port,"80");
	}
	server_port=atoi(temp_port);
	cout<<"Server name is\t"<<server_name<<"\tport is \t"<<server_port<<endl;
	get_server_process();
}

void client_handle(){
	int n;
	char buf[MAXLINE],buf1[MAXLINE],header_err[MAX];
	string command,temp,command1,temp1;
	fstream fd,fd1,fd2;
	fd.open("temp",fstream::out);
	bzero(buf,MAXLINE);
	n=recv(cfd,buf,MAXLINE,0);
	cout<<"Received from client:"<<buf<<endl;
	fd.write(buf,strlen(buf)-2);
	fd.close();
	//read hostname
	fd2.open("temp",fstream::in);
	bzero(buf1,MAXLINE);
	fd2.getline(buf1,MAXLINE);
	bzero(buf1,MAXLINE);
        fd2.getline(buf1,MAXLINE);
	fd2.close();
	command1=buf1;
	stringstream s1(command1);
	while(s1>>temp1)
		sv1.push_back(temp1);		
	fd1.open("temp",fstream::in);
	bzero(buf,MAXLINE);
	fd1.getline(buf,MAXLINE);
	fd1.close();
	command=buf;
	stringstream s(command);
	while(s>>temp)
		sv.push_back(temp);
	if((strncmp(sv[0].c_str(),"GET",3)==0)||(strncmp(sv[0].c_str(),"CONNECT",7)==0)){
	//if(strncmp(sv[0].c_str(),"GET",3)==0){
		cout<<"This is GET command process for client"<<endl;
		get_process();
	}
	else{
		//send the error of invalid method
		cout<<"Undefined function and send error header"<<endl;
		bzero(header_err,MAXLINE);
        	strcpy(header_err,"HTTP/1.1 200 OK\r\nContent-Length:43\r\nContent-Type: text/html\r\nConnection: Keepalive\r\n\r\n");
        	cout<<"header to be sent to client is \n"<<header_err<<endl;
		send(cfd,header_err,strlen(header_err),0);
                bzero(buf,MAXLINE);
		strcpy(buf,"<html>\r\n<p>400 Bad Request</p>\r\n</html>\r\n\r\n");
                send(cfd,buf,strlen(buf),0);
	}	
}

int main(int argc,char **argv){
	int childpid=0;
	char dir[MAX];
	fstream fd;

	proxy_check_arg(argc,argv);

	//Create directory to cache IP
	bzero(dir,MAX);
        getcwd(dir,MAX);
        strncat(dir,"/proxy_dir",strlen("/proxy_dir"));
        dir[strlen(dir)]='\0';
        mkdir(dir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        strncat(dir,"/hostname_ip_cache",strlen("/hostname_ip_cache"));
        dir[strlen(dir)]='\0';
        fd.open(dir,fstream::app);
        fd.close();
	//end

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
                	close(cfd);
			//close(mfd);
                        printf("\nDone handling the client\n");
                        exit(0);
                }
		close(cfd);
        }
	return 0;

}
