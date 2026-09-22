#include "behavioral/template_method/template_method.h"

#include <iostream>

// TODO: 实现 Template Method 后，在这里写客户端用法。
//
// 典型客户端：
//   1. 基类固定 import 的步骤（读 / 解析 / 存）
//   2. 子类只覆盖变化的步骤
//   3. 客户端调 import()，不自己编排顺序

int main() {
  using namespace design_pattern::behavioral::template_method;

  // CsvImporter importer;
  // importer.import("data.csv");

  std::cout << "[Template Method] 客户端骨架：实现模式后在此补充真实用法。\n";
  return 0;
}
