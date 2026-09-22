#include "creational/builder/builder.h"

#include <iostream>

// TODO: 实现 Builder 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 用链式调用填可选字段（method / url / header）
//   2. 最后 build() 得到不可变产品
//   3. 不把 10 个参数塞进一个构造函数

int main() {
  using namespace design_pattern::creational::builder;

  // HttpRequest req = HttpRequestBuilder()
  //     .method("POST")
  //     .url("/login")
  //     .header("Authorization", token)
  //     .build();

  std::cout << "[Builder] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
