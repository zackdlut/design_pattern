#include "behavioral/command/command.h"

#include <iostream>

// TODO: 实现 Command 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 把「开灯」封装成 Command 对象
//   2. Invoker（遥控器）只调 execute / undo
//   3. 可排队、可撤销、可日志重放

int main() {
  using namespace design_pattern::behavioral::command;

  // remote.set_command(std::make_unique<LightOnCommand>(light));
  // remote.press();
  // remote.undo();

  std::cout << "[Command] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
