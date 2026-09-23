#include "creational/abstract_factory/abstract_factory.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using design_pattern::creational::abstract_factory::AbstractFactory;
using design_pattern::creational::abstract_factory::ProVersionFactory;
using design_pattern::creational::abstract_factory::StandardVersionFactory;
using design_pattern::creational::abstract_factory::Version;
using design_pattern::creational::abstract_factory::VersionFactorySelector;

namespace {

struct ReleaseJob {
  std::string screen;
  std::string version; // "standard" -> Standard 族，"pro" -> Pro 族
};

std::unique_ptr<AbstractFactory> make_version_factory(std::string_view version) {
  if (version == "standard") {
    return std::make_unique<StandardVersionFactory>();
  }
  if (version == "pro") {
    return std::make_unique<ProVersionFactory>();
  }
  throw std::invalid_argument("unknown version: " + std::string(version));
}

Version parse_version(std::string_view name) {
  if (name == "standard") {
    return Version::Standard;
  }
  if (name == "pro") {
    return Version::Pro;
  }
  throw std::invalid_argument("unknown version: " + std::string(name));
}

// 客户端只依赖 AbstractFactory：换工厂即换整族产品，不会混用标准版 ProductA + 专业版 ProductB。
void render_dialog(const AbstractFactory &factory, std::string_view screen) {
  auto product_a = factory.createProductA();
  auto product_b = factory.createProductB();
  std::cout << "  [" << screen << "] " << product_a->paint() << " | "
            << product_b->paint() << "\n";
}

} // namespace

int main() {
  std::cout << "=== 抽象工厂：按版本组装产品族，ProductA 与 ProductB 必须同族 ===\n";

  const std::vector<ReleaseJob> jobs{
      {"login", "standard"},
      {"settings", "pro"},
      {"about", "standard"},
  };

  for (const auto &job : jobs) {
    auto factory = make_version_factory(job.version);
    std::cout << "[" << job.version << "]\n";
    render_dialog(*factory, job.screen);
  }

  std::cout << "\n=== 同一抽象，换具体工厂即换产品族 ===\n";
  StandardVersionFactory standard;
  ProVersionFactory pro;
  const AbstractFactory &as_standard = standard;
  const AbstractFactory &as_pro = pro;
  render_dialog(as_standard, "via AbstractFactory& (standard)");
  render_dialog(as_pro, "via AbstractFactory& (pro)");

  std::cout << "\n=== 对照：按配置枚举选工厂，使用流程仍然只依赖抽象 ===\n";
  for (std::string_view version_name : {"standard", "pro"}) {
    auto factory = VersionFactorySelector::create(parse_version(version_name));
    std::cout << "  config=" << version_name << "\n";
    render_dialog(*factory, "via VersionFactorySelector");
  }

  return 0;
}
