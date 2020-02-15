//Server side scripting
/**********************************************************
*	 Program for Napster Server			  *
*	Functionality: Join, Publish, Search		  *
**********************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <signal.h>
#define PORT 7021
int identifier = 1;
int main()
{
	FILE *fp, *fp1;	//file descriptor for peer list and file list
	int socfd;	//socket file descriptor
	int confd;	//connecting file descriptor
	int slen, clen;	//length of client and server object
	int pid;	//for process id
	struct sockaddr_in ser, cli;	//object declaration of socket family 
	char ip[25];
	char buf[1024];	//buffer
	time_t t;	//to store time

	//remove previous versions of file for storing peer list and file list
	remove("file_list.txt");
	remove("peer_list.txt");

	socfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socfd < 0)
	{
		printf("File not created.\n");
		return 0;
	}

	//initailizing the server detail
	ser.sin_family = AF_INET;	//family type
	//random ip and port number to server
	srand((unsigned) time(&t));
	ser.sin_port = htons(4000 + rand() % 100);	//converting port address to network address for storing purpose
	ser.sin_addr.s_addr = INADDR_ANY;
	slen = sizeof(ser);
	clen = sizeof(cli);
	
	if(bind(socfd, (struct sockaddr*)&ser, slen) < 0)
	{
		printf("Binding error\n");
		return 0;
	}

	printf("The server has port & IP address: %hd %s\n", ntohs(ser.sin_port),inet_ntoa(ser.sin_addr));

	listen(socfd, 10);		//to listen the request of 5 clients
	signal(SIGCHLD, SIG_IGN);
	while(1){
		confd = accept(socfd, (struct sockaddr*)&cli, &clen);		//respond to the request made by client
		printf("Connection established\n");
		if(confd < 0)
		{
			printf("Client connection failed.\n");
			return -1;
		}
		 printf("The Client has port & IP address: %hd %s\n", ntohs(cli.sin_port),inet_ntoa(cli.sin_addr));
		//join operation
		//write peer ip and port to file
		fp = fopen("peer_list.txt", "a");
		if(!fp)
		{
			printf("File couldn't be read\n");
			return 1;
		}
		
		recv(confd, &ip, 25, 0);
		fprintf(fp,"%d\t%hd\t%s\n", identifier, ntohs(cli.sin_port), ip);
		identifier++;
		fclose(fp);
		printf("One client Joined the network.\n");
		
		//Publish operation
		//write file names from a client to file
		int code;
		recv(confd, &code , sizeof(int), 0);		//read message from client
		fp1 = fopen("file_list.txt", "a");
		if(!fp1)
		{
			printf("1File couldn't be read\n");
			return 1;
		}
		
		while(code == 1)
		{

			char fname[31];
			char ip[25];
			int port;
			bzero(fname, 30);
			recv(confd, fname, 30, 0);
			bzero(ip, 25);
			recv(confd, ip, 25, 0);
//			printf("%s\t%d\t%s\n", fname, port, ip);
			fprintf(fp,"%hd\t%s\t%s\n", ntohs(cli.sin_port), ip, fname);
			recv(confd, &code, sizeof(int), 0);	
		}
		fclose(fp1);
		printf("Publishing of File by Client done..\n");
	
		pid = fork();
		if(pid == 0)
		{
			//search operation
			recv(confd, &code , sizeof(int), 0);
			while(1){
			if(code == 2)
			{
				printf("Search operation\n");
				char file_name[30], f_name[30];
				char ip[25], result = 1;
				int port, id, status;
				recv(confd, file_name, 30, 0);

				printf("For %s\n", file_name);
				fp1 = fopen("file_list.txt", "r+");
				if(!fp1)
				{
					printf("1File couldn't be read\n");
					return 1;
				}
				//check for the file in the list of files
				while(!feof(fp1))
				{
					bzero(f_name, 30);
					fscanf(fp1,"%d %s %s",&port, ip, f_name);
					if((result=strcmp(f_name, file_name)) == 0)
					{
						//If file name matches
						status = 1;
						send(confd, &status, sizeof(int), 0);
						printf("File Found:%d\n", status);
						send(confd, &port, sizeof(int), 0);
						send(confd, ip, 25, 0);
						break;
					}
				}
				close(fp1);
				if(result)
				{
					//If file name is not found
					status = 0;
					printf("File not Found:%d\n", status);
					send(confd, &status, sizeof(int), 0);
				}
			}
			recv(confd, &code , sizeof(int), 0);		
			}
		}
		else
		{

			close(confd);
		}
	}
	close(socfd);
	return 0;
}

