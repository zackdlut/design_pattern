#include "creational/abstract_factory/abstract_factory.h"

#include <stdexcept>

namespace design_pattern::creational::abstract_factory {

std::string StandardProductA::paint() const { return "StandardProductA paint"; }

std::string ProProductA::paint() const { return "ProProductA paint"; }

std::string StandardProductB::paint() const { return "StandardProductB paint"; }

std::string ProProductB::paint() const { return "ProProductB paint"; }

std::unique_ptr<ProductA> StandardVersionFactory::createProductA() const {
  return std::make_unique<StandardProductA>();
}

std::unique_ptr<ProductB> StandardVersionFactory::createProductB() const {
  return std::make_unique<StandardProductB>();
}

std::unique_ptr<ProductA> ProVersionFactory::createProductA() const {
  return std::make_unique<ProProductA>();
}

std::unique_ptr<ProductB> ProVersionFactory::createProductB() const {
  return std::make_unique<ProProductB>();
}

std::unique_ptr<AbstractFactory> VersionFactorySelector::create(Version version) {
  switch (version) {
  case Version::Standard:
    return std::make_unique<StandardVersionFactory>();
  case Version::Pro:
    return std::make_unique<ProVersionFactory>();
  }
  throw std::invalid_argument("unknown Version");
}

}  // namespace design_pattern::creational::abstract_factory
