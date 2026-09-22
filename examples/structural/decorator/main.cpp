#include "structural/decorator/decorator.h"

#include <iostream>

// TODO: 实现 Decorator 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 从核心对象（FileStream）开始
//   2. 按需叠 Buffered / Encrypted 等装饰器
//   3. 仍通过统一的 Stream 接口 write / read

int main() {
  using namespace design_pattern::structural::decorator;

  // std::unique_ptr<Stream> s = std::make_unique<FileStream>("a.txt");
  // s = std::make_unique<BufferedStream>(std::move(s));
  // s = std::make_unique<EncryptedStream>(std::move(s));
  // s->write(data);

  std::cout << "[Decorator] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
