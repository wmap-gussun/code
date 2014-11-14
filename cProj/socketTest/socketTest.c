
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

int main()
{
	printf("hello\n");
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		printf("No socket available");
		return 1;
	}
	printf("Sockfd: %d\n", sockfd);

	int yes = 1;
	int sockopt = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if(sockopt == -1)
	{
		printf("Socket unavailable");
	}

	struct sockaddr_in my_addr;
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(6000);
	my_addr.sin_addr.s_addr = inet_addr("192.168.2.101");
	if(bind(sfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in)) == -1)
	{
		printf("Bind failed");
	}


	return 0;
};
