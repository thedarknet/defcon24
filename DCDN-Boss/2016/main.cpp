#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <fcntl.h> /* Added for the nonblocking socket */
#include <time.h>
#include <list>
#include <unistd.h>
#include <string>
#include <cstring>

#define MYPORT 3456    /* the port users will be connecting to */
#define BACKLOG 10     /* how many pending connections queue will hold */

static const int MAX_TIME_BETWEEN_DATA = 120;
static const int MAX_TIME_FOR_CONNECTION = MAX_TIME_BETWEEN_DATA * 4;

static const char *CYBEREZ[64][7] = { "Cyberez Inc.", "Cyb3r3z 1nc.", "cYber3z", "debug: ok",
		"error: memory invalid", "success", "jack of spades initialized" };

static const char *HEX_STRING = "0123456789ABCDEF";

void generateRandomShit(char *p, unsigned int n) {
	bool addCyberez = ((rand() % 100) < 25 ? true : false);
	int where = -1;
	if (addCyberez) {
		int loc = rand() % 3;
		switch (loc) {
		case 1:
			where = 0;
			break;
		case 2:
			where = n / 2;
			break;
		case 3:
			where = n - 32;
			break;
		default:
			break;
		}
	}
	for (unsigned int i = 0; i < n - 1; i++) {
		if (i == where) {
			int which = rand() % 7;
			strncpy(&p[i], CYBEREZ[0][which], strlen(CYBEREZ[0][which]));
			i += strlen(CYBEREZ[0][which]);
		} else {
			p[i] = HEX_STRING[rand() % 16];
		}
	}
	p[n] = '\0';
}

struct ClientInfo {
	int FD;
	int RightAnswers;
	time_t ConnectTime;
	time_t LastDataReceived;
	std::string InputBuffer;
	std::string OutputBuffer;
	bool Dead;
	void bufferIn() {
		char buf[256];
		int n = recv(FD, &buf[0], sizeof(buf), 0);
		if (n > 0) {
			InputBuffer.append(&buf[0], n);
			LastDataReceived = time(0);
		} else {
			if (n == 0 || (n < 0 && errno != EAGAIN)) {
				Dead = true;
			}
		}
	}
	void bufferOut(const char *b, int n) {
		OutputBuffer.append(b, n);
	}
	void sendAll() {
		if (OutputBuffer.length() > 0) {
			int n = 0;
			while (OutputBuffer.length() > 0 && (n = send(FD, OutputBuffer.data(), OutputBuffer.length(), 0)) > 0) {
				OutputBuffer = OutputBuffer.substr(n);
			}
			if (n == 0 || (n == -1 && errno != EAGAIN)) {
				Dead = true;
			}
		}
	}
	ClientInfo(int fd) :
			FD(fd), RightAnswers(0), ConnectTime(time(0)), LastDataReceived(time(0)), InputBuffer(), OutputBuffer(), Dead(
					false) {
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

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET; /* host byte order */
	my_addr.sin_port = htons(MYPORT); /* short, network byte order */
	my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
	bzero(&(my_addr.sin_zero), 8); /* zero the rest of the struct */

	fcntl(sockfd, F_SETFL, O_NONBLOCK); /* Change the socket into non-blocking state	*/
	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
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
	char Prompt[7][20] = { "#connection\n", "#datadown\n", "#dataup\n", "#keygen\n", "#10/6\n", "#initiate\n" };

	bool keepRunning = true;
	while (keepRunning) {
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) != -1) {
			fcntl(new_fd, F_SETFL, O_NONBLOCK);
			printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
			ListOfSockets.push_back(new ClientInfo(new_fd));
		}
		std::list<CLIENT_LIST_IT> removeList;
		CLIENT_LIST_IT it;
		for (it = ListOfSockets.begin(); it != ListOfSockets.end(); ++it) {
			if (!(*it)->Dead) {
				(*it)->bufferIn();

				if ((*it)->InputBuffer.length() > 1) {
					if (strncmp(Results[(*it)->RightAnswers], (*it)->InputBuffer.data(),
							strlen(Results[(*it)->RightAnswers])) == 0) {
						(*it)->InputBuffer.clear();
						if ((*it)->RightAnswers == 6) {
							static const char *success = "March Hare daemon initialized.\nConnection Terminated";
							send((*it)->FD, success, strlen(success), 0);
							keepRunning = false;
						} else {
							//send((*it)->FD, Prompt[(*it)->RightAnswers], strlen(Prompt[(*it)->RightAnswers]), 0);
							(*it)->bufferOut(Prompt[(*it)->RightAnswers], strlen(Prompt[(*it)->RightAnswers]));
							(*it)->RightAnswers++;
							char buf[128];
							generateRandomShit(&buf[0], sizeof(buf));
							(*it)->bufferOut(buf, 128);
						}
					} else {
						printf("Wrong answer sent by connection: %s", inet_ntoa(their_addr.sin_addr));
						const char *message = "Incorrect code.\nConnection closed.";
						(*it)->bufferOut(message, strlen(message));
						(*it)->Dead = true;
					}
				} else {
					if (time(0) - (*it)->LastDataReceived > MAX_TIME_BETWEEN_DATA) {
						printf("%s too much time between data", inet_ntoa(their_addr.sin_addr));
						(*it)->Dead = true;
					}
					if (time(0) - (*it)->ConnectTime > MAX_TIME_FOR_CONNECTION) {
						printf("%s was connected for too long", inet_ntoa(their_addr.sin_addr));
						(*it)->Dead = true;
					}
				}
				(*it)->sendAll();
			} else {
				removeList.push_back(it);
			}
		}
		//ListOfSockets.erase(removeList.begin(), removeList.end());
		for (std::list<CLIENT_LIST_IT>::iterator rit = removeList.begin(); rit != removeList.end(); ++rit) {
			ListOfSockets.erase(*rit);
			printf("dropping connection");
			delete (*(*rit));
		}
		usleep(50);
	}
}

