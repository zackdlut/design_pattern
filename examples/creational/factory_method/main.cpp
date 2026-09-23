#include "creational/factory_method/factory_method.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using design_pattern::creational::factory_method::ConcreteFactoryA;
using design_pattern::creational::factory_method::ConcreteFactoryB;
using design_pattern::creational::factory_method::Factory;
using design_pattern::creational::factory_method::Product;
using design_pattern::creational::factory_method::ProductType;
using design_pattern::creational::factory_method::SimpleFactory;

namespace {

// 内容平台的发布任务。渠道决定走哪条流水线，客户端不 new 具体产品。
struct PublishJob {
  std::string title;
  std::string channel; // "web" -> FactoryA，"app" -> FactoryB
};

std::unique_ptr<Factory> make_publisher(std::string_view channel) {
  if (channel == "web") {
    return std::make_unique<ConcreteFactoryA>();
  }
  if (channel == "app") {
    return std::make_unique<ConcreteFactoryB>();
  }
  throw std::invalid_argument("unknown channel: " + std::string(channel));
}

// 客户端只依赖 Factory 来创建。use() / show() 由调用方完成。
void publish(const Factory &pipeline, std::string_view title) {
  const auto product = pipeline.create();
  std::cout << "  publish \"" << title << "\" -> " << product->use() << " | "
            << product->show() << "\n";
}

ProductType parse_product_type(std::string_view name) {
  if (name == "A" || name == "csv") {
    return ProductType::A;
  }
  if (name == "B" || name == "json") {
    return ProductType::B;
  }
  throw std::invalid_argument("unknown product type: " + std::string(name));
}

} // namespace

int main() {
  std::cout << "=== 工厂方法：运营后台按渠道发布，不碰具体产品类 ===\n";

  const std::vector<PublishJob> jobs{
      {"daily-digest", "web"},
      {"push-card", "app"},
      {"weekend-special", "web"},
  };

  for (const auto &job : jobs) {
    auto pipeline = make_publisher(job.channel);
    std::cout << "[" << job.channel << "]\n";
    publish(*pipeline, job.title);
  }

  std::cout << "\n=== 同一抽象，换子类即换产品 ===\n";
  ConcreteFactoryA web;
  ConcreteFactoryB app;
  const Factory &as_web = web;
  const Factory &as_app = app;
  publish(as_web, "via Factory& (web)");
  publish(as_app, "via Factory& (app)");

  std::cout << "\n=== 对照：简单工厂按配置枚举创建，调用方同样自己 use/show ===\n";
  for (std::string_view type_name : {"csv", "json"}) {
    auto product = SimpleFactory::create(parse_product_type(type_name));
    std::cout << "  config=" << type_name << " -> " << product->use() << " | "
              << product->show() << "\n";
  }

  return 0;
}
