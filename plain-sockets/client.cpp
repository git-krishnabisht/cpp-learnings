// #include <iostream>
// #include <cstring>
// #include <cstdlib>
// #include <unistd.h>
// #include <netdb.h>
// #include <netinet/in.h>
// #include <sys/socket.h>

// void error(const char* prog_name, const char* msg) {
//   string full_msg = std::string(prog_name) + ": " + msg;
//   perror(full_msg.c_str());
//   exit(0);
// }

// int main(int argc, char* argv[]) {
//   int sockfd, portno, n;
//   struct sockaddr_in serv_addr;
//   struct hostent* server;

//   char buffer[256];
//   if (argc < 3) {
//     cerr << "Usage: " << argv[0] << " hostname port\n";
//     exit(0);
//   }

//   portno = atoi(argv[2]);
//   sockfd = socket(AF_INET, SOCK_STREAM, 0);
//   if (sockfd < 0)
//     error(argv[0], "ERROR opening socket");

//   server = gethostbyname(argv[1]);

//   if (server == NULL) {
//     cerr << argv[0] << ": ERROR, no such host\n";
//     exit(0);
//   }

//   memset((char*)&serv_addr, 0, sizeof(serv_addr));

//   serv_addr.sin_family = AF_INET;

//   memcpy((char*)&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

//   serv_addr.sin_port = htons(portno);

//   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
//     error(argv[0], "ERROR connecting");

//   cout << "Please enter the message: ";
//   memset(buffer, 0, 256);
//   cin.getline(buffer, 255);

//   n = write(sockfd, buffer, strlen(buffer));
//   if (n < 0)
//     error(argv[0], "ERROR writing to socket");

//   memset(buffer, 0, 256);
//   n = read(sockfd, buffer, 255);
//   if (n < 0)
//     error(argv[0], "ERROR reading from socket");

//   cout << "Server replied: " << buffer << std::endl;

//   close(sockfd);
//   return 0;
// }


#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using std::cout;
using std::cin;
using std::endl;
using std::cerr;
using std::string;


void error(const char* msg) {
  perror(msg);
  exit(1);
}

void error(const char* prog_name, const char* msg) {
  string full_msg = string(prog_name) + ": " + msg;
  perror(full_msg.c_str());
  exit(0);
}

int main(int argc, char* argv[]) {
  int sockfd, n;
  struct addrinfo ai, *res;

  char buffer[256];

  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " hostname port\n";
    exit(0);
  }

  memset(&ai, 0, sizeof(ai));
  ai.ai_family = AF_INET;       // IPv4
  ai.ai_socktype = SOCK_STREAM; // TCP

  int status = getaddrinfo(argv[1], argv[2], &ai, &res);
  if (status != 0) {
    cerr << argv[0] << ": getaddrinfo: " << gai_strerror(status) << "\n";
    exit(0);
  }

  // struct addrinfo* itr = res;
  // while (itr) {
  //   void* addr;
  //   if (itr->ai_family == AF_INET) {
  //     struct sockaddr_in* ipv4 = (struct sockaddr_in*)itr->ai_addr;
  //     addr = &(ipv4->sin_addr);
  //   }
  //   else if (itr->ai_family = AF_INET6) {
  //     struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)itr->ai_addr;
  //     addr = &(ipv6->sin6_addr);
  //   }
  //   else {
  //     continue;
  //   }

  //   char ipstr[INET6_ADDRSTRLEN];
  //   inet_ntop(itr->ai_family, addr, ipstr, sizeof(ipstr));
  //   cout << ipstr << std::endl;
  //   itr = itr->ai_next;
  // }

  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (sockfd < 0)
    error(argv[0], "ERROR opening socket");

  // void *addr = nullptr;
  // struct sockaddr_in* ip = (struct sockaddr_in*)res->ai_addr;
  // addr = &(ip->sin_addr);
  // char ipstr[INET6_ADDRSTRLEN];
  // inet_ntop(res->ai_family, addr, ipstr, sizeof ipstr);
  // cout << "res->ai_addr: " << ipstr << std::endl;

  if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    error(argv[0], "ERROR connecting");

  freeaddrinfo(res); 

  cout << "Please enter the message: ";
  memset(buffer, 0, 256);
  cin.getline(buffer, 255);

  n = send(sockfd, buffer, strlen(buffer), 0);
  if (n < 0)
    error(argv[0], "ERROR writing to socket");

  memset(buffer, 0, 256);
  n = recv(sockfd, buffer, 255, 0);
  if (n < 0)
    error(argv[0], "ERROR reading from socket");

  cout << "Server replied: " << buffer << endl;

  if (shutdown(sockfd, 2) < 0) cout << "Error shutting down sockfd" << endl;
  close(sockfd);
  return 0;
}