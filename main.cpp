#include <cassert>
#include <memory>

consteval bool atomic_test() {
  std::atomic<std::shared_ptr<int>> aptr = std::make_shared<int>(42);
  std::shared_ptr<int> ptr1 = aptr.load();
  assert(*ptr1 == 42);
  
  std::shared_ptr<int> ptr2 = std::make_shared<int>(14);
  aptr.store(ptr2);
  
  std::shared_ptr<int> ptr3 = aptr.load();
  assert(*ptr3 == 14);
  
  auto out = aptr.exchange(ptr1);
  assert(out == ptr3);
  return true;
  
  auto expected = ptr1;
  if (aptr.compare_exchange_strong(expected, ptr2)) {
    assert(false);
  }
  
  expected = ptr3;
  if (!aptr.compare_exchange_strong(expected, ptr2)) {
    assert(false);
  }
}

static_assert(atomic_test());

consteval bool weak_test() {
  std::weak_ptr<int> wptr{};
  
  {
    std::shared_ptr<int> sptr = std::make_shared<int>(42);
    wptr = sptr;
    std::shared_ptr<int> sptr2 = wptr.lock();
    assert(sptr2 != nullptr);
  }
  
  std::shared_ptr<int> sptr3 = wptr.lock();
  assert(sptr3 == nullptr);
  return true;
}

static_assert(weak_test());

consteval bool atomic_weak_test() {
  std::atomic<std::weak_ptr<int>> wptr{};
  
  {
    std::shared_ptr<int> sptr = std::make_shared<int>(42);
    wptr.store(sptr);
    std::shared_ptr<int> sptr2 = wptr.load().lock();
    assert(sptr2 != nullptr);
    assert(*sptr2 == 42);
  }
  
  std::shared_ptr<int> sptr3 = wptr.load().lock();
  assert(sptr3 == nullptr);
  return true;
}

static_assert(atomic_weak_test());

int main(int argc, char *argv[])
{
  static_assert(__cpp_lib_constexpr_shared_ptr);
  static_assert((std::shared_ptr<int>{}, true));
  
  
  return 0;
}
