#include "behavioral/visitor/visitor.h"

#include <iostream>

// TODO: 实现 Visitor 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 元素结构（AST / 图形）稳定，操作会变
//   2. 每个节点 accept(visitor)
//   3. 新操作 = 新 Visitor，不必改节点类

int main() {
  using namespace design_pattern::behavioral::visitor;

  // for (auto& node : ast) {
  //   node.accept(pretty_printer);
  // }

  std::cout << "[Visitor] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
