#include "structural/flyweight/flyweight.h"

#include <iostream>

// TODO: 实现 Flyweight 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 内在状态（树种纹理）放进共享享元
//   2. 外在状态（坐标）每次绘制时传入
//   3. 通过工厂按 key 取同一份共享对象

int main() {
  using namespace design_pattern::structural::flyweight;

  // auto& oak = TreeFactory::get("oak");
  // oak.draw(x, y);

  std::cout << "[Flyweight] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
