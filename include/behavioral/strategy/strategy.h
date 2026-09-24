#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <string_view>

namespace design_pattern::behavioral::strategy {

// Strategy。算法接口。调用方只认 execute。
class Strategy {
public:
  virtual ~Strategy() = default;
  virtual std::string execute(std::string_view from,
                              std::string_view to) const = 0;
};

class WalkStrategy final : public Strategy {
public:
  std::string execute(std::string_view from,
                      std::string_view to) const override;
};

class DriveStrategy final : public Strategy {
public:
  std::string execute(std::string_view from,
                      std::string_view to) const override;
};

// Context。持有当前策略。setStrategy 整段换掉，execute 只做转发。
class Navigator {
public:
  explicit Navigator(std::unique_ptr<Strategy> strategy);

  void setStrategy(std::unique_ptr<Strategy> strategy);
  std::string execute(std::string_view from, std::string_view to) const;

private:
  std::unique_ptr<Strategy> strategy_;
};

// 编译期按 T 选定具体类的 execute，不走虚函数表。
// T 必须是 Strategy 的派生类，例如 StaticStrategy<WalkStrategy>。
template <typename T>
  requires std::derived_from<T, Strategy>
class StaticStrategy {
public:
  std::string execute(std::string_view from, std::string_view to) const {
    return strategy_.T::execute(from, to);
  }

private:
  T strategy_{};
};

// 对照用的封闭出行方式。只有这两种，再加一种就要改 InlineNavigator。
enum class TravelMode { Walk, Drive };

// 对照。分支写在这个类里，没有 Strategy 对象。
class InlineNavigator final {
public:
  explicit InlineNavigator(TravelMode mode);

  void setMode(TravelMode mode);
  std::string execute(std::string_view from, std::string_view to) const;

private:
  TravelMode mode_;
};

}  // namespace design_pattern::behavioral::strategy
