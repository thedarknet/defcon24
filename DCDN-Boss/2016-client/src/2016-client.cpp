//============================================================================
// Name        : 2016-client.cpp
// Author      : CmdC0de
// Version     :
// Copyright   : DCDarkNet Industries LLC  all right reserved
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <fcntl.h> /* Added for the nonblocking socket */
#include <time.h>
#include <list>
#include <unistd.h>
using namespace std;

const char *SERVER = "127.0.0.1";
const unsigned short MYPORT = 3456;

int main() {

	char inPut[128];
	int sockfd = 0, new_fd = 0; /* listen on sock_fd, new connection on new_fd */
	struct sockaddr_in my_addr; /* my address information */
	struct sockaddr_in their_addr; /* connector's address information */
	unsigned int sin_size;
	char string_read[256];
	memset(&string_read[0], 0, sizeof(string_read));

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET; /* host byte order */
	my_addr.sin_port = htons(MYPORT); /* short, network byte order */
	my_addr.sin_addr.s_addr = inet_addr(SERVER);
	bzero(&(my_addr.sin_zero), 8); /* zero the rest of the struct */

	int fd = connect(sockfd, (struct sockaddr*) &my_addr, sizeof(my_addr));

	int ret = 0;
	if (fd > 0) {
		if ((ret = send(fd, "MONA", 4, 0)) != -1) {
			if ((ret = recv(fd, &string_read[0], sizeof(string_read), 0)) != -1) {
				cout << string_read << endl;
			}
		}
		while (ret != -1) {
			cin.getline(&inPut[0], sizeof(inPut));
			ret = send(fd, &inPut[0], strlen(&inPut[0]),0);
			if (ret != -1) {
				memset(&string_read[0], 0, sizeof(string_read));
				if ((ret = recv(fd, &string_read[0], sizeof(string_read), 0)) != -1) {
					cout << string_read << endl;
				}
			}
		}
	} else {
		cout << "failed to connect" << endl;
	}

	return 0;
}
