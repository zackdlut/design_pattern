#include "structural/adapter/adapter.h"

#include <iostream>

// TODO: 实现 Adapter 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 手里有不符合 Target 的已有对象（Adaptee）
//   2. 用 Adapter 包一层
//   3. 后续代码只调 Target，不感知 Adaptee

int main() {
  using namespace design_pattern::structural::adapter;

  // LegacyPlayer adaptee;
  // MediaPlayer& player = adapter;
  // player.play("song.mp3");

  std::cout << "[Adapter] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
