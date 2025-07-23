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

string path_not_found(const string& version) {
  string res_body = "<html><body>404 Not Found</body></html>\n";
  string res_headers =
    version + " 404 Not Found\r\n"
    "Content-Type: text/html;\r\n"
    "Content-Length: " + std::to_string(res_body.size()) + "\r\n"
    "Connection: close\r\n\r\n";
  return res_headers + res_body;
}

string header_parser(const string& filepath, const string& version) {

  std::ifstream inf(filepath);
  if (!inf) return path_not_found(version);

  string res_body, line;
  while (getline(inf, line)) {
    res_body += line + "\n";
  }

  string res_headers =
    version + " 200 OK\r\n"
    "Content-Type: text/html;\r\n"
    "Content-Length: " + std::to_string(res_body.size()) + "\r\n"
    "Connection: close\r\n\r\n";

  return res_headers + res_body;
}

void http_get(string buffer, int clisockfd, string method, string path, string version) {

  std::ofstream osf("http_logs.txt");
  if (!osf) error("ERROR while opening the file");
  osf << buffer;

  int io;
  string res;

  if (path == "/") {
    res = header_parser("index.html", version);

    io = send(clisockfd, res.c_str(), res.size(), 0);
    if (io < 0) error("ERROR while writing to the client socket");

    return;
  } else {
    string safe_path = path.substr(1);

    if (safe_path.find("..") != string::npos) {
      res = path_not_found(version);
    } else {
      res = header_parser(safe_path, version);
    }

    io = send(clisockfd, res.c_str(), res.size(), 0);
    if (io < 0) error("ERROR while writing to the client socket");

    return;
  }
}

void http_post(string buffer) {
  cout << "HTTP POST: " << buffer << endl;;
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