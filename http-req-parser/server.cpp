#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <fstream>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

void error(const char* err_msg) {
  perror(err_msg);
  exit(1);
}

void http_get(std::string buffer, int clisockfd) {

  std::ofstream osf("http_logs.txt");
  if (!osf) error("ERROR while opening the file");
  osf << buffer;

  string body = "<html><body><p>hellow world <p/></body></html>";
  string headers =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Content-Length: " + std::to_string(body.size()) + "\r\n"
    "Connection: close\r\n\r\n";

  string res = headers + body;

  int _io = send(clisockfd, res.c_str(), res.size(), 0);

  if (_io < 0) error("ERROR while writing to the client socket");
  return;
}

void http_post(string buffer) {
  cout << "hellow " << buffer << endl;;
}

int main(int argc, char* argv[]) {
  int sockfd;
  int clisockfd;
  struct addrinfo ai, * res;
  struct sockaddr_storage cli_addr;
  char req_buffer[4096];
  int _io;
  socklen_t cli_len = sizeof cli_addr;
  int _ai_status;

  if (argc < 2) error("ERROR, not enough arguements provided");

  memset(&ai, 0, sizeof(ai));
  ai.ai_family = AF_INET;
  ai.ai_socktype = SOCK_STREAM;

  _ai_status = getaddrinfo("localhost", argv[1], &ai, &res);
  if (_ai_status != 0) {
    cerr << argv[0] << ": getaddrinfo: " << gai_strerror(_ai_status) << "\n";
    exit(0);
  }

  sockfd = socket(res->ai_family, res->ai_socktype, 0);
  if (sockfd < 0) error("ERROR while initialize the socket descriptor");

  if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) error("ERROR on binding");

  if (listen(sockfd, 5) < 0) error("ERROR while listening");

  clisockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);

  if (clisockfd < 0) error("ERROR accepting client reqeust");

  memset(&req_buffer, 0, 4096);

  _io = recv(clisockfd, req_buffer, 4095, 0);
  if (_io < 0) error("ERROR reading from the client socket");

  string req_str_buffer(req_buffer);

  if (req_str_buffer.substr(0, 6) == "GET / ") {
    http_get(req_str_buffer, clisockfd);
  }

  cout << "200 OK" << endl;

  if (req_str_buffer.substr(0, 5) == "POST ") {
    http_post(req_str_buffer);
  }

  close(clisockfd);
  close(sockfd);

  return 0;
}