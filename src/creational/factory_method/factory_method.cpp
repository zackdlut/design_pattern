#include "creational/factory_method/factory_method.h"

#include <stdexcept>

namespace design_pattern::creational::factory_method {

std::string ConcreteProductA::use() const { return "ConcreteProductA use"; }

std::string ConcreteProductA::show() const { return "ConcreteProductA show"; }

std::string ConcreteProductB::use() const { return "ConcreteProductB use"; }

std::string ConcreteProductB::show() const { return "ConcreteProductB show"; }

std::unique_ptr<Product> ConcreteFactoryA::create() const {
  return std::make_unique<ConcreteProductA>();
}

std::unique_ptr<Product> ConcreteFactoryB::create() const {
  return std::make_unique<ConcreteProductB>();
}

std::unique_ptr<Product> SimpleFactory::create(ProductType type) {
  switch (type) {
  case ProductType::A:
    return std::make_unique<ConcreteProductA>();
  case ProductType::B:
    return std::make_unique<ConcreteProductB>();
  }
  throw std::invalid_argument("unknown ProductType");
}

}  // namespace design_pattern::creational::factory_method
