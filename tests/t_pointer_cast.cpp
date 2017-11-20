#include <boost/detail/lightweight_test.hpp>

#include "common/pointer_cast.hpp"

int main() {

  struct test {
    uint16_t f1;
  };

  {
    test t { 8 };
    void* v = &t;
    BOOST_TEST_EQ(t.f1, common::pointer::pointer_cast<test>(v)->f1);
  }

  {
    test const t { 8 };
    void const* v = &t;
    BOOST_TEST_EQ(t.f1, common::pointer::pointer_cast<test>(v)->f1);
  }

  {
    test t { 8 };
    void const* v = &t;
    BOOST_TEST_EQ(t.f1, common::pointer::pointer_cast<test>(v)->f1);
  }

  {
    // Test exception due to wrong alignment
    test t { 8 };
    void* v = static_cast<void*>(reinterpret_cast<unsigned char*>(&t) + 1);
    BOOST_TEST_THROWS(
        common::pointer::pointer_cast<test>(v),
        std::runtime_error);
  }

  return boost::report_errors();
}

