#include "behavioral/memento/memento.h"

#include <iostream>

// TODO: 实现 Memento 后，在这里写客户端用法。
//
// 典型客户端：
//   1. Originator（编辑器）在改状态前 save()
//   2. Caretaker 只保管快照，不拆内部字段
//   3. 需要时 restore() 回到某一版

int main() {
  using namespace design_pattern::behavioral::memento;

  // editor.type("hello");
  // auto snap = editor.save();
  // editor.type(" world");
  // editor.restore(snap);

  std::cout << "[Memento] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
