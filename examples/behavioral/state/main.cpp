#include "behavioral/state/state.h"

#include <iostream>

// TODO: 实现 State 后，在这里写客户端用法。
//
// 典型客户端：
//   1. Context（播放器）把行为委托给当前 State
//   2. 调用 play / pause，内部切换 Ready / Playing
//   3. 客户端不写大段 if (state == ...)

int main() {
  using namespace design_pattern::behavioral::state;

  // player.play();   // Ready -> Playing
  // player.pause();  // Playing -> Paused

  std::cout << "[State] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
