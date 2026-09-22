#pragma once

#include <atomic>
#include <mutex>

// TODO: implement the Singleton (单例) pattern yourself.

namespace design_pattern::creational::singleton {

class SingletonMeyers final {
public:
  static SingletonMeyers &getInstance() noexcept {
    static SingletonMeyers instance_{};
    return instance_;
  }
  SingletonMeyers(const SingletonMeyers &) = delete;
  SingletonMeyers &operator=(const SingletonMeyers &) = delete;
  SingletonMeyers(SingletonMeyers &&) = delete;
  SingletonMeyers &operator=(SingletonMeyers &&) = delete;

private:
  SingletonMeyers() = default;
  ~SingletonMeyers() = default;
};

class SingletonLasy final {
public:
  static SingletonLasy &getInstance() noexcept { return instance_; }
  SingletonLasy(const SingletonLasy &) = delete;
  SingletonLasy &operator=(const SingletonLasy &) = delete;
  SingletonLasy(SingletonLasy &&) = delete;
  SingletonLasy &operator=(SingletonLasy &&) = delete;

private:
  SingletonLasy() = default;
  ~SingletonLasy() = default;
  static SingletonLasy instance_;
};

class SingletonLazyLock final {
public:
  static SingletonLazyLock &getInstance() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!instance_) {
      instance_ = new SingletonLazyLock();
    }
    return *instance_;
  }
  SingletonLazyLock(const SingletonLazyLock &) = delete;
  SingletonLazyLock &operator=(const SingletonLazyLock &) = delete;
  SingletonLazyLock(SingletonLazyLock &&) = delete;
  SingletonLazyLock &operator=(SingletonLazyLock &&) = delete;

private:
  SingletonLazyLock() = default;
  ~SingletonLazyLock() = default;

  inline static std::mutex mutex_{};
  // 有意泄漏：进程结束才由 OS 回收，避免静态析构顺序问题。
  inline static SingletonLazyLock *instance_ = nullptr;
};

class SingletonDoubleCheck final {
public:
  static SingletonDoubleCheck &getInstance() {
    auto *p = instance_.load(std::memory_order_acquire);
    if (!p) {
      std::lock_guard<std::mutex> lock(mutex_);
      p = instance_.load(std::memory_order_relaxed);
      if (!p) {
        p = new SingletonDoubleCheck();
        instance_.store(p, std::memory_order_release);
      }
    }
    return *p;
  }
  SingletonDoubleCheck(const SingletonDoubleCheck &) = delete;
  SingletonDoubleCheck &operator=(const SingletonDoubleCheck &) = delete;
  SingletonDoubleCheck(SingletonDoubleCheck &&) = delete;
  SingletonDoubleCheck &operator=(SingletonDoubleCheck &&) = delete;

private:
  SingletonDoubleCheck() = default;
  ~SingletonDoubleCheck() = default;

  inline static std::mutex mutex_{};
  // 有意泄漏：进程结束才由 OS 回收，避免静态析构顺序问题。
  inline static std::atomic<SingletonDoubleCheck *> instance_{nullptr};
};

class SingletonCallOnce final {
public:
  static SingletonCallOnce &getInstance() {
    std::call_once(once_, &SingletonCallOnce::create);
    return *instance_;
  }
  SingletonCallOnce(const SingletonCallOnce &) = delete;
  SingletonCallOnce &operator=(const SingletonCallOnce &) = delete;
  SingletonCallOnce(SingletonCallOnce &&) = delete;
  SingletonCallOnce &operator=(SingletonCallOnce &&) = delete;

private:
  SingletonCallOnce() = default;
  ~SingletonCallOnce() = default;

  static void create() { instance_ = new SingletonCallOnce(); }

  inline static std::once_flag once_{};
  // 有意泄漏：进程结束才由 OS 回收，避免静态析构顺序问题。
  inline static SingletonCallOnce *instance_ = nullptr;
};

// CRTP 单例：派生类需 friend 本模板，并把构造函数放在 private。
// class Logger : public SingletonTemplate<Logger> {
//   friend class SingletonTemplate<Logger>;
//   Logger() = default;
// };
template <typename T> class SingletonTemplate {
public:
  static T &getInstance() {
    static T instance{};
    return instance;
  }
  SingletonTemplate(const SingletonTemplate &) = delete;
  SingletonTemplate &operator=(const SingletonTemplate &) = delete;
  SingletonTemplate(SingletonTemplate &&) = delete;
  SingletonTemplate &operator=(SingletonTemplate &&) = delete;

protected:
  SingletonTemplate() = default;
  ~SingletonTemplate() = default;
};

template <typename T> class SingletonHolder {
public:
  static T &getInstance() {
    static T instance{};
    return instance;
  }
  SingletonHolder() = delete;
  ~SingletonHolder() = delete;
  SingletonHolder(const SingletonHolder &) = delete;
  SingletonHolder &operator=(const SingletonHolder &) = delete;
  SingletonHolder(SingletonHolder &&) = delete;
  SingletonHolder &operator=(SingletonHolder &&) = delete;
};
} // namespace design_pattern::creational::singleton
