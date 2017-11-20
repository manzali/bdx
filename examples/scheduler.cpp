#include <iostream>
#include <thread>
#include <chrono>

#include "common/engine.hpp"

void hello(int v) {
  //std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << v << std::endl;
}

int main() {

  common::engine::engine e(4);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  for (int i = 0; i < 100; ++i) {
    e.post(std::bind(hello, i));
  }

  std::this_thread::sleep_for(std::chrono::seconds(20));

  e.stop();

  return EXIT_SUCCESS;
}
