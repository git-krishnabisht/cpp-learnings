// server.cpp
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

using std::cout;
using std::cin;
using std::endl;
using std::cerr;

const int OSDEFPROTO = 0;

void error(const char* msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char* argv[]) {

  int sockfd;
  int clisockfd;
  struct sockaddr_storage cli_addr;
  socklen_t cli_len = sizeof cli_addr;
  char buffer[256];
  struct addrinfo ai, * res;
  int n;

  if (argc < 2) {
    cerr << "ERROR, no port provided\n";
    exit(1);
  }

  memset(&ai, 0, sizeof(ai));
  ai.ai_family = AF_INET;
  ai.ai_socktype = SOCK_STREAM;
  ai.ai_addr = INADDR_ANY;

  int status = getaddrinfo("kanyewest", argv[1], &ai, &res);
  if (status != 0) {
    cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
    exit(0);
  }

  sockfd = socket(res->ai_family, res->ai_socktype, OSDEFPROTO);
  if (sockfd < 0)
    error("ERROR opening socket");

  if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    error("ERROR on binding");

  if (listen(sockfd, 5) < 0)
    error("ERROR while listening");

  clisockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);

  if (clisockfd < 0)
    error("ERROR on accept");

  /* struct sockaddr_in peer_addr {};
  socklen_t peer_len = sizeof(peer_addr);
  if (getpeername(newsockfd, (struct sockaddr*)&peer_addr, &peer_len) == 0) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peer_addr.sin_addr, ip, sizeof(ip));
    std::cout << "Client IP: " << ip << "\n";
    std::cout << "Client Port: " << ntohs(peer_addr.sin_port) << "\n";
  } else {
    perror("getpeername");
  } */

  std::memset(buffer, 0, 256);
  n = recv(clisockfd, buffer, 255, 0);
  if (n < 0) error("ERROR reading from socket");

  cout << "Here is the message: " << buffer << endl;

  n = send(clisockfd, "I got your message", 18, 0);
  if (n < 0) error("ERROR writing to socket");

  if (shutdown(clisockfd, 2) < 0) cout << "Error shutting down newsockfd" << endl;
  if (shutdown(sockfd, 2) < 0) cout << "Error shutting down sockfd" << endl;
  close(clisockfd);
  close(sockfd);
  return 0;
}
