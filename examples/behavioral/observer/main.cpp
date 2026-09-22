#include "behavioral/observer/observer.h"

#include <iostream>

// TODO: 实现 Observer 后，在这里写客户端用法。
//
// 典型客户端：
//   1. Subject 维护订阅列表
//   2. UI / Logger attach 上去
//   3. 状态一变，所有观察者收到通知

int main() {
  using namespace design_pattern::behavioral::observer;

  // stock.attach(ui);
  // stock.attach(logger);
  // stock.set_price(10.5);

  std::cout << "[Observer] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
