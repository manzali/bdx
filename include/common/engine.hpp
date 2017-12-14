#ifndef COMMON_ENGINE_HPP
#define COMMON_ENGINE_HPP

#include <thread>
#include <chrono>
#include <stdexcept>

#include <boost/asio.hpp>

namespace common {

namespace engine {

typedef boost::asio::basic_waitable_timer<std::chrono::high_resolution_clock> timer_t;

class engine {

 public:

  engine(int n_threads = 1)
      : m_io_service(),
        m_work(m_io_service),
        m_threads(n_threads) {
    if (n_threads <= 0) {
      throw std::runtime_error("the engine requires at least 1 thread");
    }
    for (auto& th : m_threads) {
      th = std::thread([&]() {m_io_service.run();});
    }
  }

  template<typename HandlerType>
  void post(HandlerType handler) {
    m_io_service.post(handler);  // thread safe
  }

  ~engine() {
    stop();
  }

  void stop() {
    m_io_service.stop();
    for (auto& th : m_threads) {
      if (th.joinable()) {
        th.join();
      }
    }
  }

  boost::asio::io_service& get() {
    return m_io_service;
  }

  engine(engine const&) = delete;  // non construction-copyable
  engine& operator=(engine const&) = delete;  // non copyable

 private:

  boost::asio::io_service m_io_service;
  boost::asio::io_service::work m_work;  // it is needed by the io_service
  std::vector<std::thread> m_threads;
};

}

}

#endif
