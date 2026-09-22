#include "behavioral/chain_of_responsibility/chain_of_responsibility.h"

#include <iostream>

// TODO: 实现 Chain of Responsibility 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 把 Auth / RateLimit / Business 串成链
//   2. 只把请求交给链头
//   3. 谁处理、谁往后传，客户端不管

int main() {
  using namespace design_pattern::behavioral::chain_of_responsibility;

  // auth.set_next(rate_limit).set_next(handler);
  // auth.handle(request);

  std::cout << "[Chain of Responsibility] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
