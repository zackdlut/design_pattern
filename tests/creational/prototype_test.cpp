#include "creational/prototype/prototype.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using design_pattern::creational::prototype::Mage;
using design_pattern::creational::prototype::Prototype;
using design_pattern::creational::prototype::PrototypeRegistry;
using design_pattern::creational::prototype::UnitSpec;
using design_pattern::creational::prototype::Warrior;

namespace {

void expect_invalid(const char *message, auto &&action) {
  try {
    action();
    FAIL() << "expected std::invalid_argument: " << message;
  } catch (const std::invalid_argument &error) {
    EXPECT_STREQ(error.what(), message);
  }
}

std::unique_ptr<Prototype> spawn_at(const Prototype &prototype, int x, int y) {
  auto unit = prototype.clone();
  unit->setPosition(x, y);
  return unit;
}

}  // namespace

TEST(PrototypeTest, WarriorClonePreservesTypeAndState) {
  Warrior grunt("grunt", 100, 20, {"slash", "block"});
  std::unique_ptr<Prototype> copy = grunt.clone();

  ASSERT_NE(copy, nullptr);
  EXPECT_NE(copy.get(), static_cast<Prototype *>(&grunt));
  EXPECT_NE(dynamic_cast<Warrior *>(copy.get()), nullptr);
  EXPECT_EQ(dynamic_cast<Mage *>(copy.get()), nullptr);
  EXPECT_EQ(copy->kind(), "Warrior");
  EXPECT_EQ(copy->name(), "grunt");
  EXPECT_EQ(copy->x(), 0);
  EXPECT_EQ(copy->y(), 0);
  EXPECT_EQ(copy->describe(),
            "Warrior grunt hp=100 atk=20 skills=slash,block @ (0,0)");

  const auto *as_warrior = dynamic_cast<const Warrior *>(copy.get());
  ASSERT_NE(as_warrior, nullptr);
  EXPECT_EQ(as_warrior->hp(), 100);
  EXPECT_EQ(as_warrior->attack(), 20);
  ASSERT_EQ(as_warrior->skills().size(), 2U);
  EXPECT_EQ(as_warrior->skills()[0], "slash");
  EXPECT_EQ(as_warrior->skills()[1], "block");
}

TEST(PrototypeTest, MageClonePreservesTypeAndState) {
  Mage merlin("merlin", 60, 80, {"fireball", "frost"}, 2, 3);
  auto copy = merlin.clone();

  ASSERT_NE(copy, nullptr);
  EXPECT_NE(dynamic_cast<Mage *>(copy.get()), nullptr);
  EXPECT_EQ(dynamic_cast<Warrior *>(copy.get()), nullptr);
  EXPECT_EQ(copy->kind(), "Mage");
  EXPECT_EQ(copy->describe(),
            "Mage merlin hp=60 mana=80 spells=fireball,frost @ (2,3)");

  const auto *as_mage = dynamic_cast<const Mage *>(copy.get());
  ASSERT_NE(as_mage, nullptr);
  EXPECT_EQ(as_mage->hp(), 60);
  EXPECT_EQ(as_mage->mana(), 80);
  ASSERT_EQ(as_mage->spells().size(), 2U);
  EXPECT_EQ(as_mage->spells()[0], "fireball");
}

TEST(PrototypeTest, CloneIsIndependentOfOriginal) {
  Warrior grunt("grunt", 100, 20, {"slash"});
  auto copy = grunt.clone();
  copy->setName("grunt#2");
  copy->setPosition(4, 1);
  dynamic_cast<Warrior *>(copy.get())->addSkill("block");

  EXPECT_EQ(grunt.name(), "grunt");
  EXPECT_EQ(grunt.x(), 0);
  EXPECT_EQ(grunt.y(), 0);
  EXPECT_EQ(grunt.skills(), (std::vector<std::string>{"slash"}));
  EXPECT_EQ(grunt.describe(), "Warrior grunt hp=100 atk=20 skills=slash @ (0,0)");

  EXPECT_EQ(copy->name(), "grunt#2");
  EXPECT_EQ(copy->x(), 4);
  EXPECT_EQ(copy->y(), 1);
  EXPECT_EQ(copy->describe(),
            "Warrior grunt#2 hp=100 atk=20 skills=slash,block @ (4,1)");
}

TEST(PrototypeTest, ClientDependsOnPrototypeAbstraction) {
  Warrior grunt("grunt", 100, 20, {"slash"});
  Mage apprentice("apprentice", 50, 40, {"spark"});
  const Prototype &as_warrior = grunt;
  const Prototype &as_mage = apprentice;

  auto spawned_warrior = spawn_at(as_warrior, 3, 1);
  auto spawned_mage = spawn_at(as_mage, 8, 2);

  EXPECT_EQ(spawned_warrior->kind(), "Warrior");
  EXPECT_EQ(spawned_warrior->describe(),
            "Warrior grunt hp=100 atk=20 skills=slash @ (3,1)");
  EXPECT_EQ(spawned_mage->kind(), "Mage");
  EXPECT_EQ(spawned_mage->describe(),
            "Mage apprentice hp=50 mana=40 spells=spark @ (8,2)");
}

TEST(PrototypeTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<Prototype>);
  static_assert(!std::is_move_constructible_v<Prototype>);
  static_assert(!std::is_copy_assignable_v<Prototype>);
  static_assert(!std::is_move_assignable_v<Prototype>);
  static_assert(!std::is_copy_constructible_v<Warrior>);
  static_assert(!std::is_copy_constructible_v<Mage>);
  static_assert(!std::is_copy_constructible_v<PrototypeRegistry>);
  static_assert(!std::is_move_constructible_v<PrototypeRegistry>);
  static_assert(std::is_copy_constructible_v<UnitSpec>);
  static_assert(std::is_copy_assignable_v<UnitSpec>);
}

TEST(PrototypeTest, RegistryCreatesIndependentClones) {
  PrototypeRegistry barracks;
  barracks.registerPrototype("grunt",
               std::make_unique<Warrior>("grunt", 100, 20, std::vector<std::string>{"slash"}));
  barracks.registerPrototype("apprentice",
               std::make_unique<Mage>("apprentice", 50, 40, std::vector<std::string>{"spark"}));

  EXPECT_TRUE(barracks.contains("grunt"));
  EXPECT_TRUE(barracks.contains("apprentice"));
  EXPECT_FALSE(barracks.contains("dragon"));

  auto first = barracks.create("grunt");
  auto second = barracks.create("grunt");
  first->setPosition(3, 1);
  second->setPosition(4, 1);
  first->setName("grunt#1");

  EXPECT_NE(first.get(), second.get());
  EXPECT_EQ(first->describe(),
            "Warrior grunt#1 hp=100 atk=20 skills=slash @ (3,1)");
  EXPECT_EQ(second->describe(),
            "Warrior grunt hp=100 atk=20 skills=slash @ (4,1)");
  EXPECT_EQ(barracks.create("apprentice")->describe(),
            "Mage apprentice hp=50 mana=40 spells=spark @ (0,0)");
}

TEST(PrototypeTest, RegistryCreateKeepsTemplateUnchanged) {
  PrototypeRegistry barracks;
  barracks.registerPrototype("captain", std::make_unique<Warrior>("captain", 180, 35,
                                                    std::vector<std::string>{"slash", "rally"}));

  auto spawned = barracks.create("captain");
  spawned->setName("captain#7");
  spawned->setPosition(9, 9);
  dynamic_cast<Warrior *>(spawned.get())->addSkill("shield-wall");

  EXPECT_EQ(barracks.create("captain")->describe(),
            "Warrior captain hp=180 atk=35 skills=slash,rally @ (0,0)");
}

TEST(PrototypeTest, UnitSpecCopiesByValue) {
  UnitSpec template_spec("warrior", "grunt", 100);
  UnitSpec spawned = template_spec;
  spawned.setName("grunt#2");
  spawned.setPosition(4, 1);

  EXPECT_EQ(template_spec.describe(), "UnitSpec warrior grunt hp=100 @ (0,0)");
  EXPECT_EQ(spawned.describe(), "UnitSpec warrior grunt#2 hp=100 @ (4,1)");
  EXPECT_EQ(template_spec.kind(), "warrior");
  EXPECT_EQ(template_spec.name(), "grunt");
  EXPECT_EQ(spawned.x(), 4);
  EXPECT_EQ(spawned.y(), 1);
}

TEST(PrototypeTest, ConstructorsRejectInvalidFields) {
  expect_invalid("name is required", [] { Warrior("", 100, 20); });
  expect_invalid("hp must be positive", [] { Warrior("grunt", 0, 20); });
  expect_invalid("attack must be positive", [] { Warrior("grunt", 100, 0); });
  expect_invalid("skill is required",
                 [] { Warrior("grunt", 100, 20, {""}); });

  expect_invalid("name is required", [] { Mage("", 60, 80); });
  expect_invalid("hp must be positive", [] { Mage("merlin", -1, 80); });
  expect_invalid("mana must be positive", [] { Mage("merlin", 60, 0); });
  expect_invalid("spell is required", [] { Mage("merlin", 60, 80, {""}); });

  expect_invalid("kind is required", [] { UnitSpec("", "grunt", 100); });
  expect_invalid("name is required", [] { UnitSpec("warrior", "", 100); });
  expect_invalid("hp must be positive", [] { UnitSpec("warrior", "grunt", 0); });
}

TEST(PrototypeTest, SettersRejectInvalidFields) {
  expect_invalid("name is required", [] { Warrior("grunt", 100, 20).setName(""); });
  expect_invalid("skill is required",
                 [] { Warrior("grunt", 100, 20).addSkill(""); });
  expect_invalid("name is required", [] { Mage("merlin", 60, 80).setName(""); });
  expect_invalid("spell is required", [] { Mage("merlin", 60, 80).addSpell(""); });
  expect_invalid("name is required",
                 [] { UnitSpec("warrior", "grunt", 100).setName(""); });
}

TEST(PrototypeTest, RegistryRejectsInvalidOperations) {
  PrototypeRegistry barracks;
  expect_invalid("prototype name is required", [&] {
    barracks.registerPrototype("", std::make_unique<Warrior>("grunt", 100, 20));
  });
  expect_invalid("prototype is required",
                 [&] { barracks.registerPrototype("grunt", nullptr); });

  barracks.registerPrototype("grunt", std::make_unique<Warrior>("grunt", 100, 20));
  expect_invalid("duplicate prototype: grunt", [&] {
    barracks.registerPrototype("grunt", std::make_unique<Warrior>("other", 80, 15));
  });
  expect_invalid("unknown prototype: dragon",
                 [&] { (void)barracks.create("dragon"); });
}
