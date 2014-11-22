/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

#define STDIN 0  // file descriptor for standard input

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	int nbytes;

	if (argc != 2) {
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("connected to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure


	fd_set read_fds;
	fd_set master_fds;
	FD_ZERO(&master_fds);

	// Listen on stdin
	FD_SET(STDIN, &master_fds);

	// Listen on socket
	FD_SET(sockfd, &master_fds);

	int fdmax = sockfd;

	while(1)
	{
		read_fds = master_fds; // copy it

		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
			// Error with select
			perror("select");
			exit(4);
		}

		// Something changed on the connection or stdin
		if(FD_ISSET(STDIN, &read_fds))
		{
			// Data from std in
			char data[MAXDATASIZE-1];
			//numbytes = recv(STDIN, data, MAXDATASIZE-1, 0);
			fgets(data, MAXDATASIZE-3, stdin);
			// remove newline, if present
			int stringLength = strlen(data)-1;
			if(data[stringLength] == '\n')
				data[stringLength] = '\0';

			// Move data to make room for start byte
			int i;
			int data2[MAXDATASIZE-1];
			for(i = MAXDATASIZE-1;  i > 1; i--)
			{
				data2[i] = (int)data[i-2];
			}

			data2[0] = 1;
			data2[1] = stringLength;
			data2[1] = 2147483647;

			// Send data
			if(send(sockfd, data2, sizeof data2, 0) == -1)
				perror("send");
		}
		else if(FD_ISSET(sockfd, &read_fds))
		{
			/*
			// Data from socket
			if((nbytes = recv(sockfd, buf, sizeof buf, 0)) <= 0)
			{
				if(nbytes == 0)
				{
					printf("selectserver: socket %d hung up\n", sockfd);
				}
				else
				{
					perror("recv");
				}
				close(sockfd); // bye!
				FD_CLR(sockfd, &master_fds); // remove from master set
			}
			else
			{
				// Data from socket
				printf("%s\n", buf);
			}
			*/
			numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0);
			if(numbytes == 0)
			{
				// Connection closed
				printf("Connection closed!\n");
				exit(1);
			}
			if(numbytes == -1)
			{
				// Sum thing wrong
				perror("recv");
				exit(1);
			}

			buf[numbytes] = '\0';

			printf("%s\n",buf);
		}
	}
/*

	while(1)
	{
		char data[MAXDATASIZE-1];
		// Get line from std in
		/*
		   fgets(data, MAXDATASIZE-1, stdin);

		// remove newline, if present
		int i = strlen(data)-1;
		if(data[i] == '\n') 
		data[i] = '\0';

		// Send data
		if(send(sockfd, data, sizeof data, 0) == -1)
		perror("send"); 
		 */
/*
		numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0);
		if(numbytes == 0)
		{
			// Connection closed
			printf("Connection closed!\n");
			exit(1);
		}
		if(numbytes == -1)
		{
			// Sum thing wrong
			perror("recv");
			exit(1);
		}

		buf[numbytes] = '\0';

		printf("%s\n",buf);
	}

	close(sockfd);
	*/

	return 0;
}
