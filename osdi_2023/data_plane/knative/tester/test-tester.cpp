#include <ios>
#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <chrono>
#include <thread>
#include <curl/curl.h>

#include "base64.h"

int main(int argc, char ** argv) {

  CURL *curl_handle;
  CURLcode res;
  std::string readBuffer;

  for(int i = 1; i < 5; ++i)
    std::cerr << i << " " << argv[i] << std::endl;

  std::string url(argv[1]);
  int kilobytes = atoi(argv[2]);
  int repetitions = atoi(argv[3]);
  int pause = atoi(argv[4]);

  std::string test;
  int size = kilobytes;
  int input_size = floor(size * 3 / 4);
  for(int i = 0; i < input_size; ++i)
    test += "0";
  std::string out = base64_encode(test);
  {
    std::ofstream out_f("input_" + std::to_string(kilobytes) + ".txt", std::ios_base::out);
    out_f << out << '\n';
  }

  curl_handle = curl_easy_init();

  if(curl_handle) {
    curl_easy_setopt(curl_handle, CURLOPT_URL, "http://knative-noop.default.34.233.97.200.sslip.io");
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, out.c_str());

    // curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, (long)std::strlen(postthis));
    // curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, stdout);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);

    res = curl_easy_perform(curl_handle);
    if(res != CURLE_OK) fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    curl_easy_cleanup(curl_handle);
    std::ofstream out_f("output_" + std::string(argv[3]) + ".txt", std::ios_base::out);
    out_f << readBuffer << '\n';
    return 0;
  }
}

