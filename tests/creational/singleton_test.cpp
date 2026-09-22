#include "creational/singleton/singleton.h"

#include <gtest/gtest.h>

#include <set>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

using design_pattern::creational::singleton::SingletonCallOnce;
using design_pattern::creational::singleton::SingletonDoubleCheck;
using design_pattern::creational::singleton::SingletonLasy;
using design_pattern::creational::singleton::SingletonLazyLock;
using design_pattern::creational::singleton::SingletonMeyers;
using design_pattern::creational::singleton::SingletonTemplate;
using design_pattern::creational::singleton::SingletonHolder;
namespace {

class TemplateDemo : public SingletonTemplate<TemplateDemo> {
  friend class SingletonTemplate<TemplateDemo>;
  TemplateDemo() = default;
};

template <typename T>
void expectSameAddress() {
  auto &a = T::getInstance();
  auto &b = T::getInstance();
  EXPECT_EQ(&a, &b);
}

template <typename T>
void expectSameAddressConcurrent() {
  constexpr int kThreads = 8;
  std::vector<T *> pointers(static_cast<std::size_t>(kThreads), nullptr);
  std::vector<std::thread> threads;
  threads.reserve(static_cast<std::size_t>(kThreads));
  for (int i = 0; i < kThreads; ++i) {
    threads.emplace_back([&pointers, i] { pointers[static_cast<std::size_t>(i)] = &T::getInstance(); });
  }
  for (auto &t : threads) {
    t.join();
  }
  std::set<T *> unique(pointers.begin(), pointers.end());
  EXPECT_EQ(unique.size(), 1u);
}

} // namespace

TEST(SingletonTest, MeyersReturnsSameInstance) { expectSameAddress<SingletonMeyers>(); }

TEST(SingletonTest, EagerReturnsSameInstance) { expectSameAddress<SingletonLasy>(); }

TEST(SingletonTest, LazyLockReturnsSameInstance) { expectSameAddress<SingletonLazyLock>(); }

TEST(SingletonTest, DoubleCheckReturnsSameInstance) { expectSameAddress<SingletonDoubleCheck>(); }

TEST(SingletonTest, CallOnceReturnsSameInstance) { expectSameAddress<SingletonCallOnce>(); }

TEST(SingletonTest, TemplateReturnsSameInstance) { expectSameAddress<TemplateDemo>(); }

TEST(SingletonTest, DoubleCheckIsThreadSafe) { expectSameAddressConcurrent<SingletonDoubleCheck>(); }

TEST(SingletonTest, CallOnceIsThreadSafe) { expectSameAddressConcurrent<SingletonCallOnce>(); }

TEST(SingletonTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<SingletonMeyers>);
  static_assert(!std::is_move_constructible_v<SingletonLasy>);
  static_assert(!std::is_copy_assignable_v<SingletonLazyLock>);
  static_assert(!std::is_move_assignable_v<SingletonDoubleCheck>);
}

TEST(SingletonTest, TemplateDerivedCannotBeCopied) {
  static_assert(!std::is_copy_constructible_v<TemplateDemo>);
  static_assert(!std::is_move_constructible_v<TemplateDemo>);
}

TEST(SingletonTest, SingletonHolderReturnsSameInstance) {
  class Database {
  public:
    std::string get_database_host() const { return name + "@" + ip + ":" + port; }

  private:
    std::string name = "localhost";
    std::string ip = "127.0.0.1";
    std::string port = "8888";
  };

  auto &a = SingletonHolder<Database>::getInstance();
  auto &b = SingletonHolder<Database>::getInstance();
  EXPECT_EQ(&a, &b);
  EXPECT_EQ(a.get_database_host(), "localhost@127.0.0.1:8888");
}