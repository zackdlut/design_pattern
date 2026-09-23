#include "structural/flyweight/flyweight.h"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <type_traits>

using design_pattern::structural::flyweight::Forest;
using design_pattern::structural::flyweight::InlineTree;
using design_pattern::structural::flyweight::PlantedTree;
using design_pattern::structural::flyweight::Shrine;
using design_pattern::structural::flyweight::TreeFactory;
using design_pattern::structural::flyweight::TreeGlyph;
using design_pattern::structural::flyweight::TreeType;

namespace {

void expect_invalid(const char *message, auto &&action) {
  try {
    action();
    FAIL() << "expected std::invalid_argument: " << message;
  } catch (const std::invalid_argument &error) {
    EXPECT_STREQ(error.what(), message);
  }
}

void expect_range(auto &&action) {
  try {
    action();
    FAIL() << "expected std::out_of_range: tree index out of range";
  } catch (const std::out_of_range &error) {
    EXPECT_STREQ(error.what(), "tree index out of range");
  }
}

std::string paint(const TreeGlyph &glyph, int x, int y) {
  return glyph.draw(x, y);
}

const char *kOakAt12 = "oak (green) [bark] @ (1,2)";
const char *kOakAt34 = "oak (green) [bark] @ (3,4)";
const char *kPineAt85 = "pine (dark) [needle] @ (8,5)";
const char *kShrine = "shrine [ancient well] @ (0,1)";

template <typename T>
constexpr bool has_set_color = requires(T &value) { value.set_color("yellow"); };

template <typename T>
constexpr bool has_draw_at = requires(const T &value) { value.draw(0, 0); };

template <typename T>
constexpr bool has_color = requires(const T &value) { value.color(); };

template <typename Factory>
constexpr bool can_create_type = requires(Factory &factory) {
  factory.get("oak", "green", "bark");
};

template <typename Factory>
constexpr bool can_lookup_type = requires(Factory &factory) {
  factory.get("oak");
};

template <typename Factory>
constexpr bool can_query_pool = requires(Factory &factory) {
  factory.contains("oak");
  factory.size();
};

}  // namespace

TEST(FlyweightTest, TreeTypeDrawCombinesIntrinsicAndExtrinsic) {
  TreeType oak("oak", "green", "bark");

  EXPECT_EQ(oak.name(), "oak");
  EXPECT_EQ(oak.color(), "green");
  EXPECT_EQ(oak.texture(), "bark");
  EXPECT_EQ(oak.draw(1, 2), kOakAt12);
  EXPECT_EQ(oak.draw(3, 4), kOakAt34);
  EXPECT_EQ(oak.draw(-2, 0), "oak (green) [bark] @ (-2,0)");
  EXPECT_NE(oak.draw(1, 2), oak.draw(3, 4));
}

TEST(FlyweightTest, DirectTreeTypesWithTheSameDataAreDistinctObjects) {
  TreeType first("oak", "green", "bark");
  TreeType second("oak", "green", "bark");

  EXPECT_NE(&first, &second);
  EXPECT_EQ(first.draw(1, 2), second.draw(1, 2));
}

TEST(FlyweightTest, ShrineDrawsItsOwnInscription) {
  Shrine well("ancient well");

  EXPECT_EQ(well.name(), "shrine");
  EXPECT_EQ(well.inscription(), "ancient well");
  EXPECT_EQ(well.draw(0, 1), kShrine);
  EXPECT_EQ(well.draw(2, 2), "shrine [ancient well] @ (2,2)");
}

TEST(FlyweightTest, ConstructorsRejectInvalidFields) {
  expect_invalid("species is required", [] { TreeType("", "green", "bark"); });
  expect_invalid("color is required", [] { TreeType("oak", "", "bark"); });
  expect_invalid("texture is required", [] { TreeType("oak", "green", ""); });
  expect_invalid("inscription is required", [] { Shrine(""); });
  expect_invalid("species is required",
                [] { InlineTree(1, 2, "", "green", "bark"); });
  expect_invalid("color is required",
                [] { InlineTree(1, 2, "oak", "", "bark"); });
  expect_invalid("texture is required",
                [] { InlineTree(1, 2, "oak", "green", ""); });

  InlineTree oak(1, 2, "oak", "green", "bark");
  expect_invalid("color is required", [&] { oak.set_color(""); });
  EXPECT_EQ(oak.color(), "green");
  EXPECT_EQ(oak.draw(), kOakAt12);
}

TEST(FlyweightTest, FactorySharesOneObjectPerSpecies) {
  TreeFactory nursery;
  EXPECT_EQ(nursery.size(), 0);
  EXPECT_FALSE(nursery.contains("oak"));

  const TreeType &oak = nursery.get("oak", "green", "bark");
  const TreeType &again = nursery.get("oak", "green", "bark");
  const TreeType &pine = nursery.get("pine", "dark", "needle");

  EXPECT_EQ(&oak, &again);
  EXPECT_NE(&oak, &pine);
  EXPECT_EQ(nursery.size(), 2);
  EXPECT_TRUE(nursery.contains("oak"));
  EXPECT_TRUE(nursery.contains("pine"));
  EXPECT_EQ(oak.color(), "green");
  EXPECT_EQ(oak.texture(), "bark");
  EXPECT_EQ(nursery.get("oak").draw(1, 2), kOakAt12);
  EXPECT_EQ(&nursery.get("oak"), &oak);
}

TEST(FlyweightTest, FactoryRejectsConflictingIntrinsicState) {
  TreeFactory nursery;
  const TreeType &oak = nursery.get("oak", "green", "bark");

  expect_invalid("species intrinsic state conflicts",
                [&] { nursery.get("oak", "yellow", "bark"); });
  expect_invalid("species intrinsic state conflicts",
                [&] { nursery.get("oak", "green", "other"); });

  EXPECT_EQ(nursery.size(), 1);
  EXPECT_EQ(&nursery.get("oak"), &oak);
  EXPECT_EQ(oak.color(), "green");
  EXPECT_EQ(oak.texture(), "bark");
}

TEST(FlyweightTest, FactoryLookupRequiresRegistration) {
  TreeFactory nursery;
  nursery.get("oak", "green", "bark");

  expect_invalid("species is not registered", [&] { nursery.get("dragon"); });
  expect_invalid("species is required", [&] { nursery.get(""); });
  expect_invalid("species is required",
                [&] { nursery.get("", "green", "bark"); });
  expect_invalid("color is required", [&] { nursery.get("pine", "", "needle"); });
  expect_invalid("texture is required",
                [&] { nursery.get("pine", "dark", ""); });
  expect_invalid("species is required", [&] { (void)nursery.contains(""); });

  EXPECT_FALSE(nursery.contains("dragon"));
  EXPECT_EQ(nursery.size(), 1);

  const TreeFactory &catalog = nursery;
  EXPECT_EQ(catalog.get("oak").name(), "oak");
  EXPECT_EQ(catalog.size(), 1);
}

TEST(FlyweightTest, SeparateFactoriesDoNotShare) {
  TreeFactory west;
  TreeFactory east;
  const TreeType &west_oak = west.get("oak", "green", "bark");
  const TreeType &east_oak = east.get("oak", "green", "bark");

  EXPECT_NE(&west_oak, &east_oak);
  EXPECT_EQ(west_oak.draw(1, 2), east_oak.draw(1, 2));
  EXPECT_EQ(west.size(), 1);
  EXPECT_EQ(east.size(), 1);
}

TEST(FlyweightTest, FactoryLookupIsConst) {
  static_assert(can_create_type<TreeFactory>);
  static_assert(!can_create_type<const TreeFactory>);
  static_assert(can_lookup_type<TreeFactory>);
  static_assert(can_lookup_type<const TreeFactory>);
  static_assert(can_query_pool<const TreeFactory>);
}

TEST(FlyweightTest, PlantedTreeMoveChangesOnlyExtrinsicState) {
  TreeFactory nursery;
  const TreeType &oak = nursery.get("oak", "green", "bark");
  PlantedTree left(1, 2, oak);
  PlantedTree right(3, 4, oak);

  EXPECT_EQ(&left.type(), &oak);
  EXPECT_EQ(&right.type(), &oak);
  EXPECT_EQ(left.draw(), kOakAt12);
  EXPECT_EQ(right.draw(), kOakAt34);

  left.move_to(9, 8);
  EXPECT_EQ(left.x(), 9);
  EXPECT_EQ(left.y(), 8);
  EXPECT_EQ(left.draw(), "oak (green) [bark] @ (9,8)");
  EXPECT_EQ(right.draw(), kOakAt34);
  EXPECT_EQ(oak.draw(1, 2), kOakAt12);
  EXPECT_EQ(oak.color(), "green");
}

TEST(FlyweightTest, ForestSharesTypesAcrossTrees) {
  TreeFactory nursery;
  const TreeType &oak = nursery.get("oak", "green", "bark");
  const TreeType &pine = nursery.get("pine", "dark", "needle");
  Forest forest;

  EXPECT_EQ(forest.size(), 0);
  EXPECT_EQ(forest.render(), "");

  forest.plant(1, 2, oak);
  forest.plant(3, 4, oak);
  forest.plant(8, 5, pine);

  EXPECT_EQ(forest.size(), 3);
  EXPECT_EQ(nursery.size(), 2);
  EXPECT_EQ(&forest.tree(0).type(), &forest.tree(1).type());
  EXPECT_EQ(&forest.tree(0).type(), &oak);
  EXPECT_NE(&forest.tree(2).type(), &oak);
  EXPECT_EQ(forest.render(),
            std::string(kOakAt12) + "\n" + kOakAt34 + "\n" + kPineAt85);

  forest.tree(0).move_to(1, 9);
  EXPECT_EQ(forest.tree(0).draw(), "oak (green) [bark] @ (1,9)");
  EXPECT_EQ(forest.tree(1).x(), 3);
  EXPECT_EQ(forest.tree(1).y(), 4);
  EXPECT_EQ(&forest.tree(0).type(), &forest.tree(1).type());
}

TEST(FlyweightTest, ForestRejectsBadIndex) {
  TreeFactory nursery;
  Forest forest;
  expect_range([&] { (void)forest.tree(0); });

  forest.plant(1, 2, nursery.get("oak", "green", "bark"));
  EXPECT_EQ(forest.tree(0).type().name(), "oak");
  expect_range([&] { (void)forest.tree(1); });

  const Forest &view = forest;
  EXPECT_EQ(view.tree(0).draw(), kOakAt12);
  expect_range([&] { (void)view.tree(1); });
}

TEST(FlyweightTest, ClientDependsOnTreeGlyphAbstraction) {
  TreeFactory nursery;
  const TreeType &oak = nursery.get("oak", "green", "bark");
  const Shrine well("ancient well");
  const TreeGlyph &as_tree = oak;
  const TreeGlyph &as_shrine = well;

  EXPECT_EQ(paint(as_tree, 1, 2), kOakAt12);
  EXPECT_EQ(paint(as_shrine, 0, 1), kShrine);
  EXPECT_EQ(nursery.size(), 1);
  EXPECT_FALSE(nursery.contains("shrine"));
  EXPECT_FALSE(nursery.contains("ancient well"));
}

TEST(FlyweightTest, ShrineIsUnshared) {
  Shrine first("ancient well");
  Shrine second("ancient well");
  TreeFactory nursery;
  nursery.get("oak", "green", "bark");

  EXPECT_NE(&first, &second);
  EXPECT_EQ(first.draw(0, 1), second.draw(0, 1));
  EXPECT_EQ(nursery.size(), 1);

  TreeType named_like_a_shrine("shrine", "gray", "stone");
  EXPECT_NE(named_like_a_shrine.draw(0, 1), first.draw(0, 1));
}

TEST(FlyweightTest, ForestCopyDuplicatesPositionsNotTypes) {
  TreeFactory nursery;
  const TreeType &oak = nursery.get("oak", "green", "bark");
  Forest original;
  original.plant(1, 2, oak);

  Forest copy = original;
  copy.tree(0).move_to(9, 9);

  EXPECT_EQ(original.tree(0).x(), 1);
  EXPECT_EQ(original.tree(0).y(), 2);
  EXPECT_EQ(copy.tree(0).draw(), "oak (green) [bark] @ (9,9)");
  EXPECT_EQ(&original.tree(0).type(), &copy.tree(0).type());
  EXPECT_EQ(&original.tree(0).type(), &oak);
  EXPECT_EQ(nursery.size(), 1);
}

TEST(FlyweightTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<TreeGlyph>);
  static_assert(!std::is_move_constructible_v<TreeGlyph>);
  static_assert(!std::is_copy_assignable_v<TreeGlyph>);
  static_assert(!std::is_move_assignable_v<TreeGlyph>);

  static_assert(!std::is_copy_constructible_v<TreeType>);
  static_assert(!std::is_move_constructible_v<TreeType>);
  static_assert(!std::is_copy_constructible_v<Shrine>);
  static_assert(!std::is_move_constructible_v<Shrine>);

  static_assert(!std::is_copy_constructible_v<TreeFactory>);
  static_assert(!std::is_move_constructible_v<TreeFactory>);
  static_assert(!std::is_copy_assignable_v<TreeFactory>);
  static_assert(!std::is_move_assignable_v<TreeFactory>);

  static_assert(std::is_copy_constructible_v<PlantedTree>);
  static_assert(std::is_move_constructible_v<PlantedTree>);
  static_assert(std::is_copy_assignable_v<PlantedTree>);
  static_assert(std::is_move_assignable_v<PlantedTree>);

  static_assert(std::is_copy_constructible_v<Forest>);
  static_assert(std::is_move_constructible_v<Forest>);
  static_assert(std::is_copy_assignable_v<Forest>);
  static_assert(std::is_move_assignable_v<Forest>);

  static_assert(std::is_copy_constructible_v<InlineTree>);
  static_assert(std::is_move_constructible_v<InlineTree>);
  static_assert(std::is_copy_assignable_v<InlineTree>);
  static_assert(std::is_move_assignable_v<InlineTree>);
}

TEST(FlyweightTest, InheritanceAndConvertibility) {
  static_assert(std::is_base_of_v<TreeGlyph, TreeType>);
  static_assert(std::is_base_of_v<TreeGlyph, Shrine>);
  static_assert(std::has_virtual_destructor_v<TreeGlyph>);
  static_assert(!std::is_base_of_v<TreeGlyph, PlantedTree>);
  static_assert(!std::is_base_of_v<TreeGlyph, InlineTree>);
  static_assert(!std::is_base_of_v<TreeGlyph, TreeFactory>);
  static_assert(!std::is_base_of_v<TreeType, Shrine>);
  static_assert(!std::is_convertible_v<InlineTree *, TreeGlyph *>);
  static_assert(!std::is_convertible_v<PlantedTree *, TreeGlyph *>);

  static_assert(has_draw_at<TreeGlyph>);
  static_assert(has_draw_at<TreeType>);
  static_assert(has_draw_at<Shrine>);
  static_assert(!has_draw_at<PlantedTree>);
  static_assert(!has_draw_at<InlineTree>);

  static_assert(has_color<TreeType>);
  static_assert(!has_color<TreeGlyph>);
  static_assert(!has_color<Shrine>);
  static_assert(!has_set_color<TreeType>);
  static_assert(!has_set_color<Shrine>);
  static_assert(!has_set_color<PlantedTree>);
  static_assert(has_set_color<InlineTree>);
}

TEST(FlyweightTest, InlineTreeMatchesSharedDraw) {
  TreeType shared("oak", "green", "bark");
  InlineTree local(1, 2, "oak", "green", "bark");

  EXPECT_EQ(local.draw(), shared.draw(1, 2));
  EXPECT_EQ(local.name(), "oak");
  EXPECT_EQ(local.color(), "green");
  EXPECT_EQ(local.texture(), "bark");

  local.move_to(3, 4);
  EXPECT_EQ(local.draw(), shared.draw(3, 4));
  EXPECT_EQ(shared.draw(1, 2), kOakAt12);
}

TEST(FlyweightTest, InlineTreeCopiesByValue) {
  InlineTree original(1, 2, "oak", "green", "bark");
  InlineTree copy = original;
  copy.move_to(3, 4);
  copy.set_color("yellow");

  EXPECT_EQ(original.draw(), kOakAt12);
  EXPECT_EQ(original.color(), "green");
  EXPECT_EQ(copy.draw(), "oak (yellow) [bark] @ (3,4)");
  EXPECT_EQ(copy.texture(), original.texture());
  EXPECT_NE(copy.color(), original.color());
}
