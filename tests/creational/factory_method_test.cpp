#include "creational/factory_method/factory_method.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

using design_pattern::creational::factory_method::ConcreteCreatorA;
using design_pattern::creational::factory_method::ConcreteCreatorB;
using design_pattern::creational::factory_method::ConcreteProductA;
using design_pattern::creational::factory_method::ConcreteProductB;
using design_pattern::creational::factory_method::Creator;
using design_pattern::creational::factory_method::Product;
using design_pattern::creational::factory_method::ProductType;
using design_pattern::creational::factory_method::SimpleFactory;

TEST(FactoryMethodTest, CreatorACreatesProductA) {
  ConcreteCreatorA creator;
  std::unique_ptr<Product> product = creator.createProduct();
  ASSERT_NE(product, nullptr);
  EXPECT_NE(dynamic_cast<ConcreteProductA *>(product.get()), nullptr);
  EXPECT_EQ(dynamic_cast<ConcreteProductB *>(product.get()), nullptr);
  EXPECT_EQ(product->use(), "ConcreteProductA use");
  EXPECT_EQ(product->show(), "ConcreteProductA show");
}

TEST(FactoryMethodTest, CreatorBCreatesProductB) {
  ConcreteCreatorB creator;
  auto product = creator.createProduct();
  ASSERT_NE(product, nullptr);
  EXPECT_NE(dynamic_cast<ConcreteProductB *>(product.get()), nullptr);
  EXPECT_EQ(dynamic_cast<ConcreteProductA *>(product.get()), nullptr);
  EXPECT_EQ(product->use(), "ConcreteProductB use");
  EXPECT_EQ(product->show(), "ConcreteProductB show");
}

TEST(FactoryMethodTest, ProcessUsesProductFromFactoryMethod) {
  ConcreteCreatorA creator_a;
  ConcreteCreatorB creator_b;
  EXPECT_EQ(creator_a.process(), "ConcreteProductA use | ConcreteProductA show");
  EXPECT_EQ(creator_b.process(), "ConcreteProductB use | ConcreteProductB show");
}

TEST(FactoryMethodTest, ClientDependsOnCreatorAbstraction) {
  ConcreteCreatorA concrete;
  const Creator &creator = concrete;
  EXPECT_EQ(creator.process(), "ConcreteProductA use | ConcreteProductA show");
}

TEST(FactoryMethodTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<Product>);
  static_assert(!std::is_move_constructible_v<Product>);
  static_assert(!std::is_copy_assignable_v<Product>);
  static_assert(!std::is_move_assignable_v<Product>);
  static_assert(!std::is_copy_constructible_v<Creator>);
  static_assert(!std::is_move_constructible_v<Creator>);
  static_assert(!std::is_copy_constructible_v<ConcreteProductA>);
  static_assert(!std::is_copy_constructible_v<ConcreteCreatorA>);
  static_assert(!std::is_default_constructible_v<SimpleFactory>);
}

TEST(FactoryMethodTest, SimpleFactoryCreatesProductA) {
  auto product = SimpleFactory::createProduct(ProductType::A);
  ASSERT_NE(product, nullptr);
  EXPECT_NE(dynamic_cast<ConcreteProductA *>(product.get()), nullptr);
  EXPECT_EQ(dynamic_cast<ConcreteProductB *>(product.get()), nullptr);
  EXPECT_EQ(product->use(), "ConcreteProductA use");
  EXPECT_EQ(product->show(), "ConcreteProductA show");
}

TEST(FactoryMethodTest, SimpleFactoryCreatesProductB) {
  auto product = SimpleFactory::createProduct(ProductType::B);
  ASSERT_NE(product, nullptr);
  EXPECT_NE(dynamic_cast<ConcreteProductB *>(product.get()), nullptr);
  EXPECT_EQ(dynamic_cast<ConcreteProductA *>(product.get()), nullptr);
  EXPECT_EQ(product->use(), "ConcreteProductB use");
  EXPECT_EQ(product->show(), "ConcreteProductB show");
}

TEST(FactoryMethodTest, SimpleFactoryRejectsUnknownType) {
  EXPECT_THROW(SimpleFactory::createProduct(static_cast<ProductType>(99)), std::invalid_argument);
}
