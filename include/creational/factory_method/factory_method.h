#pragma once

#include <memory>
#include <string>

namespace design_pattern::creational::factory_method {

class Product {
public:
  Product() = default;
  virtual ~Product() = default;
  Product(const Product &) = delete;
  Product &operator=(const Product &) = delete;
  Product(Product &&) = delete;
  Product &operator=(Product &&) = delete;

  virtual std::string use() const = 0;
  virtual std::string show() const = 0;
};

class ConcreteProductA final : public Product {
public:
  std::string use() const override;
  std::string show() const override;
};

class ConcreteProductB final : public Product {
public:
  std::string use() const override;
  std::string show() const override;
};

class Factory {
public:
  Factory() = default;
  virtual ~Factory() = default;
  Factory(const Factory &) = delete;
  Factory &operator=(const Factory &) = delete;
  Factory(Factory &&) = delete;
  Factory &operator=(Factory &&) = delete;

  // 只负责创建。造哪一种由子类的 create() 决定，使用留在调用方。
  virtual std::unique_ptr<Product> create() const = 0;
};

class ConcreteFactoryA final : public Factory {
public:
  std::unique_ptr<Product> create() const override;
};

class ConcreteFactoryB final : public Factory {
public:
  std::unique_ptr<Product> create() const override;
};

enum class ProductType { A, B };

// 对照：简单工厂。不继承 Factory，加产品必须改这个函数。
class SimpleFactory final {
public:
  SimpleFactory() = delete;
  static std::unique_ptr<Product> create(ProductType type);
};

}  // namespace design_pattern::creational::factory_method
