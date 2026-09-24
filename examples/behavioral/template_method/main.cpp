#include "behavioral/template_method/template_method.h"

#include <iostream>

using design_pattern::behavioral::template_method::MethodB;
using design_pattern::behavioral::template_method::MethodA;

int main() {
  std::cout << "=== execute 先 common 再 unique，子类只换 unique ===\n";
  MethodA a;
  MethodB b;
  std::cout << "  " << a.execute() << "\n";
  std::cout << "  " << b.execute() << "\n";

  return 0;
}
