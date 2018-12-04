#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>


#define MAXLINE 4096 /*max text line length*/

char host_name[MAXLINE];
char temp[MAXLINE];
    
int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in servaddr;
	char sendline[MAXLINE], recvline[MAXLINE];
	alarm(120); // to terminate after 300 seconds

	//basic check of the arguments
	//additional checks can be inserted
	if (argc != 4) {
		perror("Usage: TCPClient ");
		exit(1);
	}

	//Create a socket for the client
	//If sockfd<0 there was an error in the creation of the socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Problem in creating the socket");
		exit(2);
	}

	//Creation of the socket
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(atoi(argv[2]));


	gethostname(host_name, 254);

	FILE *ofp;
	char *outputFilename = argv[3];
	ofp = fopen(outputFilename, "w");
	if (ofp == NULL) {
		fprintf(stderr, "Can't open output file %s!\n", outputFilename);
		exit(1);
	}


	//Connection of the client to the socket
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror("Problem in connecting to the server");
		exit(3);
	}

	strcpy(temp, "Client-logfile.txt\n");
	
    while (fgets(sendline, MAXLINE, stdin) != NULL) {

		if(sendline == "exit"){
			exit(4);
		}

		strcat(temp, sendline);
        
		
		send(sockfd, sendline, strlen(sendline), 0);

		if (recv(sockfd, recvline, MAXLINE, 0) == 0) {
			//error: server terminated prematurely
			perror("The server terminated prematurely");
			exit(5);
		}
		
		printf("%s", "-> ");
		fputs(recvline, stdout);
        //strcat(temp, sendline);
		strcat(temp, host_name);
		strcat(temp, "\n---------------------\n\n");
        //puts(temp);
		
	}
    fprintf(ofp, "%s\n", temp);
    fclose(ofp);
	exit(0);
}