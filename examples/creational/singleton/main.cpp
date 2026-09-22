#include "creational/singleton/singleton.h"

#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

using design_pattern::creational::singleton::SingletonHolder;
using design_pattern::creational::singleton::SingletonMeyers;
using design_pattern::creational::singleton::SingletonTemplate;

namespace {

// 进程级配置：启动时加载一次，Auth / Order 等模块都读同一份。
class AppConfig {
public:
  void load(std::string_view env) {
    if (env == "prod") {
      host_ = "10.0.0.8";
      port_ = 443;
      log_level_ = "warn";
    } else {
      host_ = "127.0.0.1";
      port_ = 8080;
      log_level_ = "info";
    }
    loaded_env_ = std::string(env);
  }

  const std::string &host() const { return host_; }
  int port() const { return port_; }
  const std::string &log_level() const { return log_level_; }
  const std::string &env() const { return loaded_env_; }

private:
  std::string host_{"127.0.0.1"};
  int port_{8080};
  std::string log_level_{"info"};
  std::string loaded_env_{"unset"};
};

using Config = SingletonHolder<AppConfig>;

// 进程级日志：各模块、各线程都写到同一个 Logger。
class Logger final : public SingletonTemplate<Logger> {
  friend class SingletonTemplate<Logger>;

public:
  void info(std::string_view module, std::string_view message) {
    std::lock_guard<std::mutex> lock(mutex_);
    records_.emplace_back(std::string("[info][") + std::string(module) + "] " +
                          std::string(message));
  }

  const std::vector<std::string> &records() const { return records_; }

private:
  Logger() = default;

  std::mutex mutex_{};
  std::vector<std::string> records_{};
};

void boot(std::string_view env) {
  auto &cfg = Config::getInstance();
  cfg.load(env);

  auto &log = Logger::getInstance();
  log.info("boot", "env=" + cfg.env() + " host=" + cfg.host() +
                       " port=" + std::to_string(cfg.port()) +
                       " log_level=" + cfg.log_level());
}

void auth_module() {
  const auto &cfg = Config::getInstance();
  Logger::getInstance().info("auth", "listen " + cfg.host() + ":" +
                                         std::to_string(cfg.port()));
}

void order_module() {
  Logger::getInstance().info("order", "ready, using shared config/logger");
}

void worker(int id) {
  Logger::getInstance().info("worker", "job-" + std::to_string(id));
}

} // namespace

int main() {
  std::cout << "=== 单例客户端：进程级配置 + 日志 ===\n";

  boot("prod");
  auth_module();
  order_module();

  auto &cfg_a = Config::getInstance();
  auto &cfg_b = Config::getInstance();
  auto &log_a = Logger::getInstance();
  auto &log_b = Logger::getInstance();

  std::cout << "Config 同一实例: " << std::boolalpha << (&cfg_a == &cfg_b)
            << "  addr=" << &cfg_a << "\n";
  std::cout << "Logger 同一实例: " << (&log_a == &log_b) << "  addr=" << &log_a
            << "\n";

  std::cout << "\n=== 多线程写同一 Logger ===\n";
  std::vector<std::thread> threads;
  threads.reserve(4);
  for (int i = 0; i < 4; ++i) {
    threads.emplace_back(worker, i);
  }
  for (auto &t : threads) {
    t.join();
  }

  for (const auto &line : Logger::getInstance().records()) {
    std::cout << line << "\n";
  }

  std::cout << "\n=== 库内 Meyers 单例同样保证唯一 ===\n";
  auto &m1 = SingletonMeyers::getInstance();
  auto &m2 = SingletonMeyers::getInstance();
  std::cout << "SingletonMeyers 同一实例: " << (&m1 == &m2) << "  addr=" << &m1
            << "\n";

  return 0;
}
