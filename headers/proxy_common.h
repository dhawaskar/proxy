#include <iostream>
#include <time.h>
#include <openssl/md5.h>
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
#define VERYLARGEMAX 1000000
using namespace std;
extern int sfd,cfd,port;
extern struct sockaddr_in servaddr,cliaddr;
extern socklen_t client_len;
extern char ip_address[MAX],cached_ip[MAX];
extern char file_type[MAX];
extern char send_file_type[MAX];


void  send_file_type_predict(char str1[MAX]){
        bzero(send_file_type,MAX);
	if(strlen(str1)==0){
		strcpy(send_file_type," ");
	}
        else if(strcmp(str1,".html")==0){
                strcpy(send_file_type,"text/html");
        }
        else if(strcmp(str1,".htm")==0){
                strcpy(send_file_type,"text/html");
        }
        else if(strcmp(str1,".txt")==0){
                strcpy(send_file_type,"text/plain");
        }
        else if(strcmp(str1,".png")==0){
                strcpy(send_file_type,"image/png");
        }
        else if(strcmp(str1,".gif")==0){
                strcpy(send_file_type,"image/gif");
        }
        else if(strcmp(str1,".jpg")==0){
                strcpy(send_file_type,"image/jpg");
        }
        else if(strcmp(str1,".css")==0){
                strcpy(send_file_type,"text/css");
        }
        else if(strcmp(str1,".js")==0){
                strcpy(send_file_type,"text/javascript");
        }
        else if(strcmp(str1,".ico")==0){
                strcpy(send_file_type,"image/x-icon");
        }
	else{
		strcpy(send_file_type," ");
	}
}

void file_type_predict(char buf[MAX]){
	cout<<"Predicting the file path for \t"<<buf<<endl;
	bzero(file_type,MAX);
        vector <string> fv;
        string segment;
        stringstream s(buf);
        while(getline(s,segment,'/')){
                fv.push_back(segment);
        }
        //cout<<fv[fv.size()-1].c_str()<<endl;
	if(strstr(fv[fv.size()-1].c_str(),"."))
        	strcpy(file_type,strstr(fv[fv.size()-1].c_str(),"."));
	else
		strcpy(file_type,"");
}


int compute_md5sum(char *buffer,int file_size){
        unsigned char digest[MD5_DIGEST_LENGTH];
        MD5((unsigned char*)buffer, file_size, (unsigned char*)&digest);
        char mdString[32];
        for(int i = 0; i < 16; i++)
                sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
        //cout<<"MD5sum is: "<<mdString<<endl;
        int v1, v2, v3, v4;
        sscanf( &mdString[0], "%x", &v1 );
        sscanf( &mdString[8], "%x", &v2 );
        sscanf( &mdString[16], "%x", &v3 );
        sscanf( &mdString[24], "%x", &v4 );
        int hash = v1 ^ v2 ^ v3 ^v4;
        //cout<<"hash value:"<<abs(hash)<<endl;
        return abs(hash);
}

int proxy_non_block(char host[MAX]){
	fstream fd;
	char dir[MAX],buf[MAX];
	int ip1,ip2;
	getcwd(dir,MAX);
	strncat(dir,"/block_ip_host",strlen("/block_ip_host"));
	dir[strlen(dir)]='\0';
	cout<<"Checking the file \t"<<dir<<"\tfor blocked IP and hosts for \t" <<host<<endl;
	fd.open(dir,fstream::in|fstream::binary);
	if(fd.fail()){
		cout<<"I am in "<<__FUNCTION__<<endl;
		perror("open");
	}
	else{
		bzero(buf,MAX);
		while(fd.getline(buf,MAX)){
			cout<<"Checking the line \t"<<buf<<"with\t"<<host<<endl;
			if(strncmp(buf,host,strlen(buf))==0){
				cout<<"This is blocked host"<<endl;
				return 0;
			}
			bzero(buf,MAX);
		}
		if(fd.eof()){
			cout<<"This is not blocked"<<endl;
			return 1;
		}
	}
}

void proxy_cached_server(char server[MAX],char ip_address[MAX]){
        char dir[MAX];
        fstream fd;
        bzero(dir,MAX);
        getcwd(dir,MAX);
        strncat(dir,"/proxy_dir/hostname_ip_cache",strlen("/proxy_dir/hostname_ip_cache"));
        dir[strlen(dir)]='\0';
	cout<<"Filename is *************************************************** \t"<<dir<<endl;
        fd.open(dir,fstream::app|fstream::binary);
        if(fd.fail()){
                cout<<"I am in "<<__FUNCTION__<<endl;
                perror("open:");
        }
        else{
                cout<<"Lets add the \t"<<server<<"and \t"<<ip_address<<"to the proxy cache"<<endl;
                fd.write(server,strlen(server));
                fd.write("\t",1);
                fd.write(ip_address,strlen(ip_address));
                fd.write("\r\n",2);
                fd.close();
        }
}

int  proxy_is_cache_server(char hostname[MAX]){
	fstream fd;
        char dir[MAX],buf[MAX];                                        
        bzero(dir,MAX);                                                
        getcwd(dir,MAX);                                               
        strncat(dir,"/proxy_dir/hostname_ip_cache",strlen("/proxy_dir/hostname_ip_cache"));
        dir[strlen(dir)]='\0';                                         
	cout<<"Filename is *************************************************** \t"<<dir<<endl;
        fd.open(dir,fstream::in|fstream::binary);                      
        if(fd.fail()){
                cout<<"I am in "<<__FUNCTION__<<endl;                  
                perror("open:");
		return 0;
        }       
        else{                                                          
                while(fd.getline(buf,MAX)){                            
                        if((strstr(buf,hostname)!=NULL)&&(strlen(buf)>1)){
                                cout<<"Found the hostname"<<endl;
                                strtok(buf,"\t");                      
                                bzero(cached_ip,MAX);
                                strcpy(cached_ip,strtok(NULL,"NULL")); 
                                fd.close();
                                return 1;
                        }       
                        bzero(buf,MAX);
                }       
                if(fd.eof()){
                        cout<<"Hostname not found, add it to proxy cache"<<endl;
                        return 0;
                }
        }

}

int proxy_hostname_to_ip(char hostname[MAX])
{
        struct hostent *he;
        struct in_addr **addr_list;
        int i,len;
        strcpy(hostname,strtok(hostname,"/"));
        len=strlen(hostname);
        hostname[len]='\0';
	cout<<"Find the IP address of \t"<<hostname<<endl;
        if ( (he = gethostbyname( hostname ) ) == NULL){
                // get the host info
                perror("gethostbyname");
                return 1;
        }
        addr_list = (struct in_addr **) he->h_addr_list;
        bzero(ip_address,MAX);
        for(i = 0; addr_list[i] != NULL; i++){
                //Return the first one;
                strcpy(ip_address , inet_ntoa(*addr_list[i]) );
                return 0;
        }
        return 1;
}

int proxy_calculate_size(char filename[MAX]){
	int len;
	size_t file_size;
	fstream fd;
	len=strlen(filename);
	filename[len]='\0';
	cout<<"calculating the file size of\t:"<<filename<<endl;
	fd.open(filename,fstream::in);
	if(fd.fail()){
		cout<<"I am in \t:"<<__FUNCTION__<<endl;
		perror("open:");
		return 0;
	}
	else{
		fd.seekg(0,fstream::end);
		file_size=fd.tellg();
		fd.close();
		return file_size;
	}
}

void proxy_check_arg(int count,char **arg){

	if(count<3){
		cout<<"Usage :"<<arg[0]<<" <port_num> <timer_value>"<<endl;
		exit(0);
	}
	else{
		cout<<"Welcome to web proxy server"<<endl;
		port=atoi(arg[1]);
	}

} 

int proxy_create_socket(){
	int sfd;
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
	return sfd;	
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
