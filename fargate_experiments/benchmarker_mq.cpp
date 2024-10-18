#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <fstream>

const char* SERVER_TO_CLIENT_QUEUE = "/server_to_client_queue";
const char* CLIENT_TO_SERVER_QUEUE = "/client_to_server_queue";
const size_t MAX_MSG_SIZE = 8192;

void send_message(mqd_t mq, const std::vector<char>& message, size_t chunk_size)
{
  size_t i = 0;
  for (i = 0; i < message.size(); i += chunk_size) {

    size_t chunk_length = std::min(chunk_size, message.size() - i);
    if (mq_send(mq, message.data() + i, chunk_length, 0) == -1) {
      std::cerr << "mq_send " << errno << std::endl;
      abort(); 
    }
  }
}

void receive_message(mqd_t mq, std::vector<char>& message, size_t chunk_size)
{
  size_t received = 0;
  while (received < message.size()) {

    ssize_t chunk_length = mq_receive(mq, message.data() + received, chunk_size, nullptr);
    if (chunk_length == -1) {
      std::cerr << "mq_receive " << errno << std::endl;
      abort(); 
    }
    received += chunk_length;

  }
}

void run_server(size_t msg_size, int repetitions)
{
  size_t chunk_size = MAX_MSG_SIZE;
  mq_unlink(SERVER_TO_CLIENT_QUEUE);
  mq_unlink(CLIENT_TO_SERVER_QUEUE);

  struct mq_attr attr;
  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = MAX_MSG_SIZE;
  attr.mq_curmsgs = 0;

  mqd_t server_to_client = mq_open(SERVER_TO_CLIENT_QUEUE, O_CREAT | O_WRONLY, 0644, &attr);
  mqd_t client_to_server = mq_open(CLIENT_TO_SERVER_QUEUE, O_CREAT | O_RDONLY, 0644, &attr);

  if (server_to_client == (mqd_t)-1 || client_to_server == (mqd_t)-1) {
      perror("mq_open");
      exit(1);
  }

  std::vector<char> message(msg_size, 42);
  std::vector<char> receive(msg_size);

  std::cerr << "Start!" << std::endl;

  for (int i = 0; i < repetitions + 1; ++i) {

    receive_message(client_to_server, receive, chunk_size);
    send_message(server_to_client, message, chunk_size);

    if(message[5] != receive[5]) {
      abort();
    }
  }

  mq_close(server_to_client);
  mq_close(client_to_server);
  mq_unlink(SERVER_TO_CLIENT_QUEUE);
  mq_unlink(CLIENT_TO_SERVER_QUEUE);
}

void run_client(size_t msg_size, int repetitions, std::string output_file)
{
  size_t chunk_size = MAX_MSG_SIZE;
  mqd_t server_to_client = mq_open(SERVER_TO_CLIENT_QUEUE, O_RDONLY);
  mqd_t client_to_server = mq_open(CLIENT_TO_SERVER_QUEUE, O_WRONLY);

  if (server_to_client == (mqd_t)-1 || client_to_server == (mqd_t)-1) {
    std::cerr << "mq_open " << errno << std::endl;
    abort();
  }

  std::vector<char> message(msg_size, 42);
  std::vector<char> receive(msg_size);

  std::vector<long> measurements;

  for (int i = 0; i < repetitions + 1; ++i) {

    auto begin = std::chrono::high_resolution_clock::now();
    send_message(client_to_server, message, chunk_size);
    receive_message(server_to_client, receive, chunk_size);
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

    out_file << i << "," << measurements[i] << '\n';

  }
  out_file.close();

  mq_close(server_to_client);
  mq_close(client_to_server);
}

int main(int argc, char** argv)
{

  if (argc != 4 && argc != 5) {
    std::cerr << "Usage: " << argv[0] << " <server|client> <message_size> <repetitions>" << std::endl;
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
