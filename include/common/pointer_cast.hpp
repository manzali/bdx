#ifndef COMMON_POINTER_CAST_HPP
#define COMMON_POINTER_CAST_HPP

#include <type_traits>
#include <exception>

namespace common {

namespace pointer {

template<typename T>
bool is_aligned_for(void* p) {
  return (reinterpret_cast<uintptr_t>(p) & (std::alignment_of<T>::value - 1))
      == 0;
}

template<typename T>
bool is_aligned_for(void const* p) {
  return (reinterpret_cast<uintptr_t>(p) & (std::alignment_of<T>::value - 1))
      == 0;
}

template<typename T>
T* pointer_cast(void* p) {
  //assert(is_aligned_for<T>(p));
  if (!is_aligned_for<T>(p)) {
    throw std::runtime_error(
        "Pointer cast fails due to wrong address alignment");
  }
  return static_cast<T*>(p);
}

template<typename T>
T const* pointer_cast(void const* p) {
  if (!is_aligned_for<T>(p)) {
    throw std::runtime_error(
        "Pointer cast fails due to wrong address alignment");
  }
  return static_cast<T const*>(p);
}

}

}

#endif
