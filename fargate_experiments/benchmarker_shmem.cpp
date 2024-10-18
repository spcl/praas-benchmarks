#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <chrono>
#include <vector>
#include <fstream>

const char* SHM_NAME = "/my_shared_memory";
const char* SEM_SERVER_NAME = "/sem_server";
const char* SEM_CLIENT_NAME = "/sem_client";

void send_message(char* shm, sem_t* sem_sender, sem_t* sem_receiver, const char* buffer, size_t msg_size)
{
  std::memcpy(shm, buffer, msg_size);
  //sem_post(sem_receiver);
  //sem_wait(sem_sender);
}

void receive_message(char* shm, sem_t* sem_receiver, sem_t* sem_sender, char* buffer, size_t msg_size)
{
  //sem_wait(sem_receiver);
  std::memcpy(buffer, shm, msg_size);
  //sem_post(sem_sender);
}

void run_server(size_t msg_size, int repetitions)
{
  // Create shared memory
  int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
      std::cerr << "shm_open failed" << std::endl;
      exit(EXIT_FAILURE);
  }
  
  if (ftruncate(shm_fd, sizeof(char) * msg_size) == -1) {
      std::cerr << "ftruncate failed" << std::endl;
      exit(EXIT_FAILURE);
  }

  char* shm = (char*)mmap(0, sizeof(char) * msg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm == MAP_FAILED) {
      std::cerr << "mmap failed" << std::endl;
      exit(EXIT_FAILURE);
  }

  // Create semaphores
  sem_t* sem_server = sem_open(SEM_SERVER_NAME, O_CREAT, 0666, 0);
  sem_t* sem_client = sem_open(SEM_CLIENT_NAME, O_CREAT, 0666, 0);
  if (sem_server == SEM_FAILED || sem_client == SEM_FAILED) {
      std::cerr << "sem_open failed" << std::endl;
      exit(EXIT_FAILURE);
  }

  std::cerr << "Start!" << std::endl;

  char* buffer = new char[msg_size];
  std::fill_n(buffer, msg_size, 42);  // Fill buffer with value 42

  for (int i = 0; i < repetitions + 1; ++i)
  {
    // Wait for message from client
    sem_wait(sem_server);
    receive_message(shm, sem_server, sem_client, buffer, msg_size);
    send_message(shm, sem_server, sem_client, buffer, msg_size);
    // Let client know the data is ready
    sem_post(sem_client);
  }

  // Cleanup
  delete[] buffer;
  munmap(shm, sizeof(char) * msg_size);
  close(shm_fd);
  shm_unlink(SHM_NAME);
  sem_close(sem_server);
  sem_close(sem_client);
  sem_unlink(SEM_SERVER_NAME);
  sem_unlink(SEM_CLIENT_NAME);
}

void run_client(size_t msg_size, int repetitions, std::string output_file)
{
  // Open shared memory
  int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
  if (shm_fd == -1) {
    std::cerr << "shm_open failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  char* shm = (char*)mmap(0, sizeof(char) * msg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm == MAP_FAILED) {
    std::cerr << "mmap failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  sem_t* sem_server = sem_open(SEM_SERVER_NAME, 0);
  sem_t* sem_client = sem_open(SEM_CLIENT_NAME, 0);
  if (sem_server == SEM_FAILED || sem_client == SEM_FAILED) {
    std::cerr << "sem_open failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  char* buffer = new char[msg_size];
  std::fill_n(buffer, msg_size, 42);  // Fill buffer with value 42

  std::vector<long> measurements;
  measurements.reserve(repetitions);

  for (int i = 0; i < repetitions + 1; ++i) {

    auto begin = std::chrono::high_resolution_clock::now();
    send_message(shm, sem_client, sem_server, buffer, msg_size);
    // Let server know we sent data
    sem_post(sem_server);
    // Wait for reply
    sem_wait(sem_client);
    receive_message(shm, sem_client, sem_server, buffer, msg_size);
    auto end = std::chrono::high_resolution_clock::now();

    if(i > 0) {
      measurements.emplace_back(
          std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()
      );
    }
  }

  std::ofstream out_file{output_file, std::ios::out};
  out_file << "size,repetition,time\n";
  for(int i = 0; i < measurements.size(); ++i) {
      out_file << msg_size << "," << i << "," << measurements[i] << '\n';
  }
  out_file.close();

  // Cleanup
  delete[] buffer;
  munmap(shm, sizeof(char) * msg_size);
  close(shm_fd);
  sem_close(sem_server);
  sem_close(sem_client);
}

int main(int argc, char** argv)
{
  if (argc != 4 && argc != 5) {
    std::cerr << "Usage: " << argv[0] << " <server|client> <message_size> <repetitions> [output_file]" << std::endl;
    return 1;
  }

  std::string mode = argv[1];
  size_t msg_size = std::stoull(argv[2]);
  int repetitions = std::atoi(argv[3]);

  if (mode == "server") {
    run_server(msg_size, repetitions);
  } else if (mode == "client") {
    if (argc != 5) {
        std::cerr << "Client mode requires an output file." << std::endl;
        return 1;
    }
    run_client(msg_size, repetitions, argv[4]);
  } else {
    std::cerr << "Invalid mode. Use 'server' or 'client'." << std::endl;
    return 1;
  }

  return 0;
}
