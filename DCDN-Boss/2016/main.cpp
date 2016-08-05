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

#define MYPORT 3456    /* the port users will be connecting to */
#define BACKLOG 10     /* how many pending connections queue will hold */

static const int MAX_TIME_BETWEEN_DATA = 45;
static const int MAX_TIME_FOR_CONNECTION = MAX_TIME_BETWEEN_DATA * 4;

static const char *CYBEREZ[3][16] = {
		"Cyberez Inc.",
		"Cyb3r3z 1nc.",
		"cYber3z 11c"
};

static const char *HEX_STRING = "0123456789ABCDEF";

void generateRandomShit(char *p, unsigned int n) {
	bool addCyberez = ((rand()%100)<10?true:false);
	int where = -1;
	if(addCyberez) {
		int loc = rand()%3;
		switch(loc) {
		case 1:
			where = 0;
			break;
		case 2:
			where = n/2;
			break;
		case 3:
			where = n-16;
			break;
		default:
			break;
		}
	}
	for(unsigned int i=0;i<n;i++) {
		if(i==where) {
			int which = rand()%3;
			strcpy(&p[i],CYBEREZ[which][0]);
			i+=strlen(CYBEREZ[which][0]);
		} else {
			p[i] =HEX_STRING[rand()%17];
		}
	}
}

struct ClientInfo {
	int FD;
	int RightAnswers;
	time_t ConnectTime;
	time_t LastDataReceived;
	ClientInfo(int fd) :
			FD(fd), RightAnswers(0), ConnectTime(time(0)), LastDataReceived(time(0)) {
	}
	~ClientInfo() {
		close(FD);
	}
};

int main(int arc, char *agrv[]) {
	srand(time(0));
	int sockfd = 0, new_fd = 0; /* listen on sock_fd, new connection on new_fd */
	struct sockaddr_in my_addr; /* my address information */
	struct sockaddr_in their_addr; /* connector's address information */
	unsigned int sin_size;
	char string_read[64];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET; /* host byte order */
	my_addr.sin_port = htons(MYPORT); /* short, network byte order */
	my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
	bzero(&(my_addr.sin_zero), 8); /* zero the rest of the struct */

	fcntl(sockfd, F_SETFL, O_NONBLOCK); /* Change the socket into non-blocking state	*/
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	typedef std::list<ClientInfo *> CLIENT_LIST;
	typedef std::list<ClientInfo *>::iterator CLIENT_LIST_IT;

	CLIENT_LIST ListOfSockets;

	char Results[7][20] = { "MONA", "XfjnhD0ZQ8", "5zQXLfSo71", "E2ElmnWDuv", "MY8VBVunA6", "ZWxEcrPWc0", "4OmUw7DuEo" };

	bool keepRunning = true;
	while (keepRunning) {
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) != -1) {
			printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
			ListOfSockets.push_back(new ClientInfo(new_fd));
		}
		std::list<CLIENT_LIST_IT> removeList;
		CLIENT_LIST_IT it;
		for (it = ListOfSockets.begin(); it != ListOfSockets.end(); ++it) {
			int n = recv(new_fd, string_read, sizeof(string_read), 0);
			if (n == 0) {
				removeList.push_back(it);
			} else if (n > 0) {
				(*it)->LastDataReceived = time(0);
				if (strcmp(&Results[(*it)->RightAnswers][0], string_read) == 0) {
					if ((*it)->RightAnswers == 6) {
						static const char *success = "March Hare daemon initialized";
						send((*it)->FD, success, strlen(success), 0);
						keepRunning = false;
					} else {
						(*it)->RightAnswers++;
						char buf[128];
						generateRandomShit(&buf[0], sizeof(buf));
						//send random hex string with a chance of having cyberez in it
						send((*it)->FD,&buf[0],sizeof(buf),0);
					}
				} else {
					removeList.push_back(it);
				}
			} else {
				if (time(0) - (*it)->LastDataReceived > MAX_TIME_BETWEEN_DATA) {
					removeList.push_back(it);
				}
				if (time(0) - (*it)->ConnectTime > MAX_TIME_FOR_CONNECTION) {
					removeList.push_back(it);
				}
			}
		}
		//ListOfSockets.erase(removeList.begin(), removeList.end());
		for (std::list<CLIENT_LIST_IT>::iterator rit = removeList.begin(); rit != removeList.end(); ++rit) {
			ListOfSockets.erase(*rit);
			delete (*it);
		}
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 100 * 1000000;
		nanosleep(&ts, NULL);
	}
}

