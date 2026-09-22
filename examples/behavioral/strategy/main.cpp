#include "behavioral/strategy/strategy.h"

#include <iostream>

// TODO: 实现 Strategy 后，在这里写客户端用法。
//
// 典型客户端：
//   1. Context（导航）持有可替换的 Strategy
//   2. 运行时换成步行 / 驾车算法
//   3. 选路逻辑不写在 Context 的 if-else 里

int main() {
  using namespace design_pattern::behavioral::strategy;

  // navigator.set_strategy(std::make_unique<WalkStrategy>());
  // navigator.build_route(from, to);

  std::cout << "[Strategy] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
