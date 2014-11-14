/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold


#define MAXDATASIZE 100 // max number of bytes we can get at once


// Function to clear old forks
void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	/*
	   int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	   struct addrinfo hints, *servinfo, *p;
	   struct sockaddr_storage their_addr; // connector's address information
	   socklen_t sin_size;
	   struct sigaction sa;
	   int yes=1;
	   int rv;

	/***                              Setup server                     ***/
	/*
	   memset(&hints, 0, sizeof hints);
	   hints.ai_family = AF_UNSPEC;
	   hints.ai_socktype = SOCK_STREAM;
	   hints.ai_flags = AI_PASSIVE; // use my IP

	   rv = getaddrinfo(NULL, PORT, &hints, &servinfo);
	   if(rv != 0)
	   {
	   fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	   return 1;
	   }

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
	sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol);
	if(sockfd  == -1)
	{
	perror("server: socket");
	continue;
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
	perror("setsockopt");
	exit(1);
	}

	if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
	{
	close(sockfd);
	perror("server: bind");
	continue;
	}

	break;
	}

	if(p == NULL)
	{
	fprintf(stderr, "server: failed to bind\n");
	return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if(listen(sockfd, BACKLOG) == -1)
	{
	perror("listen");
	exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
	{
	perror("sigaction");
	exit(1);
	}


	// Connected
	printf("server: waiting for connections...\n");

	// Holds ip of connected client
	char remoteIP[INET6_ADDRSTRLEN];


	// Main accept() loop
	while(1)
	{
		// Wait for connection
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if(new_fd == -1)
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), remoteIP, sizeof remoteIP);
		printf("server: got connection from %s\n", remoteIP);

		if(!fork())
		{
			// child doesn't need the listener
			close(sockfd);

			// this is the child process
			char responseData[MAXDATASIZE];
			int numbytes;
			int doClose = 0;
			char data[MAXDATASIZE-1];

			while(1)
			{
				// Recieve
				numbytes = recv(new_fd, responseData, MAXDATASIZE-1, 0);
				if(numbytes == 0)
				{
					// Connection closed
					printf("Connection closed: %s\n", remoteIP);
					doClose = 1;
				}
				else if(numbytes == -1)
				{
					// Sum thing wrong
					perror("recv");
					doClose = 1;
				}
				else
				{
					// End string proper	
					responseData[numbytes] = '\0';
					// Print recieved
					printf("server: received '%s' from %s\n", responseData, remoteIP);

					// Create respond data
					if(strcmp(responseData, "test") == 0)
					{
						strcpy(data, "Test command is OK");
					}
					else
						strcpy(data, "Unknown command");

					// Send
					if(send(new_fd, data, sizeof data, 0) == -1)
					{
						perror("send");
						doClose = 1;
					}
				}

				// Close connection on request
				if(doClose)
				{
					close(new_fd);
					exit(0);
				}
			}
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
	*/



	fd_set master_fds;    // master file descriptor list
	fd_set read_fds;  // temp file descriptor list for select()
	int fdmax;        // maximum file descriptor number

	int listener;     // listening socket descriptor
	int newfd;        // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	socklen_t addrlen;

	char buf[256];    // buffer for client data
	int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

	int yes = 1;        // for setsockopt() SO_REUSEADDR, below
	int i, j, rv;

	struct addrinfo hints, *ai, *p;

	FD_ZERO(&master_fds);    // clear the master and temp sets
	FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
	{
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	for(p = ai; p != NULL; p = p->ai_next)
	{
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0)
		{
			continue;
		}

		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if(bind(listener, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(listener);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if(p == NULL)
	{
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

	// listen
	if(listen(listener, 10) == -1)
	{
		perror("listen");
		exit(3);
	}

	// add the listener to the master set
	FD_SET(listener, &master_fds);

	// keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one

	// main loop
	while(1)
	{
		read_fds = master_fds; // copy it

		// Listen on all connections untill something happens on them
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
			// Error with select
			perror("select");
			exit(4);
		}

		// Something changed on one of the connections, handle it

		// run through the existing connections looking for data to read
		for(i = 0; i <= fdmax; i++)
		{
			if(FD_ISSET(i, &read_fds))
			{
				// This is one connection that changed
				if(i == listener)
				{
					// handle new connections
					addrlen = sizeof remoteaddr;
					newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

					if(newfd == -1)
					{
						perror("accept");
					}
					else
					{
						FD_SET(newfd, &master_fds); // add to master set
						if(newfd > fdmax)
						{
							// keep track of the max
							fdmax = newfd;
						}
						printf("selectserver: new connection from %s on socket %d\n", inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), newfd);
					}
				}
				else
				{
					// New event on a connection
					if((nbytes = recv(i, buf, sizeof buf, 0)) <= 0)
					{
						// got error or connection closed by client
						if(nbytes == 0)
						{
							// connection closed
							printf("selectserver: socket %d hung up\n", i);
						}
						else
						{
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master_fds); // remove from master set
					}
					else
					{
						// we got some data from a client
						for(j = 0; j <= fdmax; j++)
						{
							// send to everyone!
							if(FD_ISSET(j, &master_fds))
							{
								// except the sender
								if(j != listener)
								{
									if(send(j, buf, nbytes, 0) == -1)
									{
										perror("send");
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return 0;
}
