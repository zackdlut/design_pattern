#include "creational/abstract_factory/abstract_factory.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

using design_pattern::creational::abstract_factory::AbstractFactory;
using design_pattern::creational::abstract_factory::ProProductA;
using design_pattern::creational::abstract_factory::ProProductB;
using design_pattern::creational::abstract_factory::ProVersionFactory;
using design_pattern::creational::abstract_factory::ProductA;
using design_pattern::creational::abstract_factory::ProductB;
using design_pattern::creational::abstract_factory::StandardProductA;
using design_pattern::creational::abstract_factory::StandardProductB;
using design_pattern::creational::abstract_factory::StandardVersionFactory;
using design_pattern::creational::abstract_factory::Version;
using design_pattern::creational::abstract_factory::VersionFactorySelector;

namespace {

std::string render(const AbstractFactory &factory) {
  auto product_a = factory.createProductA();
  auto product_b = factory.createProductB();
  return product_a->paint() + " | " + product_b->paint();
}

} // namespace

TEST(AbstractFactoryTest, StandardVersionFactoryCreatesStandardFamily) {
  StandardVersionFactory factory;
  std::unique_ptr<ProductA> product_a = factory.createProductA();
  std::unique_ptr<ProductB> product_b = factory.createProductB();
  ASSERT_NE(product_a, nullptr);
  ASSERT_NE(product_b, nullptr);
  EXPECT_NE(dynamic_cast<StandardProductA *>(product_a.get()), nullptr);
  EXPECT_EQ(dynamic_cast<ProProductA *>(product_a.get()), nullptr);
  EXPECT_NE(dynamic_cast<StandardProductB *>(product_b.get()), nullptr);
  EXPECT_EQ(dynamic_cast<ProProductB *>(product_b.get()), nullptr);
  EXPECT_EQ(product_a->paint(), "StandardProductA paint");
  EXPECT_EQ(product_b->paint(), "StandardProductB paint");
}

TEST(AbstractFactoryTest, ProVersionFactoryCreatesProFamily) {
  ProVersionFactory factory;
  auto product_a = factory.createProductA();
  auto product_b = factory.createProductB();
  ASSERT_NE(product_a, nullptr);
  ASSERT_NE(product_b, nullptr);
  EXPECT_NE(dynamic_cast<ProProductA *>(product_a.get()), nullptr);
  EXPECT_EQ(dynamic_cast<StandardProductA *>(product_a.get()), nullptr);
  EXPECT_NE(dynamic_cast<ProProductB *>(product_b.get()), nullptr);
  EXPECT_EQ(dynamic_cast<StandardProductB *>(product_b.get()), nullptr);
  EXPECT_EQ(product_a->paint(), "ProProductA paint");
  EXPECT_EQ(product_b->paint(), "ProProductB paint");
}

TEST(AbstractFactoryTest, ClientDependsOnAbstractFactory) {
  StandardVersionFactory standard;
  ProVersionFactory pro;
  const AbstractFactory &as_standard = standard;
  const AbstractFactory &as_pro = pro;
  EXPECT_EQ(render(as_standard), "StandardProductA paint | StandardProductB paint");
  EXPECT_EQ(render(as_pro), "ProProductA paint | ProProductB paint");
}

TEST(AbstractFactoryTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<ProductA>);
  static_assert(!std::is_move_constructible_v<ProductA>);
  static_assert(!std::is_copy_assignable_v<ProductA>);
  static_assert(!std::is_move_assignable_v<ProductA>);
  static_assert(!std::is_copy_constructible_v<ProductB>);
  static_assert(!std::is_move_constructible_v<ProductB>);
  static_assert(!std::is_copy_constructible_v<AbstractFactory>);
  static_assert(!std::is_move_constructible_v<AbstractFactory>);
  static_assert(!std::is_copy_constructible_v<StandardProductA>);
  static_assert(!std::is_copy_constructible_v<StandardVersionFactory>);
  static_assert(!std::is_default_constructible_v<VersionFactorySelector>);
}

TEST(AbstractFactoryTest, VersionFactorySelectorCreatesStandardFamily) {
  auto factory = VersionFactorySelector::create(Version::Standard);
  ASSERT_NE(factory, nullptr);
  EXPECT_NE(dynamic_cast<StandardVersionFactory *>(factory.get()), nullptr);
  EXPECT_EQ(dynamic_cast<ProVersionFactory *>(factory.get()), nullptr);
  EXPECT_EQ(render(*factory), "StandardProductA paint | StandardProductB paint");
}

TEST(AbstractFactoryTest, VersionFactorySelectorCreatesProFamily) {
  auto factory = VersionFactorySelector::create(Version::Pro);
  ASSERT_NE(factory, nullptr);
  EXPECT_NE(dynamic_cast<ProVersionFactory *>(factory.get()), nullptr);
  EXPECT_EQ(dynamic_cast<StandardVersionFactory *>(factory.get()), nullptr);
  EXPECT_EQ(render(*factory), "ProProductA paint | ProProductB paint");
}

TEST(AbstractFactoryTest, VersionFactorySelectorRejectsUnknownVersion) {
  EXPECT_THROW(VersionFactorySelector::create(static_cast<Version>(99)),
               std::invalid_argument);
}
