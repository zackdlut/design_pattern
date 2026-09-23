#include "creational/factory_method/factory_method.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

using design_pattern::creational::factory_method::ConcreteFactoryA;
using design_pattern::creational::factory_method::ConcreteFactoryB;
using design_pattern::creational::factory_method::ConcreteProductA;
using design_pattern::creational::factory_method::ConcreteProductB;
using design_pattern::creational::factory_method::Factory;
using design_pattern::creational::factory_method::Product;
using design_pattern::creational::factory_method::ProductType;
using design_pattern::creational::factory_method::SimpleFactory;

TEST(FactoryMethodTest, FactoryACreatesProductA) {
  ConcreteFactoryA factory;
  std::unique_ptr<Product> product = factory.create();
  ASSERT_NE(product, nullptr);
  EXPECT_NE(dynamic_cast<ConcreteProductA *>(product.get()), nullptr);
  EXPECT_EQ(dynamic_cast<ConcreteProductB *>(product.get()), nullptr);
  EXPECT_EQ(product->use(), "ConcreteProductA use");
  EXPECT_EQ(product->show(), "ConcreteProductA show");
}

TEST(FactoryMethodTest, FactoryBCreatesProductB) {
  ConcreteFactoryB factory;
  auto product = factory.create();
  ASSERT_NE(product, nullptr);
  EXPECT_NE(dynamic_cast<ConcreteProductB *>(product.get()), nullptr);
  EXPECT_EQ(dynamic_cast<ConcreteProductA *>(product.get()), nullptr);
  EXPECT_EQ(product->use(), "ConcreteProductB use");
  EXPECT_EQ(product->show(), "ConcreteProductB show");
}

TEST(FactoryMethodTest, ClientUsesProductAfterCreate) {
  ConcreteFactoryA factory_a;
  ConcreteFactoryB factory_b;
  auto product_a = factory_a.create();
  auto product_b = factory_b.create();
  ASSERT_NE(product_a, nullptr);
  ASSERT_NE(product_b, nullptr);
  EXPECT_EQ(product_a->use() + " | " + product_a->show(),
            "ConcreteProductA use | ConcreteProductA show");
  EXPECT_EQ(product_b->use() + " | " + product_b->show(),
            "ConcreteProductB use | ConcreteProductB show");
}

TEST(FactoryMethodTest, ClientDependsOnFactoryAbstraction) {
  ConcreteFactoryA concrete;
  const Factory &factory = concrete;
  auto product = factory.create();
  ASSERT_NE(product, nullptr);
  EXPECT_NE(dynamic_cast<ConcreteProductA *>(product.get()), nullptr);
  EXPECT_EQ(product->use(), "ConcreteProductA use");
  EXPECT_EQ(product->show(), "ConcreteProductA show");
}

TEST(FactoryMethodTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<Product>);
  static_assert(!std::is_move_constructible_v<Product>);
  static_assert(!std::is_copy_assignable_v<Product>);
  static_assert(!std::is_move_assignable_v<Product>);
  static_assert(!std::is_copy_constructible_v<Factory>);
  static_assert(!std::is_move_constructible_v<Factory>);
  static_assert(!std::is_copy_constructible_v<ConcreteProductA>);
  static_assert(!std::is_copy_constructible_v<ConcreteFactoryA>);
  static_assert(!std::is_default_constructible_v<SimpleFactory>);
}

TEST(FactoryMethodTest, SimpleFactoryCreatesProductA) {
  auto product = SimpleFactory::create(ProductType::A);
  ASSERT_NE(product, nullptr);
  EXPECT_NE(dynamic_cast<ConcreteProductA *>(product.get()), nullptr);
  EXPECT_EQ(dynamic_cast<ConcreteProductB *>(product.get()), nullptr);
  EXPECT_EQ(product->use(), "ConcreteProductA use");
  EXPECT_EQ(product->show(), "ConcreteProductA show");
}

TEST(FactoryMethodTest, SimpleFactoryCreatesProductB) {
  auto product = SimpleFactory::create(ProductType::B);
  ASSERT_NE(product, nullptr);
  EXPECT_NE(dynamic_cast<ConcreteProductB *>(product.get()), nullptr);
  EXPECT_EQ(dynamic_cast<ConcreteProductA *>(product.get()), nullptr);
  EXPECT_EQ(product->use(), "ConcreteProductB use");
  EXPECT_EQ(product->show(), "ConcreteProductB show");
}

TEST(FactoryMethodTest, SimpleFactoryRejectsUnknownType) {
  EXPECT_THROW(SimpleFactory::create(static_cast<ProductType>(99)), std::invalid_argument);
}
