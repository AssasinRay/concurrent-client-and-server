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
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream> 

#define BACKLOG 20	 // how many pending connections queue will hold
#define total_thread_num 20
#define data_size 1024

using namespace std;

pthread_t	 thread_id[total_thread_num];
int 		 client_file_descriptor[total_thread_num];	
int 		 client_id_array[total_thread_num];

/*
initilize all the global variable
*/
void initilization()
{
	for(int i=0;i<total_thread_num;i++)
	{
		client_file_descriptor[i] = -1;
		client_id_array[i]	 = i;
	}
	return;
}

/*
find empty spot for this client
*/
int find_spot()
{
	for(int i=0;i<total_thread_num;i++)
	{
		if(client_file_descriptor[i] == -1) return i; 
	}
	cout<<"no empty spot find!"<<endl;
	return -1;
}

/*
clean up
close socket file descriptor
kill thread
reset client_file_descriptor
*/
void terminate_thread(int tid, int sockfd)
{
	close(sockfd);
	pthread_join(thread_id[tid], NULL);
	client_file_descriptor[tid] = -1;
	return;
}

/*
split the request by space
*/
vector<string> split_request(string request)
{
	vector<string> res;
	request = request.substr(0,request.find("\r\n"));
	cout<<"request info"<<request<<endl;
	istringstream req(request);
	string buffer;
	while(getline(req,buffer,' '))
	{
		res.push_back(buffer);
	}
	return res;
}

/*
Parse the request if bad request status will be -1 
otherwise status be 1 and get the path
*/
vector<string> Parse_request(string request)
{
	vector<string> request_info = split_request(request);
	vector<string> res;
	string path = "";
	string status = "";
	//cout<<"request_info size"<<request_info.size()<<endl;
	// if(request_info.size() !=3 )
	// {
	// 	//cout<<"Bad Request!"<<endl;
	// 	status = "-1";
	// 	res.push_back(status);
	// 	return res;
	// }

	status = "0";
	path   = request_info[1].substr(1);
	res.push_back(status);
	res.push_back(path);

	return res;
}
void* thread_routine(void* cid)
{
	int client_id = *(int *)cid;
	int sockfd 	  = client_file_descriptor[client_id];


	char buffer[data_size ];	
	memset(buffer, '\0', data_size );


	int numbytes = recv(sockfd, buffer, data_size , 0);
	if(numbytes<=0)
	{
		perror("recv");
		terminate_thread(client_id,sockfd);
		exit(1);
	}

	const char bad_request[] = "HTTP/1.0 400 Bad Reqeust\r\n\r\n"; 
	const char not_found[] = "HTTP/1.0 404 Not Found\r\n\r\n"; 
	const char nomral_situation[] = "HTTP/1.0 200 OK\r\n\r\n";

	vector<string> request_info = Parse_request(string(buffer));
	string status = request_info[0];

	//Bad Reqeust
	if(status == "-1")
	{	
		status = "400";
		send(sockfd, bad_request, strlen(bad_request), 0);
		terminate_thread(client_id, sockfd);
		return &status;
	}

	//not found error
	string file_path = request_info[1];
	FILE *fp = fopen(file_path.c_str(), "r") ;
	if(fp == NULL)
	{
		status = "404";
		send(sockfd, not_found, strlen(not_found),0);
		terminate_thread(client_id, sockfd);
		return &status;
	}

	//normal one
	send(sockfd, nomral_situation, strlen(nomral_situation), 0);
	status = "200";
	//receive data
	char msg_body_buffer[data_size ];

	while(1)
	{
		memset(msg_body_buffer, '\0', data_size );
		size_t read_size = fread(msg_body_buffer, sizeof(char), data_size , fp);
		if(read_size <= 0) break; // finished reading
		send(sockfd, msg_body_buffer, read_size, 0);
		if(read_size < data_size ) break; //finished reading
	}

	fclose(fp);
	terminate_thread(client_id,sockfd);
	return &status;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char **argv)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	if(argc !=2)
	{
		cout<<"usage: ./httpe_server port_num"<<endl;
		exit(1);
	}

	string port_num = string(argv[1]);			//read port
	char const* PORT = port_num.c_str();
	cout<<"port num is :"<<PORT<<endl;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	initilization();

	while(1) {  // main accept() loop

		int client_id = find_spot();
		cout<<"client_id: "<<client_id<<endl;
		if(client_id == -1 ) continue; //if max occupied wait for spot
		cout<<"before accpet"<<endl;
		client_file_descriptor[client_id] = accept(sockfd, NULL, NULL);
		cout<<"client_fd:"<<client_file_descriptor[client_id]<<endl;
		cout<<"after accepct"<<endl;
		if(client_file_descriptor[client_id] == -1)
		{
			cout<<"Failed to accept"<<endl;
			continue;
		}

		pthread_t * thread_addr = &thread_id[client_id];
		int*		client_id_addr = &client_id_array[client_id];  //prevent new client change the value of client_id

		if(pthread_create(thread_addr, NULL, thread_routine, (void *) client_id_addr) != 0)
		{
			cout<<"Failed to create Thread"<<client_id<<endl;
			exit(1);
		}

	}

	return 0;
}

