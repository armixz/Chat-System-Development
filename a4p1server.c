#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>


/* Added: A lock to access numConnections.
   Note that this file needs to be compiled
   with -DMUTEX for the lock to actually be used */
#ifdef MUTEX
pthread_mutex_t lock;
#endif

char host_name[254];
FILE *ofps;

struct serverParm {
           int connectionDesc;
};

void *serverThread(void *parmPtr) {
    #define PARMPTR ((struct serverParm *) parmPtr)
    int recievedMsgLen;
    //template to store all the messages from all the clients.
    char temp[1025];
    char messageBuf[1025];
    /* Server thread code to deal with message processing */
    printf("DEBUG: connection made, connectionDesc=%d\n",
            PARMPTR->connectionDesc);
    if (PARMPTR->connectionDesc < 0) {
        printf("Accept failed\n");
        return(0);    /* Exit thread */
    }
    
    //get host name from client and store to char type
    //to use the value in strcat();
    gethostname(host_name, 254);

    //template = empty
    strcpy(temp, " ");

    /* ADDED: Protect access to numConnections with the lock */
    #ifdef MUTEX
    pthread_mutex_lock (&lock);
    #endif

    /* Receive messages from sender... */
    while ((recievedMsgLen=
            read(PARMPTR->connectionDesc,messageBuf,sizeof(messageBuf)-1)) > 0)
    {
        //open FILE as text file to store data. and w for writing on it.
        ofps = fopen("server-logfile.txt", "w");
    
        //if ERROR!
        if (ofps == NULL) {
            fprintf(stderr, "Can't open output file %s!\n", "logfile.txt");
            exit(1);
        }
        //because of messageBuf that is emptying for every
        //single loop, we stored data for every loop in template.
        strcat(temp, messageBuf);
        strcat(temp, host_name);
        strcat(temp, "\n------------------------\n\n");
        //puts(temp);

        //write strings into text file.
        fprintf(ofps, "%s\n", temp);
        
        //empty the messageBuf
        recievedMsgLen[messageBuf] = '\0';

        //print messages
        printf("\nMessage: %s",messageBuf);
        
        //print hostname and date for server-side console.
        system("hostname; date\n");
        
        if (write(PARMPTR->connectionDesc,"\0\n",7) < 0) {
            perror("Server: write error");
            return(0);
        }
    }
  

    close(PARMPTR->connectionDesc);  /* Avoid descriptor leaks */
    free(PARMPTR);                   /* And memory leaks */
    fclose(ofps);                    /* close FILE */
    
    /* ADDED: Unlock the lock when we're done with it. */
    #ifdef MUTEX
    pthread_mutex_unlock (&lock);
    #endif
    
    return(0);                       /* Exit thread */
}

int main (int argc, char *argv[]) {

    int listenDesc;
    struct sockaddr_in myAddr;
    struct serverParm *parmPtr;
    int connectionDesc;
    pthread_t threadID;
    
    /* For testing purposes, make sure process will terminate eventually */
    alarm(120); /* Terminate in 120 seconds */
    /* Create socket from which to read */
    if ((listenDesc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("open error on socket");
        exit(1);
    }
    /* Create "name" of socket */
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = INADDR_ANY;
    myAddr.sin_port = htons(atoi(argv[1]));
     
    if (bind(listenDesc, (struct sockaddr *) &myAddr, sizeof(myAddr)) < 0) {
        perror("bind error");
        exit(1);
    }
    /* Start accepting connections.... */
    /* Up to 5 requests for connections can be queued... */
    listen(listenDesc,5);
    while (1) /* Do forever */ {
        /* Wait for a client connection */
        connectionDesc = accept(listenDesc, NULL, NULL);
        /* Create a thread to actually handle this client */
        parmPtr = (struct serverParm *)malloc(sizeof(struct serverParm));
        parmPtr->connectionDesc = connectionDesc;

        /* ADDED: Initialize the lock */
        #ifdef MUTEX
        pthread_mutex_init(&lock, NULL);
        #endif
        
        if (pthread_create(&threadID, NULL, serverThread, (void *)parmPtr)
              != 0) {
            perror("Thread create error");
            close(connectionDesc);
            close(listenDesc);
            exit(1);
        }

        /* ADDED: Destroy the lock */
        #ifdef MUTEX
        pthread_mutex_destroy(&lock);
        #endif
        
        printf("Parent ready for another connection\n");
    }
}