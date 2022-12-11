#include <ios>
#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <chrono>
#include <thread>
#include <curl/curl.h>

#include "base64.h"

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main(int argc, char ** argv)
{
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  for(int i = 1; i < 5; ++i)
    std::cerr << i << " " << argv[i] << std::endl;

  std::string url(argv[1]);
  int kilobytes = atoi(argv[2]);
  int repetitions = atoi(argv[3]);
  int pause = atoi(argv[4]);

  std::string test;
  int size = kilobytes - 3;
  int input_size = floor(size * 3 / 4);
  for(int i = 0; i < input_size; ++i)
    test += "0";
  std::string out = base64_encode(test);
  {
    std::ofstream out_f("input_" + std::to_string(kilobytes) + ".txt", std::ios_base::out);
    out_f << out << '\n';
  }

  curl = curl_easy_init();
  std::string post = "{\"in\": \"" + out + "\"}";
  std::string header[] = {"Content-Type: application/json"};
  if(curl) {
    int len;
    char * str = curl_easy_unescape(curl, url.c_str(), url.length(), &len);
    std::cerr << str << " " << " " << kilobytes << " " << repetitions << '\n';
    curl_easy_setopt(curl, CURLOPT_URL, str);

    struct curl_slist *list = NULL;
    list = curl_slist_append(list, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    //curl_easy_setopt(curl, CURLOPT_USERPWD, "23bc46b1-71f6-4ed5-8c54-816aa4f8c502:123zO3xZCLrMN6v2BKK1dXYFpXlPkccOFqm12CdAsMgRU4VrNZ9lyGVCGuMDGIwP");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);
    for(int i = 0; i < repetitions + 1; ++i) {
      readBuffer.clear();
      auto start = std::chrono::high_resolution_clock::now();
      res = curl_easy_perform(curl);
      auto end = std::chrono::high_resolution_clock::now();
      if(i > 0)
        std::cout << std::chrono::duration<double>(end - start).count() * 1000 << '\n';
      std::this_thread::sleep_for(std::chrono::milliseconds(pause));
    }
    curl_easy_cleanup(curl);
    curl_slist_free_all(list);
    std::ofstream out_f("output_" + std::string(argv[3]) + ".txt", std::ios_base::out);
    out_f << readBuffer << '\n';
    free(str);
    //std::cout << "Done "<<  readBuffer << std::endl;
  }
  return 0;
}

