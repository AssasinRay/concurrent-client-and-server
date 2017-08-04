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

#define MAXDATASIZE 500 // max number of bytes we can get at once 
int client(char *argument);
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void write_to_server(int sockfd, char *command){
	int w;
	if ((w = write(sockfd, command, strlen(command))) < 0) {
		perror("write");
		exit(1);
	}
	// printf("c: %s",command);
}

void read_from_server(int sockfd){
	FILE *f;
    f = fopen("output", "w+");

	int bytes;
	char answer[MAXDATASIZE]; 
	memset(answer, '\0', MAXDATASIZE);
	int isHeader = 1;
	while ((bytes = recv(sockfd, answer, MAXDATASIZE, 0)) > 0) {
		answer[bytes-1] = '\0';
		if (isHeader) {
			isHeader = 0;
			char *status_code = strndup(answer + 9, 3);
			printf("status_code: %s\n", status_code);

			if (strcmp(status_code, "301") == 0) {
				char *location = answer + 42;
				//printf("%s\n", location);
				client(location);
				memset(answer, '\0', MAXDATASIZE);
				break;
			} else {
				char *end_of_header = strstr(answer, "\r\n\r\n");
    			fprintf(f, "%s\n", end_of_header + 4);
    			memset(answer, '\0', MAXDATASIZE);
			}
			
		} else {
			fprintf(f, "%s\n", answer);
			memset(answer, '\0', MAXDATASIZE);
		}

	}

	fclose(f);
}

int client(char *argument) {
	char *hostname = strdup(argument);
	hostname = hostname + 7;
	char *path = strdup(argument);
	int i;
	for (i = 7; i < strlen(argument); i++) {
		if (argument[i] == '/') {
			hostname[i-7] = '\0';
			path = path + i;
			break;
		}
	}

	if (strcmp(path, argument) == 0) {
		path = "/";
	}
	//printf("%s\n", hostname);
	//printf("%s\n", path);

	char *port = malloc(6);
	port[0] = '8';
	port[1] = '0';
	port[2] = '\0';
	for (i = 0; i < strlen(hostname); i++) {
		if (hostname[i] == ':') {
			printf("here\n");
			strncpy(port, hostname+i+1, strlen(hostname)-i-1);
			break;
		}
	}

	//printf("%s\n", port);


	char* command = malloc(26 + strlen(hostname) + strlen(path));
	command[0] = '\0';
	char* get_str = "GET ";
	strcat(command, get_str);
	strcat(command, path);
	char* http_str = " HTTP/1.0\r\n";
	strcat(command, http_str);
	char* host_str = "HOST: ";
	strcat(command, host_str);
	strcat(command, hostname);
	strcat(command, "\r\n\r\n");
	//printf("%s\n", command);
	

	struct addrinfo hints, *servinfo, *p;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM; // TCP

	int rv = 0;
	if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	int sockfd;
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

	freeaddrinfo(servinfo); 



	write_to_server(sockfd, command);
	read_from_server(sockfd);

	close(sockfd);
	free(port);
	free(command);

	return 0;
}

int main(int argc, char* argv[]){
	if (argc != 2) {
	    fprintf(stderr,"usage: http://hostname[:port]/path/to/file\n");
	    exit(1);
	}

	return client(argv[1]);

}
