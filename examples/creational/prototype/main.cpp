#include "creational/prototype/prototype.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using design_pattern::creational::prototype::Mage;
using design_pattern::creational::prototype::Prototype;
using design_pattern::creational::prototype::PrototypeRegistry;
using design_pattern::creational::prototype::UnitSpec;
using design_pattern::creational::prototype::Warrior;

namespace {

// 客户端只依赖 Prototype：clone 保留具体类型，再改副本上的差异字段。
std::unique_ptr<Prototype> spawn_at(const Prototype &prototype, int x, int y,
                                    std::string_view instance_name) {
  auto unit = prototype.clone();
  unit->setName(instance_name);
  unit->setPosition(x, y);
  return unit;
}

}  // namespace

int main() {
  std::cout << "=== 原型：兵营按模板克隆出兵，再改名字和位置 ===\n";

  PrototypeRegistry barracks;
  barracks.registerPrototype("grunt", std::make_unique<Warrior>(
                            "grunt", 100, 20, std::vector<std::string>{"slash"}));
  barracks.registerPrototype("captain",
               std::make_unique<Warrior>(
                   "captain", 180, 35,
                   std::vector<std::string>{"slash", "rally"}));
  barracks.registerPrototype("apprentice",
               std::make_unique<Mage>("apprentice", 50, 40,
                                      std::vector<std::string>{"spark"}));

  auto grunt_a = barracks.create("grunt");
  auto grunt_b = barracks.create("grunt");
  grunt_a->setName("grunt#1");
  grunt_a->setPosition(3, 1);
  grunt_b->setName("grunt#2");
  grunt_b->setPosition(4, 1);
  std::cout << "[grunt]\n";
  std::cout << "  template still: " << barracks.create("grunt")->describe()
            << "\n";
  std::cout << "  spawn #1:       " << grunt_a->describe() << "\n";
  std::cout << "  spawn #2:       " << grunt_b->describe() << "\n";

  auto captain = barracks.create("captain");
  captain->setPosition(1, 0);
  auto mage = barracks.create("apprentice");
  mage->setName("merlin-jr");
  mage->setPosition(8, 2);
  std::cout << "[other templates]\n";
  std::cout << "  " << captain->describe() << "\n";
  std::cout << "  " << mage->describe() << "\n";

  std::cout << "\n=== 同一抽象 Prototype&，clone() 保留具体类型 ===\n";
  Warrior grunt_template("grunt", 100, 20, {"slash"});
  Mage mage_template("apprentice", 50, 40, {"spark"});
  const Prototype &as_warrior = grunt_template;
  const Prototype &as_mage = mage_template;
  std::cout << "  via Prototype& (warrior): "
            << spawn_at(as_warrior, 3, 1, "grunt#3")->describe() << "\n";
  std::cout << "  via Prototype& (mage):    "
            << spawn_at(as_mage, 8, 2, "spark-1")->describe() << "\n";

  std::cout << "\n=== 对照：已知具体类型时，值对象直接拷贝 ===\n";
  UnitSpec spec("warrior", "grunt", 100);
  UnitSpec left = spec;
  UnitSpec right = spec;
  left.setName("grunt-L");
  left.setPosition(3, 1);
  right.setName("grunt-R");
  right.setPosition(4, 1);
  std::cout << "  template: " << spec.describe() << "\n";
  std::cout << "  copy L:   " << left.describe() << "\n";
  std::cout << "  copy R:   " << right.describe() << "\n";

  std::cout << "\n=== 错误处理：登记时验模板，create() 再查名字 ===\n";
  try {
    barracks.registerPrototype("grunt", std::make_unique<Warrior>("other", 80, 15));
  } catch (const std::invalid_argument &error) {
    std::cout << "  registry: " << error.what() << "\n";
  }
  try {
    (void)barracks.create("dragon");
  } catch (const std::invalid_argument &error) {
    std::cout << "  spawn:    " << error.what() << "\n";
  }

  return 0;
}
