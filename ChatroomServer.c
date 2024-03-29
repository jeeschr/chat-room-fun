/* Final Project: ChatroomServer.c
 * Members:
 * 	Jennifer Schror
 * 	Sivaramharesh Siva
 * 	Krisha Paula Olanday
 */
/****SERVER CODE****/  

#include <stdio.h>            
#include <stdlib.h>            
#include <sys/socket.h>	 
#include <sys/types.h>            
#include <string.h>      
#include <netinet/in.h> 
#include <pthread.h>     
#include <arpa/inet.h>   
#include <unistd.h>      
#include <signal.h>     

#define BACKLOG 100      /* connections in the queue */  
#define MAXDATALEN 256   /* max size of messages to be sent */ 
#define PORT 2012        /* default port number */
 
/* Note:   The port argument is optional. If no port is specified,  
 *         the server uses the default given by PORT.*/  
   
/* Structure to handle all clients
 */ 
struct client{  
  int port;             /* holds client's port num */
  char username[10];    /* client username */  
  struct client *next;  /* pointer to the next client */
};

typedef struct client *clients;  
typedef clients head;             // pointer to the first client addr 
typedef clients addr;             // pointer to the client addr to traverse through the linked list

/* FUNTION HEADERS
 */
void SendToAll(char *, int connfd);                                   /* WE DONT USE THIS send chat msgs to all connected clients*/  
void SendPrivateMessage (char *msg, char *sender, char *receiver);    /* sends a private message */
void NotifyServerShutdown( );                                         /* send msg to all if server shuts down */  
head MakeEmpty(head headptr);                                         /* clearing client list */  
void RemoveClient( int port, head headptr );                          /* delete client values on client exit */  
void InsertClient(int port,char*,head headptr, addr addr_ptr);        /* inserting new client */  
void DeleteList( head headptr);                                       /* clearing client list */  
void DisplayList( const head headptr);                                /* list all clients connected */  
void *CloseServer( );                                                 /* signal handler */  
void *connClientToServer(void * arg);                                 /* server instance for every connected client */  
void sigBlocktoDisplay();  

/* GLOBAL VARIABLES
 */
int sf2;  
head head_ptr;                /* variable of type struct head */
char      username[10];       /* size of username */  
char     buffer[MAXDATALEN];  

/******main starts ***********/  
int main(int argc, char *argv[]) {  
      int                  listenfd, connfd;  /* variables for socket */  
      int                  portnum;           /* variable for port numb if provided*/  
      struct sockaddr_in   server_addr;       /*structure to hold server's address */  
      struct sockaddr_in   client_addr;       /*structure to hold client's address */  
      socklen_t            cli_size;          /*length of address */  
      pthread_t            thr;               /*variable to hold thread ID */  
      int                  yes = 1;  
      addr addr_ptr;                          /*variable of type struct addr*/  
      
      printf("\n\t*-*-*-*SERVER STARTED*-*-*-*\n");  
      
  /* optional or default port argument */     
  if( argc == 2 )       
      portnum = atoi(argv[1]);  
  
  else   
      portnum = PORT; /*if port number not given as argument then using default port */ 
      printf("PORT NO.:\t%d\n",portnum);  
      head_ptr = MakeEmpty(NULL);              		  /* frees the list */
	printf("Does it get here\n");
      
      /* set info of server  */  
      server_addr.sin_family=AF_INET;                     /* set family to Internet   */  
      server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    /* set IP address */  
      server_addr.sin_port=htons(portnum);  
       printf("IP ADDRESS:\t%s\n",inet_ntoa(server_addr.sin_addr));  
      
      /* creating socket */  
    listenfd = socket(AF_INET, SOCK_STREAM, 0);  
  if(listenfd == -1){  
      printf("server- socket() error");	/* debugging */  
      exit(1);  
  } else {  
      printf("socket\t\tcreated.\n");
  }

  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {  
      printf("setsockopt error");	/* debugging */  
      exit(1);  
  } else {
      printf("reusing\t\tport\n");
  }  
  
  /*= binding socket =*/  
  if(bind(listenfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))==-1){  
      printf("binding failed\n");	/* debugging */
      exit(1);  
  } else {   
      printf("binding\t\tsuccess.\n\n");
  }

  printf("\t\tPRESS CTRL+Z TO VIEW ONLINE CLIENTS\n\n");  
    
  /*= socket on listening mode =*/  
  listen(listenfd, BACKLOG);  
  printf("waiting for clients......\n");  
  if (signal(SIGINT,(void *)CloseServer)==0)	/* signal handler */  
  if(signal(SIGTSTP, sigBlocktoDisplay)==0)	/* signal handler */ 
  
  /* loop for multi client connection */
  while(1){  
      cli_size = sizeof(struct sockaddr_in);                                  /* cli_size necessary as an argument for pthread_create */
      connfd = accept(listenfd, (struct sockaddr *)&client_addr, &cli_size);   /* accepting connection from client */   
      addr_ptr = head_ptr;  
      
      /*= SIGN IN WITH NAME =*/  
      bzero(username,10);            
    
    if(recv(connfd, username, sizeof(username),0) >0);  
        username[strlen(username) - 1] = ':';

        printf("\t%d->%s JOINED chatroom\n", connfd,username);  
        sprintf(buffer,"%s IS ONLINE\n",username);  
        
        InsertClient(connfd, username, head_ptr, addr_ptr );               /* inserting newly accepted client socked fd in list */  
        addr_ptr = addr_ptr->next;  
        /*= notify all clients about newly joining clients =*/   
        addr_ptr = head_ptr ;  
          
        do {  
            addr_ptr = addr_ptr->next;  
            sf2 = addr_ptr->port;  
            
            if( sf2 != connfd){  
                send(sf2,buffer ,sizeof(buffer),0);
            }

        } while( addr_ptr->next != NULL );

        printf("ALERT: server got connection from %s & %d\n\n",inet_ntoa(client_addr.sin_addr),connfd); /* debugging */ 
            
        struct client args;						/* struct to pass multiple arguments to server function */  
        args.port = connfd;  
        strncpy(args.username, username, 10);  
        pthread_create(&thr,NULL,connClientToServer,(void*)&args);	/* creating thread for every client connected */
        pthread_detach(thr);  
  
  } /*while end*/  
       
  DeleteList(head_ptr);       /* deleting all clients when server closes */
  close(listenfd);

  return 0;

} /* END OF MAIN */  


/*******ALL FUNCTIONS DEFINED BELOW*********/  

/* ==========Server function for every connected Client =========*/  
void *connClientToServer(void *arguments){  
    struct client *args = arguments;  
    char   buffer[MAXDATALEN], ubuf[50], uname[10];   /* buffer for string the server sends */   
    char   *strp;            
    char   *msg = (char *) malloc(MAXDATALEN);  
    int    ts_fd,x, bufflen;  
    int    sfd,msglen;  
    ts_fd  = args->port;	/*socket variable passed as arg*/
    
    char *recvrname;
    char *buff;
    
    strcpy(uname, args->username);   
    addr   addr_ptr;  
    
    /*=sending list of clients online=*/  
    addr_ptr = head_ptr ;  
    
    do{  
        addr_ptr = addr_ptr->next;  
        sprintf( ubuf," %s is online\n",addr_ptr->username );  
        send(ts_fd,ubuf,strlen(ubuf),0);  
    } while( addr_ptr->next != NULL );  
    
    /*=start chatting=*/  
    while(1){  
        bzero(buffer,256);  
        bufflen = recv(ts_fd,buffer,MAXDATALEN,0);  
       
        if (bufflen == 0)   
           goto jmp;  
        
        /* parse client message */
        printf("Server Received: %s\n", buffer);

        if (buffer[0] == '@'){
            printf("SENDING PRIVATE MESSAGE.\n");
            char *spacelocation;
            /* parsing the username of the receiver */
            buff = (char*)malloc(bufflen);
            strcpy(buff, buffer+1);

            int namelen, i = 0;
            recvrname = (char*)malloc(12);  
            spacelocation = strchr(buff, ' ');

            while ((buff + i) != spacelocation){
              strncat(recvrname, buff + i, 1);
              i++;
            }

            recvrname[i+1] = '\0';
            
            printf("Client %s will receive the pm from %s\n", recvrname, uname);
            namelen = strlen(recvrname);
            
            char* temp = (char*)malloc(strlen(buff));  /* message to send to the receiving client */
            strcpy(temp, buff+namelen);
            printf("The private message is: %s\n", temp);

            SendPrivateMessage(temp, uname, recvrname);
            free(temp);
            free(recvrname);

        } else { /* send message to all */
            printf("SENDING MESSAGE TO ALL.\n");
            /*=if a client quits=*/  
            if ( strncmp( buffer, "quit", 4) == 0 ){  
                jmp:    printf("%d ->%s left chat deleting from list\n",ts_fd,uname);  
                sprintf(buffer,"%s has left the chat\n",uname);  
                addr addr_ptr = head_ptr ;  
            
                do{  
                    addr_ptr = addr_ptr->next;  
                    sfd = addr_ptr->port;  
                    if(sfd == ts_fd)   
                     RemoveClient( sfd, head_ptr );  
                    if(sfd != ts_fd)   
                    send(sfd,buffer,MAXDATALEN,0);  
                } while ( addr_ptr->next != NULL );  
            
                DisplayList(head_ptr);  
                close(ts_fd);  
                free(msg);  
                break;  
            }  

	    /*=sending message to all clients =*/  
            printf("%s %s\n",uname,buffer);  
            strcpy(msg,uname);  
            x = strlen(msg);  
            strp = msg;  
            strp += x;  
            strcat(strp,buffer);  
            msglen = strlen(msg);  
            addr addr_ptr = head_ptr;  
      
      	    do{  
            		addr_ptr = addr_ptr->next;  
            		sfd = addr_ptr->port;   
          	   	if(sfd != ts_fd)   
                	 send(sfd,msg,msglen,0);  
            } while( addr_ptr->next != NULL );  
       

            DisplayList( head_ptr );  
            bzero(msg,MAXDATALEN);
        } /* end if */ 
    } /* end while */
    return 0;  

} /* end server */  


/*SendPrivateMessage - sends a message only to the specified client
 *Inputs:
 *  msg ->     the message that will be sent
 *  sender ->  address of the client that sent the message
 *  recvr ->   address of the client who will receive the message
 */
void SendPrivateMessage (char *msg, char *sender, char *receiver){
    addr  addr_ptr = head_ptr;
    char *usercheck = NULL;
    int   recvrport;
    char *buff = (char*)malloc(MAXDATALEN);
    buff[0] = '\0';
    strncat(receiver, ":", 1);

    /* FORMATTING MESSAGE
     * message format: [From username] Message from username. */
    strncat(buff, "[From ", 6);
    strncat(buff, sender, strlen(sender));
    strncat(buff, "] ", 2);
    strncat(buff, msg, strlen(msg));
    strncat(buff, "\0", 2);
    printf("Private message: %s\n", buff);

    /* OBTAINING RECEIVER'S PORT NUM */
    do {
        usercheck = addr_ptr->username;
        printf("Checking the usercheck: %s\n", usercheck);

        if(strcmp(usercheck, receiver) == 0){
          printf("FOUND IT!\n");
          break;
        } else {
          if (addr_ptr->next == NULL){
            break;
          }

          addr_ptr = addr_ptr->next;
        }

    } while(addr_ptr != NULL);

    recvrport = addr_ptr->port;

    send(recvrport, buff, strlen(buff), 0); /* sending the private message */

} /* end SendPrivateMessage */


 
/*=====empties and deletes the list======*/  
head MakeEmpty( head head_ptr ){  
    if( head_ptr != NULL ) 
          DeleteList(head_ptr);  
          head_ptr = malloc(sizeof(struct client));  
    
    if( head_ptr == NULL )
          printf( "Out of memory!" );  
          head_ptr->next = NULL;  

    return head_ptr;  
} /* end MakeEmpty */



/*======delete list=======*/  
void DeleteList( head headptr ){  
    addr addr_ptr, Tmp;  
    addr_ptr = head_ptr->next;   
    headptr->next = NULL;  
    
    while( addr_ptr != NULL ){  
        Tmp = addr_ptr->next;  
        free( addr_ptr );  
        addr_ptr = Tmp;  
    }  
} /* end DeleteList */  

/*===============inserting new clients to list==========*/  
void InsertClient( int port,char *username, head headptr, addr addr_ptr ){  
    addr TmpCell;  
    TmpCell = malloc( sizeof(struct client));  
           
    if( TmpCell == NULL )  
        printf( "Out of space!!!" );  
    
    TmpCell->port = port;  
    strcpy(TmpCell->username,username);  
    TmpCell->next = addr_ptr->next;  
    addr_ptr->next = TmpCell;  
} /* end Insert */

/*========displaying all clients in list==================*/  
void DisplayList( const head headptr ){  
    addr addr_ptr = headptr ;  
    
    if( headptr->next == NULL ) {
        printf( "NO ONLINE CLIENTS\n" );  
    
    } else { 
        do {  
            addr_ptr = addr_ptr->next;  
            printf( "%d->%s \t", addr_ptr->port,addr_ptr->username );  
        } while( addr_ptr->next != NULL );  
        
        printf( "\n" );  
    }  
} /* end Display */  

/*===========client deleted from list if client quits================*/  
void RemoveClient( int port, head headptr ){  
    addr addr_ptr, TmpCell;  
    addr_ptr = headptr;  
    
    while( addr_ptr->next != NULL && addr_ptr->next->port != port )  
       addr_ptr = addr_ptr->next;  
    
    if( addr_ptr->next != NULL ){             
        TmpCell = addr_ptr->next;  
        addr_ptr->next = TmpCell->next;   
        free( TmpCell );  
    }  
} /* Delete */  

/*======handling signals==========*/  
void *CloseServer(){        
    printf("\n\nSERVER SHUTDOWN\n");  
    NotifyServerShutdown( );  
    exit(0);  
}  

/*===============notifying server shutdown===========*/  
void NotifyServerShutdown(){  
    int sfd;  
    addr addr_ptr = head_ptr ;  
    int i = 0;  
    
    if( head_ptr->next == NULL ) {  
        printf( "......BYE.....\nno clients \n\n" );  
        exit(0);  
    
    } else {  
        do{  
            i++;  
            addr_ptr = addr_ptr->next;  
            sfd = addr_ptr->port;  
            send(sfd,"server down",13,0);  
        } while( addr_ptr->next != NULL );  
       

        printf("%d clients closed\n\n",i);         
    }  
} /* end NotifyServerShutdown */


void sigBlocktoDisplay(){  
    printf("\rDISPLAYING ONLINE CLIENTS\n\n");  
    DisplayList(head_ptr);  
}  

 
