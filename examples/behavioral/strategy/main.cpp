#include "behavioral/strategy/strategy.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

using design_pattern::behavioral::strategy::DriveStrategy;
using design_pattern::behavioral::strategy::InlineNavigator;
using design_pattern::behavioral::strategy::Navigator;
using design_pattern::behavioral::strategy::StaticStrategy;
using design_pattern::behavioral::strategy::TravelMode;
using design_pattern::behavioral::strategy::WalkStrategy;

int main() {
  std::cout << "=== 运行时换策略：Navigator 只转发 execute ===\n";
  Navigator navigator(std::make_unique<WalkStrategy>());
  std::cout << "  " << navigator.execute("家", "公司") << "\n";
  navigator.setStrategy(std::make_unique<DriveStrategy>());
  std::cout << "  " << navigator.execute("家", "公司") << "\n";

  std::cout << "\n=== 编译期按 T 选定 execute ===\n";
  StaticStrategy<WalkStrategy> walk;
  StaticStrategy<DriveStrategy> drive;
  std::cout << "  " << walk.execute("家", "公司") << "\n";
  std::cout << "  " << drive.execute("家", "公司") << "\n";

  std::cout << "\n=== 对照：方式已经关闭时，枚举分支就够了 ===\n";
  InlineNavigator spec(TravelMode::Walk);
  InlineNavigator copy = spec;
  copy.setMode(TravelMode::Drive);
  std::cout << "  原件 " << spec.execute("家", "公司") << "\n";
  std::cout << "  副本 " << copy.execute("家", "公司") << "\n";

  std::cout << "\n=== 空策略在入口拒绝 ===\n";
  try {
    Navigator missing(nullptr);
    (void)missing;
  } catch (const std::invalid_argument &error) {
    std::cout << "  " << error.what() << "\n";
  }

  return 0;
}
