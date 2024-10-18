#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <fstream>

const char* SOCKET_PATH = "/tmp/uds_example_socket";
const size_t MAX_MSG_SIZE = 8192;

void send_message(int sockfd, const std::vector<char>& message)
{
  size_t sent = 0, total_size = message.size();
  while (sent < message.size()) {

    ssize_t to_send = total_size - sent;
    ssize_t chunk_length = send(sockfd, message.data() + sent, to_send, 0);
    if (sent == -1) {
      std::cerr << "send " << errno << std::endl;
      abort();
    }

    sent += chunk_length;
  }
}

void receive_message(int sockfd, std::vector<char>& message)
{
  size_t received = 0, total_size = message.size();
  while (received < total_size) {

    ssize_t to_read = total_size - received;
    ssize_t chunk_length = recv(sockfd, message.data() + received, to_read, 0);
    if (chunk_length == -1) {
        std::cerr << "recv " << errno << std::endl;
        abort();
    }
    received += chunk_length;

  }
}

void run_server(size_t msg_size, int repetitions)
{
  size_t chunk_size = MAX_MSG_SIZE;
  int server_fd, client_fd;
  struct sockaddr_un address;

  if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) {
    std::cerr << "socket failed" << std::endl;
    abort();
  }

  memset(&address, 0, sizeof(address));
  address.sun_family = AF_UNIX;
  strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

  unlink(SOCKET_PATH);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    std::cerr << "bind failed" << std::endl;
    abort();
  }

  if (listen(server_fd, 1) < 0) {
    std::cerr << "listen failed" << std::endl;
    abort();
  }

  std::cerr << "Start!" << std::endl;

  if ((client_fd = accept(server_fd, NULL, NULL)) < 0) {
    std::cerr << "accept failed" << std::endl;
    abort();
  }

  std::vector<char> message(msg_size, 42);
  std::vector<char> receive(msg_size);

  for (int i = 0; i < repetitions + 1; ++i) {
    receive_message(client_fd, receive);
    send_message(client_fd, message);

    if(message[5] != receive[5]) {
      abort();
    }
  }

  close(client_fd);
  close(server_fd);
  unlink(SOCKET_PATH);
}

void run_client(size_t msg_size, int repetitions, std::string output_file)
{
  size_t chunk_size = MAX_MSG_SIZE;
  int sock = 0;
  struct sockaddr_un serv_addr;

  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    std::cerr << "Socket creation error" << std::endl;
    abort();
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strncpy(serv_addr.sun_path, SOCKET_PATH, sizeof(serv_addr.sun_path) - 1);

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cerr << "Connection Failed" << std::endl;
    abort();
  }

  std::vector<char> message(msg_size, 42);
  std::vector<char> receive(msg_size);

  std::vector<long> measurements;

  for (int i = 0; i < repetitions + 1; ++i) {

    auto begin = std::chrono::high_resolution_clock::now();
    send_message(sock, message);
    receive_message(sock, receive);
    auto end = std::chrono::high_resolution_clock::now();

    if(message[5] != receive[5]) {
      abort();
    }

    if(i > 0) {
      measurements.emplace_back(
          std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()
      );
    }
  }

  std::ofstream out_file{output_file, std::ios::out};
  out_file << "size, repetition, time" << '\n';
  for(int i = 0; i < measurements.size(); ++i) {
      out_file << msg_size << "," << i << "," << measurements[i] << '\n';
  }
  out_file.close();

  close(sock);
}

int main(int argc, char** argv)
{
  if (argc != 4 && argc != 5) {
    std::cerr << "Usage: " << argv[0] << " <server|client> <message_size> <repetitions> [output_file]" << std::endl;
    return 1;
  }

  std::string mode = argv[1];
  int msg_size = std::atoi(argv[2]);
  int repetitions = std::atoi(argv[3]);

  if (mode == "server") {
    run_server(msg_size, repetitions);
  } else if (mode == "client") {
    run_client(msg_size, repetitions, argv[4]);
  } else {
    std::cerr << "Invalid mode. Use 'server' or 'client'." << std::endl;
    return 1;
  }

  return 0;
}
