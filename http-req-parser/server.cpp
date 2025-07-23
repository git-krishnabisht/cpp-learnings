#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

void error(const char* err_msg) {
  perror(err_msg);
  exit(1);
}

void http_get(string buffer, int clisockfd, string method, string path, string version) {

  std::ofstream osf("http_logs.txt");
  if (!osf) error("ERROR while opening the file");
  osf << buffer;

  int io;

  string res_body;
  string res_status_code;
  string res_content_type;
  string res_connection;
  string res_content_length;
  string res_headers;
  string res;

  auto header_parser = [&](string path) {
    std::ifstream inf(path);
    if (!inf) error("ERROR while opening the file");

    string line;
    while (getline(inf, line)) {
      res_body += line + "\n";
    }

    res_status_code = "200 OK";
    res_content_type = "text/html;";
    res_content_length = std::to_string(res_body.size());
    res_connection = "close";

    res_headers =
      version + " " + res_status_code + "\r\n"
      "Content-Type: " + res_content_type + "\r\n"
      "Content-Length: " + res_content_length + "\r\n"
      "Connection: " + res_connection + "\r\n\r\n";

    return res_headers + res_body;
  };

  if (path == "/") {
    res = header_parser("index.html");

    io = send(clisockfd, res.c_str(), res.size(), 0);
    if (io < 0) error("ERROR while writing to the client socket");

    return;
  }
  else {
    string _path;
    for (int i = 1; i < path.size(); ++i) {
      _path += path[i];
    }

    res = header_parser(_path);

    io = send(clisockfd, res.c_str(), res.size(), 0);
    if (io < 0) error("ERROR while writing to the client socket");

    return;
  }
}

void http_post(string buffer) {
  cout << "hellow " << buffer << endl;;
}

auto handle_client(int clisockfd) {
  char req_buffer[4096];
  memset(&req_buffer, 0, 4096);
  int io = recv(clisockfd, req_buffer, 4095, 0);
  if (io < 0) error("ERROR reading from the client socket");

  std::istringstream iss(req_buffer);
  string method, path, version;

  iss >> method >> path >> version;

  if (method == "GET") {
    http_get(req_buffer, clisockfd, method, path, version);
  }

  close(clisockfd);
}

int main(int argc, char* argv[]) {
  int sockfd;
  int clisockfd;
  struct addrinfo ai, * res;
  struct sockaddr_storage cli_addr;
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

  while (true) {
    struct sockaddr_storage cli_addr;
    socklen_t cli_len = sizeof cli_addr;

    int clisockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    if (clisockfd < 0) {
      error("ERROR on accept");
      continue;
    }

    std::thread(handle_client, clisockfd).detach();
  }

  close(sockfd);

  return 0;
}