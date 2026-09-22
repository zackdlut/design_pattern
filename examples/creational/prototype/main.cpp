#include "creational/prototype/prototype.h"

#include <iostream>

// TODO: 实现 Prototype 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 预先注册若干原型（敌人模板、文档模板）
//   2. 运行时 clone()，再改副本上的差异字段
//   3. 避免反复走昂贵的完整构造

int main() {
  using namespace design_pattern::creational::prototype;

  // auto prototype = registry.get("enemy");
  // auto clone = prototype->clone();
  // clone->set_position(x, y);

  std::cout << "[Prototype] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
