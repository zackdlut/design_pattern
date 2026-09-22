#include "structural/composite/composite.h"

#include <iostream>

// TODO: 实现 Composite 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 叶子（File）和容器（Folder）共用 Component 接口
//   2. 往文件夹里 add 文件或子文件夹
//   3. 对根节点一次 draw() / size()，递归作用到整棵树

int main() {
  using namespace design_pattern::structural::composite;

  // Folder root;
  // root.add(file);
  // root.add(subfolder);
  // root.draw();

  std::cout << "[Composite] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
