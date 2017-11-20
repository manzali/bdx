#ifndef COMMON_MAKE_UNIQUE_HPP
#define COMMON_MAKE_UNIQUE_HPP

namespace common {

namespace pointer {

// std::make_unique is available starting from c++14
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args&& ...args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

}

#endif
