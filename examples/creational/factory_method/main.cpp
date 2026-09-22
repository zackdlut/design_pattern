#include "creational/factory_method/factory_method.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using design_pattern::creational::factory_method::ConcreteCreatorA;
using design_pattern::creational::factory_method::ConcreteCreatorB;
using design_pattern::creational::factory_method::Creator;
using design_pattern::creational::factory_method::Product;
using design_pattern::creational::factory_method::ProductType;
using design_pattern::creational::factory_method::SimpleFactory;

namespace {

// 内容平台的发布任务。渠道决定走哪条流水线，客户端不 new 具体产品。
struct PublishJob {
  std::string title;
  std::string channel; // "web" -> CreatorA，"app" -> CreatorB
};

std::unique_ptr<Creator> make_publisher(std::string_view channel) {
  if (channel == "web") {
    return std::make_unique<ConcreteCreatorA>();
  }
  if (channel == "app") {
    return std::make_unique<ConcreteCreatorB>();
  }
  throw std::invalid_argument("unknown channel: " + std::string(channel));
}

// 客户端只依赖 Creator：稳定流程在 process()，产品类型由子类决定。
void publish(const Creator &pipeline, std::string_view title) {
  std::cout << "  publish \"" << title << "\" -> " << pipeline.process() << "\n";
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
  ConcreteCreatorA web;
  ConcreteCreatorB app;
  const Creator &as_web = web;
  const Creator &as_app = app;
  publish(as_web, "via Creator& (web)");
  publish(as_app, "via Creator& (app)");

  std::cout << "\n=== 对照：简单工厂按配置枚举创建，调用方自己 use/show ===\n";
  for (std::string_view type_name : {"csv", "json"}) {
    auto product = SimpleFactory::createProduct(parse_product_type(type_name));
    std::cout << "  config=" << type_name << " -> " << product->use() << " | "
              << product->show() << "\n";
  }

  return 0;
}
