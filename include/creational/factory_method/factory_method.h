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

class Creator {
public:
  Creator() = default;
  virtual ~Creator() = default;
  Creator(const Creator &) = delete;
  Creator &operator=(const Creator &) = delete;
  Creator(Creator &&) = delete;
  Creator &operator=(Creator &&) = delete;

  // 稳定业务流程：变化点只在子类的 createProduct()。
  std::string process() const;
  virtual std::unique_ptr<Product> createProduct() const = 0;
};

class ConcreteCreatorA final : public Creator {
public:
  std::unique_ptr<Product> createProduct() const override;
};

class ConcreteCreatorB final : public Creator {
public:
  std::unique_ptr<Product> createProduct() const override;
};

enum class ProductType { A, B };

// 对照：简单工厂。不继承 Creator，没有 process()，加产品必须改这个函数。
class SimpleFactory final {
public:
  SimpleFactory() = delete;
  static std::unique_ptr<Product> createProduct(ProductType type);
};

}  // namespace design_pattern::creational::factory_method
