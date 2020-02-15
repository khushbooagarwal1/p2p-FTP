//Client side scripting
/*******************************************************************
*	Program for Napster peer implementation.		   *
*	Functioanlity: Join, Publish, Search, Fetch		   *
*******************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#include <sys/time.h> 
#include <time.h>
int identifier;
struct sockaddr_in peer;
int join(int socfd, struct sockaddr_in cli, struct sockaddr_in ser, int slen);
int publish(int socfd, struct sockaddr_in cli, struct sockaddr_in ser, int slen,char* fname);
int search(int socfd, struct sockaddr_in ser, int slen, char* file_name);
int fetch(int socfd, struct sockaddr_in ser, int slen, char* file_name);

int main(int argc, char *argv[])
{
	int socfd;	//socket file descriptor
	int slen, clen;	//length of  object
	struct sockaddr_in ser, cli;	//object declaration of socket family 
	char buf[1024];	//buffer
	time_t t;	//to store time value
	slen = sizeof(ser);
	clen = sizeof(cli);
	socfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socfd < 0)
	{
		printf("File not created.\n");
		return 0;
	}
	
	//initailizing the server detail
	cli.sin_family = AF_INET;       //family type

	//assigning a random port and ip to client
	srand((unsigned) time(&t));
	cli.sin_port = htons(8000+rand()%100);    
	cli.sin_addr.s_addr = INADDR_ANY;
	if(bind(socfd, (struct sockaddr*)&cli, clen) < 0)
	{
		printf("Binding error\n");
		return 0;
	}

	//if ip address and port number is provided or not
	if(argc != 3)
	{
		printf("Invalid input\n");
		return 0;
	}

	//to store the server details
	ser.sin_family = AF_INET;
	ser.sin_addr.s_addr = inet_addr(argv[1]);	//inet_addr convers string into dotted decimal ip
	ser.sin_port = htons(atoi(argv[2]));
	//connect to server
	if(connect(socfd, (struct sockaddr*)&ser, slen) < 0)
	{
		printf("Connection failed.\n");
		return 0;
	}	
	char *ip = inet_ntoa(cli.sin_addr);	//ip address of client
	send(socfd, ip, 25, 0);	//send client's ip to server for join operation
	printf("Client Connected to Server. Join Successful.\n");
	//search for files for publishing
	DIR *d;
	struct dirent *dir;
	d = opendir("p2p-files");
	if(d)
  	{

		while((dir = readdir(d)) != NULL)
   		{
			//If its a file type
			if (dir->d_type == DT_REG)
  			{
				//publish the file name to server
				publish(socfd, cli, ser, slen, dir->d_name);
   			}
		}
		closedir(d);
    		printf("Publishing file Done.\n");
	}
	
	signal(SIGCHLD, SIG_IGN);
	while(1)
	{
	/*	int i, client_socket[30], max_sd;
		fd_set readfds;
		 for (i = 0; i < max_clients; i++) 
   		 {
        		client_socket[i] = 0;
    		}
		*/int stop = -1;
		send(socfd, &(stop), sizeof(int), 0);

		printf("Do you need any File? (Enter 1 if Yes / 0 if No):");
		fflush(stdin);
		int check;
		scanf("%d",&check); 
		if(check == 1)
		{
			char file_name[30];
			int found;
			scanf("%s", file_name);
			//Search for the file name
			found = search(socfd, ser, slen, file_name);
			//If file is found
			if(found != -1)
			{
				fetch(socfd, cli, clen, file_name);
			}
		}
		else
		{
			//if not file is needed
			send(socfd, &(stop), sizeof(int), 0);
		}
		
		struct sockaddr_in node;	//to store requesting client info
        	listen(socfd, 4);
        	if(fork() == 0)
        	{
			//start connection to serve the file transfer request
			printf("Open for connection..\n");
               		int confd = accept(socfd, (struct sockaddr*)&node, &slen);
                	char buf[1024], sendfile[30];
                	FILE *fd;
                	int send_status = -1;
			//recieve file naem name and open the file
                	recv(confd, sendfile, 30, 0);
                	fd = fopen(sendfile, "r");
			//send the file
                	while(fd)
                	{
                        	send_status = 1;
                        	bzero(buf, 1024);
                        	send(confd, &send_status, sizeof(int), 0);
                        	fread(buf, sizeof(char), 1024, fd);
                	}
			send_status = 0;
                	send(confd, &send_status, sizeof(int), 0);
			close(confd);
		
        	}

		//wait for few seconds to check for new file request
		clock_t new_time;
		clock_t cur_time = clock();
		do{
			new_time = clock();
		}while((new_time - cur_time)/1000 <= 20000); 

	}
	close(socfd);
	return 1;
}

int join(int socfd, struct sockaddr_in cli, struct sockaddr_in ser, int slen)
{
	printf("Join to Server\n");
	int port = ntohs(cli.sin_port);
	char *ip = inet_ntoa(cli.sin_addr);
	printf("%s",ip);
	int code = 0;
	send(socfd, &(code), sizeof(int), 0);	//writing message for server	
	send(socfd, &port, sizeof(int), 0);	//writing message for server
	send(socfd, ip, strlen(ip)+1, 0);	//writing message for server
	recv(socfd, &identifier, sizeof(int), 0);
	return 0;
}

//Publish the file list and IP address to server
int publish(int socfd, struct sockaddr_in cli, struct sockaddr_in ser, int slen,char* fname)
{
	int publish_id = 1;
	int port = ntohs(cli.sin_port);
	char *ip = inet_ntoa(cli.sin_addr);

	send(socfd, &(publish_id), sizeof(int), 0);
	send(socfd, fname, 30, 0);
	send(socfd, ip, 25, 0);	//writing message for server

}
//Search for a particular file in server's database
int search(int socfd, struct sockaddr_in ser, int slen, char* file_name)
{
	int search_id =2;
	int found;
	send(socfd, &(search_id), sizeof(int), 0);
	send(socfd, file_name, 30, 0);
	printf("Searching for file: %s\n", file_name);

	recv(socfd, &found, sizeof(int), 0);
	if(found == 1)
	{
		char ip[25];
		int port;
		recv(socfd, &port, sizeof(int), 0);
		recv(socfd, ip, 25, 0);
		//initialize the peer socket value with the ip and port recieved from server
		peer.sin_family = AF_INET;
		peer.sin_addr.s_addr = inet_addr(ip);	
		peer.sin_port = htons(port);
		
		return 0;
	}
	else
	{
		printf("Not found\n");
		return -1;
	}
}
int fetch(int socfd, struct sockaddr_in cli, int clen, char* file_name)
{
	time_t t;
	int peerfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in temp;
	temp.sin_family = AF_INET;       //family type

	//assigning a random port and ip to client
	srand((unsigned) time(&t));
	temp.sin_port = htons(1000+rand()%100);    
	temp.sin_addr.s_addr = INADDR_ANY;
	//bind socket descriptor to address for communication
	if(bind(peerfd, (struct sockaddr*)&temp, clen) < 0)
        {
                printf("Binding error\n");
                return 0;
	}
	int status;
	char buf[1024];
	FILE *fd;
	fd = fopen(file_name, "a");
	printf("Connect to peer with port & IP address: %hd %s\n", ntohs(peer.sin_port),inet_ntoa(peer.sin_addr));
	//connect to the peer address
	if(connect(peerfd, (struct sockaddr*)&peer, clen) < 0)
	{
		printf("Connection failed.\n");
		return 0;
	}	
	send(peerfd, file_name, 30, 0);
	recv(peerfd, &status, sizeof(int), 0);
	//recieve and write to file
	while(status == 1)
	{
		bzero(buf, 1024);
		recv(peerfd, buf, 1024, 0);
		fwrite(buf, sizeof(char), 1024, fd);
		recv(peerfd, &status, sizeof(int), 0);
	}
	fclose(fd);
	close(peerfd);
}

