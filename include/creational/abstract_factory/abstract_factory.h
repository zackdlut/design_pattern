#pragma once

#include <memory>
#include <string>

namespace design_pattern::creational::abstract_factory {

class ProductA {
public:
  ProductA() = default;
  virtual ~ProductA() = default;
  ProductA(const ProductA &) = delete;
  ProductA &operator=(const ProductA &) = delete;
  ProductA(ProductA &&) = delete;
  ProductA &operator=(ProductA &&) = delete;

  virtual std::string paint() const = 0;
};

class StandardProductA final : public ProductA {
public:
  std::string paint() const override;
};

class ProProductA final : public ProductA {
public:
  std::string paint() const override;
};

class ProductB {
public:
  ProductB() = default;
  virtual ~ProductB() = default;
  ProductB(const ProductB &) = delete;
  ProductB &operator=(const ProductB &) = delete;
  ProductB(ProductB &&) = delete;
  ProductB &operator=(ProductB &&) = delete;

  virtual std::string paint() const = 0;
};

class StandardProductB final : public ProductB {
public:
  std::string paint() const override;
};

class ProProductB final : public ProductB {
public:
  std::string paint() const override;
};

class AbstractFactory {
public:
  AbstractFactory() = default;
  virtual ~AbstractFactory() = default;
  AbstractFactory(const AbstractFactory &) = delete;
  AbstractFactory &operator=(const AbstractFactory &) = delete;
  AbstractFactory(AbstractFactory &&) = delete;
  AbstractFactory &operator=(AbstractFactory &&) = delete;

  virtual std::unique_ptr<ProductA> createProductA() const = 0;
  virtual std::unique_ptr<ProductB> createProductB() const = 0;
};

class StandardVersionFactory final : public AbstractFactory {
public:
  std::unique_ptr<ProductA> createProductA() const override;
  std::unique_ptr<ProductB> createProductB() const override;
};

class ProVersionFactory final : public AbstractFactory {
public:
  std::unique_ptr<ProductA> createProductA() const override;
  std::unique_ptr<ProductB> createProductB() const override;
};

enum class Version { Standard, Pro };

// 对照：按配置选具体工厂。客户端拿到的仍是 AbstractFactory，不加产品不必改使用流程。
class VersionFactorySelector final {
public:
  VersionFactorySelector() = delete;
  static std::unique_ptr<AbstractFactory> create(Version version);
};

}  // namespace design_pattern::creational::abstract_factory
