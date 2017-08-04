/*
** client.c -- a stream socket client demo
*/
//wode !!
//ben tiancai xie de !!
//zhengda yanjing ! 

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <vector>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fstream> 
using namespace std;

/*
in: string: "http://hostname[:port]/path/to/file"
out: vector<string>: [hostname, port, file path]
default port:80
default file: index.html
*/

vector<string> Parse_argument(string input)
{
	//cout<<"input arg:  "<<input<<endl;
	string port_number 		=	"";
	string hostname    		=	"";
	string file_path   		=	"";
	vector<string> res;
	size_t not_space 		= 	input.find_first_not_of(" ");
	input 					=	input.substr(not_space);
	string http 			=   "http://";
	input 					=	input.substr(http.size());
	cout<<input<<endl;
	size_t slash_pos		=	input.find('/');
	size_t semicol_pos		=	input.find(':');

	if(slash_pos==string::npos and semicol_pos == string::npos)
	{
		hostname	=	input;
		port_number	=	"80";
		file_path	=	"index.html";
	}

	else{
	if(slash_pos==string::npos)
	{
		file_path	=	"index.html";
		slash_pos 	=	input.size()-1;
	}
	else	
	{
		file_path 	= 	input.substr(slash_pos+1);
		if(file_path.size()==0) file_path = "index.html";
	}

	if(semicol_pos == string::npos)
	{
		port_number	=	"80";
		hostname 	=	input.substr(0,slash_pos);
	}
	else
	{
		port_number = input.substr(semicol_pos+1, slash_pos - semicol_pos - 1);
		hostname	= input.substr(0,semicol_pos);
	}		}					

	res.push_back(hostname);
	res.push_back(port_number);
	res.push_back(file_path);
	return res;
}

/*
in: string: path
out: string: "GET /path HTTP/1.0 \r\n\r\"
*/
string build_request(string path, string hostname)
{
	string res = "GET /";
	res+=path;
	res+=" HTTP/1.0";
	res+="\r\n";
	res+="Host: "+hostname;
	res+="\r\n\r\n";
	return res;
}
/*
in: char *header
out: string: status code
*/
string Parse_header(string header_str)
{
	vector<string>	res;
	//string header_str	=	string(header);
	header_str			=	header_str.substr(9); // ignore HTTP/1.0
	size_t	space_pos	=	header_str.find(' ');
	string status_code	=	header_str.substr(0,space_pos); 		
	return status_code;
}

string Parse_redirection(string header_str)
{

	//string header_str	=	string(header);
	cout<<header_str<<endl;
	string website 		=	"";
	size_t location_pos = 	header_str.find("Location: ");
	//cout<<"Parse_redirection"<<http_pos<<endl;
	size_t http_pos		=	location_pos+10;
	website				=	header_str.substr(http_pos);
	size_t r_n_pos		=	website.find("\r\n");
	website				=	website.substr(0,r_n_pos);
	return website;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}




int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: ./http_client http://hostname[:port]/path/to/file\n");
	    exit(1);
	}

	//parse the argument and get info and it may redirect
	redirect:
	cout<<"input url:"<<argv[1]<<endl;
	vector<string> arguments	=	Parse_argument(string(argv[1]));
	char const* hostname		=	arguments[0].c_str();
	char const* port 	 		=   arguments[1].c_str();
	char const* file_path  		=	arguments[2].c_str();

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
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

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	
	string request_str		=	build_request(string(file_path), string(hostname));
	char const*  request 	=	request_str.c_str();
	send(sockfd,request,strlen(request),0);
	std::ofstream outfile("output");
	std::ofstream test("testclient.txt");

	//char* buffer 			= 	(char*)calloc(1024, sizeof(char));
	char buffer[1024];	
	memset(buffer, '\0', 1024);
	bool  header_handle 	= 	false;
	string status_code		=	"";
	while((numbytes = recv(sockfd, buffer, 1023, 0))>0)
	{	
		test.write(buffer,numbytes);
		if(header_handle==false)
		{
			status_code 	=	Parse_header(string(buffer));
			//cout<<buffer<<endl;
			cout<<"status_code: "<<status_code<<endl;

			if(status_code == "404")
			{
				//outfile << "<html><h1>404 Not Found</h1></html>\r\n\r\n"<<endl;
			}
			else if(status_code == "400")
			{
				//outfile << "<html><h1>400 Bad request</h1></html>\r\n\r\n" <<endl;
			}
			else if(status_code == "301" or status_code == "302")
			{
				string new_website = Parse_redirection(string(buffer));
				argv[1] = (char*)new_website.c_str();
				//free(buffer);
				goto redirect;
			}

			size_t empty_line_pos = string(buffer).find("\r\n\r\n");
			outfile.write(buffer+empty_line_pos+4, numbytes  - sizeof(char)*(empty_line_pos+4));
			cout<<"sb buffer:"<<strlen(buffer)<<"sb num: "<<numbytes<<endl;
			//outfile.write(buffer+empty_line_pos+4, strlen(buffer) - sizeof(char)*(empty_line_pos+4));
			memset(buffer, '\0', 1024);
			header_handle	=	true;
		}

		else
		{	
			outfile.write(buffer,numbytes);
			memset(buffer, '\0', 1024);
		}
		
	}

	//free(buffer);
	close(sockfd);
	outfile.close();
	test.close();
	return 0;
}

