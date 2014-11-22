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

/**
 * Keeps state of each connection
 */
class Client
{
	public:
		Client(int myFd, fd_set *myFdList);
		~Client();
		//void Send();
		void read();
	protected:
		int fd; // Filedescriptor for socet
		int state;
		fd_set *connectionListPntr; // Pointer to main list of filedescriptors
};

Client::Client(int myFd, fd_set *myFdList)
{
	fd = myFd;
	connectionListPntr = myFdList;
}
Client::~Client()
{
	// TODO Cleanup
}
void Client::read()
{
	// Check for info to read
	if(FD_ISSET(fd, connectionListPntr))
	{
		char buf[256]; // Buffer for client data TODO: Diff on state
		// Read from socket
		int readBytes = recv(fd, buf, sizeof buf, 0);
		if(readBytes <= 0)
		{
			// Got error or connection closed by client
			if(readBytes == 0)
			{
				// Connection closed
				printf("Client %d disconnected\n", fd);
			}
			else
			{
				perror("recv");
			}
			close(fd); // Disconnect
			FD_CLR(fd, connectionListPntr); // Remove from main connection list
		}
		else
		{
			// We got some data from a client
			printf("%s\n", buf);
			// send to everyone!
			/*
			for(j = 0; j <= fdmax; j++)
			{
				if(FD_ISSET(j, connectionListPntr))
				{
					// except the sender
					if(j != serverSocket && fd != j)
					{
						if(send(j, buf, readBytes, 0) == -1)
						{
							perror("send");
						}
					}
				}
			}
			*/
		}
	}
}









// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	fd_set connectionList; // List of file descriptors for sockets
	int fdmax; // Maximum file descriptor number

	int serverSocket; // Listening socket descriptor
	int newfd; // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	socklen_t addrlen;

	char remoteIP[INET6_ADDRSTRLEN];

	int yes = 1; // for setsockopt() SO_REUSEADDR, below
	int j, rv;

	struct addrinfo hints, *ai, *p;

	FD_ZERO(&connectionList); // clear the master and temp sets

	// Create socket
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
		serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (serverSocket < 0)
		{
			continue;
		}

		// lose the pesky "address already in use" error message
		setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if(bind(serverSocket, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(serverSocket);
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
	if(listen(serverSocket, 10) == -1)
	{
		perror("listen");
		exit(3);
	}

	// add the serverSocket to the master set
	FD_SET(serverSocket, &connectionList);

	// keep track of the biggest file descriptor
	fdmax = serverSocket; // so far, it's this one

	// Create list of clients
	Client *clients[10];
	int numClients = 0;

	// main loop
	while(1)
	{
		// Listen on all connections untill something happens on them
		if(select(fdmax+1, &connectionList, NULL, NULL, NULL) == -1)
		{
			// Error with select
			perror("select");
			exit(4);
		}

		// Check for new connection
		if(FD_ISSET(serverSocket, &connectionList))
		{
			// Get filedescriptor of new connection
			addrlen = sizeof remoteaddr;
			newfd = accept(serverSocket, (struct sockaddr *)&remoteaddr, &addrlen);

			if(newfd == -1)
			{
				// Error with accept
				perror("accept");
			}
			else
			{
				// Add to connection list
				FD_SET(newfd, &connectionList);
				// Keep track of the max fd id
				if(newfd > fdmax)
					fdmax = newfd;
				// Print connection info to console
				printf("New connection from %s on socket %d\n", inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), newfd);
				// Create new Client
				Client newClient(newfd, &connectionList);
				// Add client to client list
				clients[numClients] = &newClient;
				numClients++;
			}
		}

		// Run through the existing connections looking for data to read
		for(int i = 0; i < numClients; i++)
		{
			if(clients[i] != NULL)
			{
				clients[i]->read();
			}
		}
	}

	return 0;
}
