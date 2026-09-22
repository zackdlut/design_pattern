#include "structural/facade/facade.h"

#include <iostream>

// TODO: 实现 Facade 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 不直接编排编解码、滤镜、写文件等子系统
//   2. 只调外观上的一两个方法
//   3. 复杂步骤藏在 Facade 内部

int main() {
  using namespace design_pattern::structural::facade;

  // VideoConverter converter;
  // converter.convert("demo.mp4", "mp3");

  std::cout << "[Facade] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
