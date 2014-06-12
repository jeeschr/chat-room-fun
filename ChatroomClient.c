/* Final Project: ChatroomClient.c
 * Members:
 * 	Jennifer Schror
 * 	Sivaramharesh Siva
 * 	Krisha Paula Olanday
 */

#include <stdio.h>  
#include <stdlib.h>        
#include <string.h>     
#include <unistd.h>      
#include <sys/types.h>            
#include <sys/socket.h>          
#include <netinet/in.h>          
#include <pthread.h>          
#include <signal.h>          

#define PORTNUM 2012    /* default port number */  
#define MAXDATALEN 256


/*====== Global Variables ======*/
int	sockfd; 		// socket file descriptor 
int 	n; 			// number of bytes received/sent
struct 	sockaddr_in serv_addr;	// server address  structure 
char   	buffer[MAXDATALEN];  
char   	username[10];       


/*====== Functions ======*/
void *quitproc();  
void *chatwrite(int);  
void *chatread(int);  
void *ctrlzhandler();  
void writelogfile(char str[256]);


FILE *file; // log file

int main(int argc, char *argv[]){  

	char fname[100];
	int rc;
	time_t temp;
	struct tm *timeptr;
	
	pthread_t thr1,thr2;          // two threads, one to read socket, one to write socket
	if( argc != 2 ){       
		printf("Specify Server IP Address\n");  
		exit(0);  
	}  
	// 1. Socket  
	sockfd = socket(AF_INET, SOCK_STREAM, 0);  
	if (sockfd == -1)  
	printf ("client socket error\n");  
	else{       
		//printf("Socket Created\n");  
	}
	
	// 2. Server Information
	bzero((char *) &serv_addr, sizeof(serv_addr));  
	serv_addr.sin_family = AF_INET;  
	serv_addr.sin_port = htons(PORTNUM);  
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);  
	
	// Get the Client Username
	bzero(username,10);  
	printf("\nUserName: ");  
	fgets(username,10,stdin);  
	__fpurge(stdin);       
	username[strlen(username)-1]=':';  
	
	// Set up the Conversation History File name with DateTime And User Name
	temp = time(NULL);
	timeptr = localtime(&temp);
	rc = strftime(fname,sizeof(fname),"%b_%d_%Y_Log_File", timeptr);
	//printf("%d characters in Date Time String \n%s\n",rc,fname);
	
	fname[rc]='-';
	int i = 0;
	while(i < strlen(username)){
		fname[rc+i+1]=username[i];
		i++;
	}
	fname[rc+i+1]='\0';
	
	file = fopen(fname,"a+"); // append file, add text to a file or create a file if it does not exist.
	
	// 3. Connect
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))==-1)  
	{  
		printf("client connect error\n");  
		exit(0);  
	}  
	else { 
		printf("\n To send a private message use the format: '@user message'\n"); 
		printf("%s Connected To Server\n",username);  
		printf("\r You Joined Chat as %s",buffer-1);
		 
		send(sockfd,username,strlen(username),0);  


		pthread_create(&thr2,NULL,(void *)chatwrite,(void *)sockfd);	//thread to write
		pthread_create(&thr1,NULL,(void *)chatread,(void *)sockfd);     //thread to read
		pthread_join(thr2,NULL);  
		pthread_join(thr1,NULL);  
		fclose(file);
	}
	
	return 0;  
} /* end of Main */


/* chatwrite - writes to socket
 * for PM look for @ and update the message look
 */
void* chatwrite(int sockfd)  
{  
	char fname[100];
	int rc;
	time_t temp;
	struct tm *timeptr;

	// Write to Socket
	while(1)  
	{  
		printf("%s",username);  

		fflush(file);
		fgets(buffer,MAXDATALEN-1,stdin);  
		if(strlen(buffer)-1>sizeof(buffer)){  
			printf("buffer size full\t enter within %zu characters\n",sizeof(buffer));  
			bzero(buffer,MAXDATALEN);  
			__fpurge(stdin);  
		}  
		fprintf(file,"%s ",username); 	//writes UserName   conv history file

		writelogfile(buffer); 		// write entry into log file
		
		n = send(sockfd,buffer,strlen(buffer),0);  
		if(strncmp(buffer,"quit",4)==0)  
			exit(0);

		bzero(buffer,MAXDATALEN);  
	} /* end of while */
	return NULL;
}  /* end chatwrite */


/* chatread - reads message from the socket 
 */
void* chatread(int sockfd)  
{  
	// Read from Socket
	
	// Signal handler
	if(signal(SIGTSTP, (void *)ctrlzhandler)==0)  
	
	while(1)  
	{  
		n=recv(sockfd,buffer,MAXDATALEN-1,0);  
		if(n==0){   
			// no bytes recieved
			printf("\nServer is Down\n\n");  
			exit(0);  
		}  
		if(n>0){  
			printf("\n%s ",buffer);  
			
			writelogfile(buffer); // write entry to log file
			
			fflush(file);
			bzero(buffer,MAXDATALEN);  
		}  
	}//end of while

	return NULL;
}  /* end chatread */

void *ctrlzhandler(){             
	// Signal handler for ctrl+z
	printf("\rType 'quit' to exit\n");

	return NULL;
}

void writelogfile(char str[256]){
	char fname[100];
	int rc;
	time_t temp;
	struct tm *timeptr;
	
	fprintf(file,"%s",str); // add received text into the log file
	
	// add the current date and time
	temp = time(NULL);
	timeptr = localtime(&temp);
	rc = strftime(fname,sizeof(fname),"%a,%b %d %r", timeptr);
	fprintf(file,"%s\n",fname);
}  
