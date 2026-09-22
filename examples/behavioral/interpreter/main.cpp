#include "behavioral/interpreter/interpreter.h"

#include <iostream>

// TODO: 实现 Interpreter 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 把表达式解析成语法树（或直接组装 Expression）
//   2. 带着 Context 调用 interpret()
//   3. 适合小型 DSL，而不是通用编程语言

int main() {
  using namespace design_pattern::behavioral::interpreter;

  // Expression* expr = parse("1 + 2");
  // expr->interpret(context);

  std::cout << "[Interpreter] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
