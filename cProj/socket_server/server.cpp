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
#include <stdint.h>

#define PORT "3490"  // The port users will be connecting to

#define MESSAGESIZE 1024 // Max number of bytes per package
#define PACKAGESIZE MESSAGESIZE+sizeof(uint32_t) // Size of package


enum PackageTypes
{
	PACKAGE_ACC=1,
	INTERFACE_INFO=2
} PackageType;

struct Package_new
{
	uint32_t type;
	uint32_t parts;
	uint32_t length;
};
struct Package_part
{
	uint32_t type;
	char buf[MESSAGESIZE];
};
struct Package_acc
{
	uint32_t type;
};

union Package
{
	Package_new new_;
	Package_part part_;
	Package_acc acc_;
	uint32_t type;
	char buf[PACKAGESIZE];
};

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
		fd_set *connectionListPntr; // Pointer to main list of filedescriptors
		Package readBuffer;
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
		// Read from socket
		int readBytes = recv(fd, readBuffer.buf, sizeof readBuffer.buf, 0);
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
			printf("Typet %d\n", readBuffer.type);
			if(readBuffer.type == PACKAGE_ACC) // New
			{
				printf("New message of %d parts with %d size\n", readBuffer.new_.parts, readBuffer.new_.length);
				// Prepare for new message
				// Send acc
			}
			else if(readBuffer.type == 2) // Message part
			{
				printf("Message part %s\n", readBuffer.part_.buf);
				// Send acc
			}
			else if(readBuffer.type == 3) // Acc
			{
				// Send worked!
				// Send next message if any
			}

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


// SEND
// Check cwreceive (state == META && messages.length == 0) or append to messages, divided into multiple
// if idle send and set state to receiving_acc






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


	/** Connection stuff **/
	int serverSocket; // Listening socket descriptor
	struct sockaddr_storage remoteaddr; // Client address
	socklen_t addrlen;
	char remoteIP[INET6_ADDRSTRLEN];
	int yes = 1; // For setsockopt() SO_REUSEADDR, below
	struct addrinfo hints, *ai, *p;
	FD_ZERO(&connectionList); // Clear the master and temp sets

	// Get address information
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	int rv = getaddrinfo(NULL, PORT, &hints, &ai);
	if(rv != 0)
	{
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	// Bind socket
	for(p = ai; p != NULL; p = p->ai_next)
	{
		serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(serverSocket < 0)
		{
			continue;
		}

		// Lose the pesky "address already in use" error message
		setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if(bind(serverSocket, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(serverSocket);
			continue;
		}

		break;
	}
	if(p == NULL)
	{
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}
	freeaddrinfo(ai); // All done with this
	// Listen
	if(listen(serverSocket, 10) == -1)
	{
		perror("listen");
		exit(3);
	}


	// Add the serverSocket to the connection list
	FD_SET(serverSocket, &connectionList);

	// Keep track of the biggest file descriptor
	fdmax = serverSocket; // So far, it's this one

	// Create list of clients
	Client *clients[10];
	int numClients = 0;

	// Main listen loop
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
			int newfd = accept(serverSocket, (struct sockaddr *)&remoteaddr, &addrlen);

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
