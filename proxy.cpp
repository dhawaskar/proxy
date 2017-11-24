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
vector <string> sv1; 
char header[MAXLINE],server_name[MAX],header_server[MAXLINE],header_client[MAXLINE],ip_address[MAX],cached_ip[MAX];

char * header_generation(){
	cout<<"Header generation"<<endl;
	int len;
	char server[MAX];
	bzero(server,MAX);
        strncpy(server,server_name,strlen(server_name));
        //strtok(server,"/");
        //strcpy(server,strtok(NULL,"/"));
        cout<<"The server name should be ****************\t"<<server<<endl;
	bzero(header,MAXLINE);
	strncpy(header,"GET ",strlen("GET "));
	//strncat(header,"http://",strlen("http://"));
	//strncat(header,server_name,strlen(server_name));
	strncat(header,"/index.html",strlen("/index.html"));
//	strncat(header,"/godlytalias/My-Codes/blob/master/B%2Btree.c",strlen("/godlytalias/My-Codes/blob/master/B%2Btree.c"));
	//strncat(header,"/exec/obidos/subst/home/home.html",strlen("/exec/obidos/subst/home/home.html"));
	//strncat(header,"/",1);
	strncat(header," ",1);
	strncat(header,sv[2].c_str(),strlen(sv[2].c_str()));
	strncat(header,"\r\n",2);
	strncat(header,"Host: ",strlen("Host: "));
	//strncat(header,sv1[1].c_str(),strlen(sv1[1].c_str()));
	strncat(header,server,strlen(server));
	//strncat(header,"localhost",strlen("localhost"));
	strncat(header,"\r\n",2);
	strncat(header,"Connection: Close",strlen("Connection: Close"));
	strncat(header,"\r\n\r\n",4);
	len=strlen(header);
	header[len]='\0';
	//cout<<"Header length is \t"<<len<<endl;
	return header;
}



void send_response_client(){
	//create the response file from server
	char buf[MAXLINE],file_type[MAX],header[MAXLINE],file_size_str[MAX],filename[MAX];
        int n,file_size,m,len;
        bzero(filename,MAX);
        strcpy(filename,"response.html");
        filename[strlen(filename)]='\0';
        fstream fd,fd1,fd2;
        fd.open("main_server_resp",fstream::in|fstream::binary);
        fd1.open(filename,fstream::out|fstream::binary);
        if(fd.fail())   perror("open");
        bzero(buf,MAXLINE);
        while(fd.getline(buf,MAXLINE)){
                if(strlen(buf)<=1){
                        cout<<"The empty line is reached"<<endl;
                        bzero(buf,MAXLINE);
                        while(fd.read(buf,1)){
                                //cout<<buf<<endl;
                                fd1.write(buf,1);
                                //fd1.write("\r\n",2);
                                bzero(buf,MAXLINE);
                        }
                        fd1.close();
                        fd.close();
                        break;
                }
                bzero(buf,MAXLINE);
        }
	//calculate the file_size
	file_size=proxy_calculate_size(filename);
	//send the file to server
	bzero(header,MAXLINE);
	strcpy(header,"HTTP/1.1 200 OK\r\nContent-Length:");
	sprintf(file_size_str,"%d",file_size);
	len=strlen(file_size_str);
	file_size_str[len]='\0';
	cout<<"The file size obtained is "<<file_size_str<<endl;
	strcat(header,file_size_str);	
	strcat(header,"\r\nContent-Type: text/html\r\nConnection: Keepalive\r\n\r\n");
	cout<<"header to be sent to client is \n"<<header<<endl;
	n=send(cfd,header,strlen(header),0);
	if(n<0) perror("send header to client");
	//now send the actual file
	if(file_size>0){
		fd2.open(filename,fstream::in|fstream::binary);
		if(fd2.fail()){
			cout<<"you are in"<<__FUNCTION__<<endl;
			perror("open:");
		}
		else{
			bzero(buf,MAXLINE);
			while(fd2.read(buf,file_size)){
				send(cfd,buf,file_size,0);
				bzero(buf,MAXLINE);
			}
			cout<<"Sent the complete file to client"<<endl;
		}
	}
	//Still need to figure out *************
	else{
		cout<<"Nothing is received from client"<<endl;
		fd2.open("test.html",fstream::in|fstream::binary);
                if(fd2.fail()){
                        cout<<"you are in"<<__FUNCTION__<<endl;
                        perror("open:");
                }
                else{
                        bzero(buf,MAXLINE);
                        while(fd2.read(buf,190)){
                                send(cfd,buf,190,0);
                                bzero(buf,MAXLINE);
                        }
                }
	}
}


void get_server_process(){
	char *temp,buf[MAXLINE],header_err[MAXLINE],serv_buff[MAX],file[VERYLARGEMAX];
	int len,n;
	fstream fd;
	cout<<"Lets talk to main server:\t"<<server_name<<"\tat port:\t"<<server_port<<endl;
	bzero(serv_buff,MAX);
	//strcpy(serv_buff,strstr(server_name,"www."));
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
				//main_addr.sin_addr.s_addr =inet_addr("127.0.0.1");
				main_addr.sin_port = htons(server_port); 
				//main_addr.sin_port = htons(8001); 
				mfd=proxy_create_socket();
				if(connect(mfd,(struct sockaddr *)&main_addr,sizeof(main_addr))<0){
					cout<<"I am in \t"<<__FUNCTION__<<endl;
					perror("connect:");
					bzero(header_err,MAXLINE);
                                	strcpy(header_err,"HTTP/1.1 200 OK\r\nContent-Length:46\r\nContent-Type: text/html\r\nConnection: Keepalive\r\n\r\n");
                                	cout<<"header to be sent to client is \n"<<header_err<<endl;
                                	send(cfd,header_err,strlen(header_err),0);
                                	bzero(buf,MAXLINE);
                                	strcpy(buf,"<html>\r\n<p>ERROR CONNECTION FAILED</p>\r\n</html>\r\n\r\n");
                                	send(cfd,buf,strlen(buf),0);
					return;
				}
				else{
					cout<<"Connection is establised with\t"<<server_name<<endl;
					temp=header_generation();
					len=strlen(temp);
					strncpy(header_server,temp,len);
					header_server[len]='\0';
					/*char *header_server = "GET /index.html HTTP/1.1\r\nHost: www.amazon.com\r\n\r\n";*/
					cout<<"generated header to send to main server:\n"<<header_server<<endl;
					if(n=sendto(mfd,header_server,strlen(header_server),0,(struct sockaddr *)&main_addr,sizeof(main_addr))<0)
						perror("sendto");	
					else{
						cout<<"sent the request to main server"<<endl;
					}
					cout<<"Receive the response from main server"<<endl;
					bzero(buf,MAXLINE);
					fd.open("main_server_resp",fstream::out);
					//content
					bzero(file,VERYLARGEMAX);	
					n=recv(mfd,buf,VERYLARGEMAX,0);
					if(n<0){
						perror("recv");
					}
					cout<<"Received from main server\n"<<buf<<endl;
					fd.write(buf,n);	
					fd.close();
					send_response_client();
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
	strncpy(server_sock,sv[1].c_str(),len);
	server_sock[len]='\0';
	stringstream s(server_sock);
	while(getline(s,segment,':')){
		com.push_back(segment);
	} 
	strcpy(temp,com[1].c_str());
	strncpy(server_name,temp+2,strlen(temp)-2);
	cout<<"The client wants connection with :"<<server_sock<<endl;
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
                        printf("\nDone handling the client\n");
                        exit(0);
                }
                close(cfd);
        }
	close(sfd);
	return 0;

}
