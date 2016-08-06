#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) {
	perror(msg);
	exit(0);
}

bool readAllData(int fd) {
	char buffer[256];
	bzero(buffer, 256);
	int n = read(fd, buffer, sizeof(buffer));
	if(n==0) return false;
	printf("%s\n",buffer);
	return true;
}

int main(int argc, char *argv[]) {
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];
	portno = 3456;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname("54.174.215.133");
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr,
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR connecting");
	}
	n = write(sockfd, "MONA", 4);
	usleep(100);
	readAllData(sockfd);

	for(int i=0;i<6;i++) {
		bzero(buffer, 256);
		fgets(buffer, 255, stdin);
		n = write(sockfd, buffer, strlen(buffer));
		if(n<=0) break;
		usleep(1000);
		if(!readAllData(sockfd)) break;
	}

	close(sockfd);
	return 0;
}

