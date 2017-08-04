#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <fstream>  
#include <sstream>
using namespace std;

vector<string> split_request(string request)
{
	vector<string> res;
	istringstream req(request);
	string buffer;
	while(getline(req,buffer,' '))
	{
		res.push_back(buffer);
	}
	return res;
}

vector<string> Parse_request(string request)
{
	vector<string> request_info = split_request(request);
	vector<string> res;
	string path = "";
	string status = "";

	if(request_info.size() != 3 or request_info[0] != "GET" or request_info[1].size()<=1 or request_info[2] != "HTTP/1.0")
	{
		cout<<"Bad Request!"<<endl;
		status = "-1";
		res.push_back(status);
		return res;
	}

	status = "0";
	path   = request_info[1].substr(1);
	res.push_back(status);
	res.push_back(path);

	return res;



}
/*
in: "http://hostname[:port]/path/to/file"
out: vector -> [hostname, port, file path]
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
	//cout<<input<<endl;
	size_t slash_pos		=	input.find('/');
	size_t semicol_pos		=	input.find(':');

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
	}							
	

	cout<<"hostname:   "<< hostname<<endl;
	cout<<"port_number:  "<< port_number<<endl;
	cout<<"file_path:  "<< file_path<<endl;
	res.push_back(hostname);
	res.push_back(port_number);
	res.push_back(file_path);

	return res;
}

string build_request(string path)
{
	string res = "GET /";
	res+=path;
	res+=" HTTP/1.0";
	res+="\r\n\r\n";
	return res;
}

string Parse_header(char *header)
{
	vector<string>	res;
	string header_str	=	string(header);
	header_str			=	header_str.substr(9); // ignore HTTP/1.0
	size_t	space_pos	=	header_str.find(' ');
	string status_code	=	header_str.substr(0,space_pos); 		
	return status_code;
}

string Parse_redirection(char * header)
{
	string header_str	=	string(header);
	string website 		=	"";
	size_t http_pos 	= 	header_str.find("http://");
	website				=	header_str.substr(http_pos);
	size_t r_n_pos		=	website.find("\r\n");
	website				=	website.substr(0,r_n_pos);
	return website;

}
int main(int argc, char **argv)
{
	//"usage: ./http_client http://hostname[:port]/path/to/file\n"


	/*vector<string> arguments = Parse_argument(string(argv[1]));
	//cout<<arguments[0]<<endl;
	//cout<<arguments[1]<<endl;
	//cout<<arguments[2]<<endl;
	char const* caonima = arguments[2].c_str();
	//cout<<caonima<<endl;

	string asdd = build_request(string(caonima));
	char const* asd = asdd.c_str();
	cout<<asd<<endl;*/

	 /*char *cao = "HTTP/1.1 400  sad    kengideneirong\r\n\r\n watchmeneineimonijijisbsbsbsbs";
	string status_code = Parse_header(cao);
	cout<<cao<<endl;
	cout<<status_code<<endl;*/


/*std::ofstream outfile ("test.txt");
char* wuliao = "HTTP/1.1 400  sad    kengideneirong\r\n\r\n watchmeneineimonijijisbsbsbsbs\r\n\r\n#@ =nitamade chile dabian \r\n";
//outfile<< wuliao;

				size_t empty_line_pos = string(wuliao).find("\r\n\r\n");
				outfile.write(wuliao+empty_line_pos+4, strlen(wuliao) - sizeof(char)*(empty_line_pos+4));*/

	// 	char* buffer = "HTTP/1.1 400 kengideneirong\r\nLocation: http://www.google.com/caonima /asda /asd a////asd/\r\n asdasdasd \r\n 日乐购";
	// string sb = Parse_redirection(buffer);
	// cout<<sb<<endl;


				//string req = "GET /dir/subdir/index.php HTTP/1.0";
				string req = "GET /index.pup HTTP/1.0 ";
				vector<string> info = Parse_request(req);
				cout<<info[0]<<endl;
				cout<<info[1]<<endl;
	return 0;


}