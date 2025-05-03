//#include <memory>
#define _GLIBCXX_VISIBILITY(x)
#define _GLIBCXX_BEGIN_NAMESPACE_VERSION
#define _GLIBCXX_END_NAMESPACE_VERSION
#define _GLIBCXX26_CONSTEXPR constexpr
#define __glibcxx_out_ptr
#include <memory>
#include <cassert>
#include "out_ptr.h"


namespace outptr_raw_typed_ptr {
  constexpr void fnc(int ** ptr) {
    assert(ptr != nullptr);
    assert(*ptr == nullptr);
    *ptr = new int{42};
  }
  
  constexpr bool test() {
    int * ptr = nullptr;
    fnc(hana_std::out_ptr(ptr));
    if (ptr) {
      int value = *ptr;
      delete ptr;
      return value == 42;
    } else {
      return false;
    }
  }
  
  static_assert(test());
}

namespace outptr_raw_void_ptr {
  constexpr void fnc(void ** ptr) {
    assert(ptr != nullptr);
    assert(*ptr == nullptr);
    *ptr = new int{42};
  }
  
  constexpr bool test() {
    int * ptr = nullptr;
    fnc(hana_std::out_ptr(ptr));
    if (ptr) {
      int value = *ptr;
      delete ptr;
      return value == 42;
    } else {
      return false;
    }
  }
  
  static_assert(test());
}

namespace outptr_unique_typed_ptr {
  constexpr void fnc(int ** ptr) {
    assert(ptr != nullptr);
    assert(*ptr == nullptr);
    *ptr = new int{42};
  }
  
  constexpr bool test() {
    std::unique_ptr<int> ptr;
    fnc(hana_std::out_ptr(ptr));
    if (ptr) {
      return *ptr == 42;
    } else {
      return false;
    }
  }
  
  static_assert(test());
}

namespace outptr_unique_void_ptr {
  constexpr void fnc(void ** ptr) {
    assert(ptr != nullptr);
    assert(*ptr == nullptr);
    *ptr = new int{42};
  }
  
  constexpr bool test() {
    std::unique_ptr<int> ptr;
    fnc(hana_std::out_ptr(ptr));
    if (ptr) {
      return *ptr == 42;
    } else {
      return false;
    }
  }
  
  static_assert(test());
}

namespace outptr_shared_typed_ptr {
  constexpr void fnc(int ** ptr) {
    assert(ptr != nullptr);
    assert(*ptr == nullptr);
    *ptr = new int{42};
  }
  
  constexpr bool test() {
    std::shared_ptr<int> ptr;
    fnc(hana_std::out_ptr(ptr));
    if (ptr) {
      return *ptr == 42;
    } else {
      return false;
    }
  }
  
  static_assert(test());
}

namespace outptr_shared_void_ptr {
  constexpr void fnc(void ** ptr) {
    assert(ptr != nullptr);
    assert(*ptr == nullptr);
    *ptr = new int{42};
  }
  
  constexpr bool test() {
    std::shared_ptr<int> ptr;
    fnc(hana_std::out_ptr(ptr));
    if (ptr) {
      return *ptr == 42;
    } else {
      return false;
    }
  }
  
  static_assert(test());
}


namespace inoutptr_raw_typed_ptr {
  constexpr void fnc(int ** ptr) {
    assert(ptr != nullptr);
    assert(*ptr != nullptr);
    assert(**ptr == 13);
    delete *ptr;
    *ptr = new int{42};
  }
  
  constexpr bool test() {
    int * ptr = new int{13};
    fnc(hana_std::inout_ptr(ptr));
    if (ptr) {
      int value = *ptr;
      delete ptr;
      return value == 42;
    } else {
      return false;
    }
  }
  
  static_assert(test());
}

namespace inoutptr_raw_void_ptr {
  constexpr void fnc(void ** ptr) {
    assert(ptr != nullptr);
    assert(*ptr != nullptr);
    assert(*static_cast<int *>(*ptr) == 13);
    delete static_cast<int *>(*ptr);
    *ptr = new int{42};
  }
  
  constexpr bool test() {
    int * ptr = new int{13};
    fnc(hana_std::inout_ptr(ptr));
    if (ptr) {
      int value = *ptr;
      delete ptr;
      return value == 42;
    } else {
      return false;
    }
  }
  
  static_assert(test());
}

namespace inoutptr_unique_typed_ptr {
  constexpr void fnc(int ** ptr) {
    assert(ptr != nullptr);
    assert(*ptr != nullptr);
    assert(**ptr == 13);
    delete *ptr;
    *ptr = new int{42};
  }
  
  constexpr bool test() {
    std::unique_ptr<int> ptr{std::make_unique<int>(13)};
    fnc(hana_std::inout_ptr(ptr));
    if (ptr) {
      return *ptr == 42;
    } else {
      return false;
    }
  }
  
  static_assert(test());
}

namespace inoutptr_unique_void_ptr {
  constexpr void fnc(void ** ptr) {
    assert(ptr != nullptr);
    assert(*ptr != nullptr);
    assert(*static_cast<int *>(*ptr) == 13);
    delete static_cast<int *>(*ptr);
    *ptr = new int{42};
  }
  
  constexpr bool test() {
    std::unique_ptr<int> ptr{std::make_unique<int>(13)};
    fnc(hana_std::inout_ptr(ptr));
    if (ptr) {
      return *ptr == 42;
    } else {
      return false;
    }
  }
  
  static_assert(test());
}

int main(int argc, char *argv[]) {
  
}