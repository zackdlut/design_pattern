#include "structural/bridge/bridge.h"

#include <iostream>

// TODO: 实现 Bridge 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 抽象（Shape）和实现（Renderer）各自可换
//   2. 构造时把实现注入抽象
//   3. 画圆 / 画方 不必为每种渲染器再派生子类

int main() {
  using namespace design_pattern::structural::bridge;

  // Shape circle(std::make_unique<VectorRenderer>());
  // circle.draw();

  std::cout << "[Bridge] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
