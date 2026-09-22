#include "structural/proxy/proxy.h"

#include <iostream>

// TODO: 实现 Proxy 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 拿的是与 RealSubject 相同的接口
//   2. 第一次 display() 才真正加载（虚代理）
//   3. 也可以在代理里做权限检查、缓存、远程转发

int main() {
  using namespace design_pattern::structural::proxy;

  // Image& img = proxy;
  // img.display();  // lazy load / access check

  std::cout << "[Proxy] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
