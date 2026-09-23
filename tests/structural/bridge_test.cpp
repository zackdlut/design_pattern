#include "structural/bridge/bridge.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

using design_pattern::structural::bridge::Circle;
using design_pattern::structural::bridge::DrawBackend;
using design_pattern::structural::bridge::InlineCircle;
using design_pattern::structural::bridge::RasterRenderer;
using design_pattern::structural::bridge::Rectangle;
using design_pattern::structural::bridge::Renderer;
using design_pattern::structural::bridge::Shape;
using design_pattern::structural::bridge::VectorRenderer;

namespace {

void expect_invalid(const char *message, auto &&action) {
  try {
    action();
    FAIL() << "expected std::invalid_argument: " << message;
  } catch (const std::invalid_argument &error) {
    EXPECT_STREQ(error.what(), message);
  }
}

std::string paint(const Shape &shape) { return shape.draw(); }

const char *kVectorCircle = "Circle -> vector circle (1,2,r=3)";
const char *kRasterCircle = "Circle -> raster circle (1,2,r=3)";
const char *kVectorRectangle =
    "Rectangle -> vector line (0,0)-(4,0) | vector line (4,0)-(4,3) | "
    "vector line (4,3)-(0,3) | vector line (0,3)-(0,0)";
const char *kRasterRectangle =
    "Rectangle -> raster line (0,0)-(4,0) | raster line (4,0)-(4,3) | "
    "raster line (4,3)-(0,3) | raster line (0,3)-(0,0)";

class DotRenderer final : public Renderer {
public:
  std::string draw_line(int, int, int, int) const override { return "dot line"; }

  std::string draw_circle(int, int, int) const override { return "dot circle"; }

  const std::string &name() const override { return name_; }

private:
  std::string name_{"dot"};
};

template <typename T>
constexpr bool has_draw = requires(const T &value) { value.draw(); };

template <typename T>
constexpr bool has_draw_line = requires(const T &value) {
  value.draw_line(0, 0, 1, 1);
};

template <typename T>
constexpr bool has_draw_circle = requires(const T &value) {
  value.draw_circle(0, 0, 1);
};

template <typename T>
constexpr bool has_radius = requires(const T &value) { value.radius(); };

template <typename T>
constexpr bool has_set_radius = requires(T &value) { value.set_radius(1); };

template <typename T>
constexpr bool has_set_size = requires(T &value) { value.set_size(1, 1); };

template <typename T>
constexpr bool has_name = requires(const T &value) { value.name(); };

}  // namespace

TEST(BridgeTest, VectorRendererStrokesLinesAndCircles) {
  VectorRenderer pen;
  EXPECT_EQ(pen.name(), "vector");
  EXPECT_EQ(pen.draw_line(0, 0, 4, 0), "vector line (0,0)-(4,0)");
  EXPECT_EQ(pen.draw_circle(1, 2, 3), "vector circle (1,2,r=3)");
  EXPECT_EQ(pen.draw_line(-2, 4, 0, 0), "vector line (-2,4)-(0,0)");
  EXPECT_EQ(pen.draw_circle(0, 0, 0), "vector circle (0,0,r=0)");
}

TEST(BridgeTest, RasterRendererStrokesLinesAndCircles) {
  RasterRenderer pen;
  EXPECT_EQ(pen.name(), "raster");
  EXPECT_EQ(pen.draw_line(0, 0, 4, 0), "raster line (0,0)-(4,0)");
  EXPECT_EQ(pen.draw_circle(1, 2, 3), "raster circle (1,2,r=3)");
}

TEST(BridgeTest, CircleDrawsWithEitherRenderer) {
  Circle vector_circle(1, 2, 3, std::make_unique<VectorRenderer>());
  Circle raster_circle(1, 2, 3, std::make_unique<RasterRenderer>());

  EXPECT_EQ(vector_circle.backend(), "vector");
  EXPECT_EQ(raster_circle.backend(), "raster");
  EXPECT_EQ(vector_circle.x(), 1);
  EXPECT_EQ(vector_circle.y(), 2);
  EXPECT_EQ(vector_circle.radius(), 3);

  EXPECT_EQ(vector_circle.draw(), kVectorCircle);
  EXPECT_EQ(raster_circle.draw(), kRasterCircle);
  EXPECT_EQ(vector_circle.draw(), kVectorCircle);
}

TEST(BridgeTest, RectangleIsFourLinesOnEitherRenderer) {
  Rectangle vector_rect(0, 0, 4, 3, std::make_unique<VectorRenderer>());
  Rectangle raster_rect(0, 0, 4, 3, std::make_unique<RasterRenderer>());

  EXPECT_EQ(vector_rect.backend(), "vector");
  EXPECT_EQ(raster_rect.backend(), "raster");
  EXPECT_EQ(vector_rect.width(), 4);
  EXPECT_EQ(vector_rect.height(), 3);
  EXPECT_EQ(vector_rect.draw(), kVectorRectangle);
  EXPECT_EQ(raster_rect.draw(), kRasterRectangle);
  EXPECT_EQ(vector_rect.draw().find("circle"), std::string::npos);
}

TEST(BridgeTest, ShapesAcceptAnUnforeseenRenderer) {
  Circle circle(1, 2, 3, std::make_unique<DotRenderer>());
  EXPECT_EQ(circle.backend(), "dot");
  EXPECT_EQ(circle.draw(), "Circle -> dot circle");

  Rectangle rect(0, 0, 4, 3, std::make_unique<DotRenderer>());
  EXPECT_EQ(rect.backend(), "dot");
  EXPECT_EQ(rect.draw(),
            "Rectangle -> dot line | dot line | dot line | dot line");
}

TEST(BridgeTest, GeometryChangesStayOnTheShape) {
  Circle circle(1, 2, 3, std::make_unique<VectorRenderer>());
  circle.set_position(-2, 4);
  circle.set_radius(5);
  EXPECT_EQ(circle.backend(), "vector");
  EXPECT_EQ(circle.draw(), "Circle -> vector circle (-2,4,r=5)");

  Rectangle rect(0, 0, 4, 3, std::make_unique<RasterRenderer>());
  rect.set_position(1, 1);
  rect.set_size(2, 2);
  EXPECT_EQ(rect.backend(), "raster");
  EXPECT_EQ(rect.width(), 2);
  EXPECT_EQ(rect.height(), 2);
  EXPECT_EQ(rect.draw(),
            "Rectangle -> raster line (1,1)-(3,1) | raster line (3,1)-(3,3) | "
            "raster line (3,3)-(1,3) | raster line (1,3)-(1,1)");
}

TEST(BridgeTest, ShapesKeepIndependentGeometry) {
  Circle left(0, 0, 2, std::make_unique<VectorRenderer>());
  Circle right(0, 0, 2, std::make_unique<RasterRenderer>());

  left.set_radius(9);
  left.set_position(3, 4);

  EXPECT_EQ(left.draw(), "Circle -> vector circle (3,4,r=9)");
  EXPECT_EQ(right.draw(), "Circle -> raster circle (0,0,r=2)");
  EXPECT_EQ(right.radius(), 2);
  EXPECT_EQ(right.backend(), "raster");
}

TEST(BridgeTest, ClientDependsOnShapeAbstraction) {
  Circle circle(1, 2, 3, std::make_unique<VectorRenderer>());
  Rectangle rect(0, 0, 4, 3, std::make_unique<RasterRenderer>());
  const Shape &as_circle = circle;
  const Shape &as_rect = rect;

  EXPECT_EQ(paint(as_circle), kVectorCircle);
  EXPECT_EQ(paint(as_rect), kRasterRectangle);
  EXPECT_EQ(as_circle.backend(), "vector");
  EXPECT_EQ(as_rect.backend(), "raster");
}

TEST(BridgeTest, InlineCircleMatchesTheClosedCase) {
  Circle vector_circle(1, 2, 3, std::make_unique<VectorRenderer>());
  Circle raster_circle(1, 2, 3, std::make_unique<RasterRenderer>());
  InlineCircle vector_inline(1, 2, 3, DrawBackend::Vector);
  InlineCircle raster_inline(1, 2, 3, DrawBackend::Raster);

  EXPECT_EQ(vector_inline.draw(), vector_circle.draw());
  EXPECT_EQ(raster_inline.draw(), raster_circle.draw());
  EXPECT_EQ(vector_inline.backend(), DrawBackend::Vector);
  EXPECT_EQ(raster_inline.backend(), DrawBackend::Raster);
  EXPECT_EQ(vector_inline.x(), 1);
  EXPECT_EQ(vector_inline.y(), 2);
  EXPECT_EQ(vector_inline.radius(), 3);
}

TEST(BridgeTest, InlineCircleCopiesByValue) {
  InlineCircle spec(1, 2, 3, DrawBackend::Vector);
  InlineCircle copy = spec;
  copy.set_position(4, 5);
  copy.set_radius(6);

  EXPECT_EQ(spec.draw(), kVectorCircle);
  EXPECT_EQ(copy.draw(), "Circle -> vector circle (4,5,r=6)");
  EXPECT_EQ(spec.radius(), 3);
  EXPECT_EQ(copy.backend(), DrawBackend::Vector);
}

TEST(BridgeTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<Renderer>);
  static_assert(!std::is_move_constructible_v<Renderer>);
  static_assert(!std::is_copy_assignable_v<Renderer>);
  static_assert(!std::is_move_assignable_v<Renderer>);

  static_assert(!std::is_copy_constructible_v<VectorRenderer>);
  static_assert(!std::is_move_constructible_v<VectorRenderer>);
  static_assert(!std::is_copy_constructible_v<RasterRenderer>);
  static_assert(!std::is_move_constructible_v<RasterRenderer>);

  static_assert(!std::is_copy_constructible_v<Shape>);
  static_assert(!std::is_move_constructible_v<Shape>);
  static_assert(!std::is_copy_assignable_v<Shape>);
  static_assert(!std::is_move_assignable_v<Shape>);

  static_assert(!std::is_copy_constructible_v<Circle>);
  static_assert(!std::is_move_constructible_v<Circle>);
  static_assert(!std::is_copy_constructible_v<Rectangle>);
  static_assert(!std::is_move_constructible_v<Rectangle>);

  static_assert(std::is_copy_constructible_v<InlineCircle>);
  static_assert(std::is_move_constructible_v<InlineCircle>);
  static_assert(std::is_copy_assignable_v<InlineCircle>);
  static_assert(std::is_move_assignable_v<InlineCircle>);
}

TEST(BridgeTest, InheritanceAndConvertibility) {
  static_assert(std::is_base_of_v<Renderer, VectorRenderer>);
  static_assert(std::is_base_of_v<Renderer, RasterRenderer>);
  static_assert(std::is_base_of_v<Shape, Circle>);
  static_assert(std::is_base_of_v<Shape, Rectangle>);
  static_assert(std::has_virtual_destructor_v<Renderer>);
  static_assert(std::has_virtual_destructor_v<Shape>);

  static_assert(!std::is_base_of_v<Renderer, Shape>);
  static_assert(!std::is_base_of_v<Shape, Renderer>);
  static_assert(!std::is_base_of_v<Renderer, Circle>);
  static_assert(!std::is_base_of_v<Renderer, Rectangle>);
  static_assert(!std::is_base_of_v<Circle, Rectangle>);
  static_assert(!std::is_base_of_v<VectorRenderer, RasterRenderer>);
  static_assert(!std::is_base_of_v<Shape, InlineCircle>);
  static_assert(!std::is_base_of_v<Renderer, InlineCircle>);

  static_assert(std::is_convertible_v<Circle *, Shape *>);
  static_assert(std::is_convertible_v<Rectangle *, Shape *>);
  static_assert(std::is_convertible_v<VectorRenderer *, Renderer *>);
  static_assert(std::is_convertible_v<RasterRenderer *, Renderer *>);
  static_assert(!std::is_convertible_v<Circle *, Renderer *>);
  static_assert(!std::is_convertible_v<VectorRenderer *, Shape *>);
  static_assert(!std::is_convertible_v<Circle *, Rectangle *>);
  static_assert(!std::is_convertible_v<InlineCircle *, Shape *>);
  static_assert(!std::is_convertible_v<Shape *, Circle *>);
}

TEST(BridgeTest, QueriesStayOnTheirOwnSide) {
  static_assert(has_draw<Shape>);
  static_assert(has_draw<Circle>);
  static_assert(has_draw<Rectangle>);
  static_assert(has_draw<InlineCircle>);
  static_assert(!has_draw<Renderer>);
  static_assert(!has_draw<VectorRenderer>);

  static_assert(has_draw_line<Renderer>);
  static_assert(has_draw_circle<Renderer>);
  static_assert(has_draw_line<VectorRenderer>);
  static_assert(!has_draw_line<Shape>);
  static_assert(!has_draw_line<Circle>);
  static_assert(!has_draw_line<Rectangle>);
  static_assert(!has_draw_circle<Circle>);
  static_assert(!has_draw_circle<Rectangle>);
  static_assert(!has_draw_line<InlineCircle>);

  static_assert(has_radius<Circle>);
  static_assert(has_radius<InlineCircle>);
  static_assert(has_set_radius<Circle>);
  static_assert(!has_radius<Shape>);
  static_assert(!has_radius<Rectangle>);
  static_assert(!has_radius<Renderer>);
  static_assert(!has_set_radius<Shape>);
  static_assert(!has_set_radius<Rectangle>);
  static_assert(has_set_size<Rectangle>);
  static_assert(!has_set_size<Circle>);
  static_assert(!has_set_size<Shape>);
  static_assert(!has_set_size<Renderer>);

  static_assert(has_name<Renderer>);
  static_assert(has_name<VectorRenderer>);
  static_assert(!has_name<Shape>);
  static_assert(!has_name<Circle>);
  static_assert(!has_name<Rectangle>);

  static_assert(std::is_same_v<decltype(std::declval<const Shape &>().backend()),
                               const std::string &>);
  static_assert(std::is_same_v<
                decltype(std::declval<const InlineCircle &>().backend()),
                DrawBackend>);
}

TEST(BridgeTest, ConstructorsRejectInvalidInput) {
  expect_invalid("renderer is required",
                 [] { Circle circle(0, 0, 1, nullptr); });
  expect_invalid("renderer is required",
                 [] { Circle circle(0, 0, 0, nullptr); });
  expect_invalid("renderer is required",
                 [] { Rectangle rect(0, 0, 1, 1, nullptr); });
  expect_invalid("radius must be positive", [] {
    Circle circle(0, 0, 0, std::make_unique<VectorRenderer>());
  });
  expect_invalid("radius must be positive", [] {
    Circle circle(0, 0, -1, std::make_unique<RasterRenderer>());
  });
  expect_invalid("width must be positive", [] {
    Rectangle rect(0, 0, 0, 3, std::make_unique<VectorRenderer>());
  });
  expect_invalid("height must be positive", [] {
    Rectangle rect(0, 0, 4, 0, std::make_unique<VectorRenderer>());
  });
  expect_invalid("height must be positive", [] {
    Rectangle rect(0, 0, 4, -2, std::make_unique<RasterRenderer>());
  });
  expect_invalid("radius must be positive",
                 [] { InlineCircle circle(0, 0, 0, DrawBackend::Vector); });
  expect_invalid("backend is unknown", [] {
    InlineCircle circle(0, 0, 1, static_cast<DrawBackend>(7));
  });
}

TEST(BridgeTest, SettersRejectInvalidSizes) {
  Circle circle(1, 2, 3, std::make_unique<VectorRenderer>());
  expect_invalid("radius must be positive", [&] { circle.set_radius(0); });
  expect_invalid("radius must be positive", [&] { circle.set_radius(-4); });
  EXPECT_EQ(circle.radius(), 3);
  EXPECT_EQ(circle.draw(), kVectorCircle);

  Rectangle rect(0, 0, 4, 3, std::make_unique<VectorRenderer>());
  expect_invalid("width must be positive", [&] { rect.set_size(0, 5); });
  expect_invalid("height must be positive", [&] { rect.set_size(5, 0); });
  expect_invalid("height must be positive", [&] { rect.set_size(5, -1); });
  EXPECT_EQ(rect.width(), 4);
  EXPECT_EQ(rect.height(), 3);
  EXPECT_EQ(rect.draw(), kVectorRectangle);

  InlineCircle inline_circle(1, 2, 3, DrawBackend::Raster);
  expect_invalid("radius must be positive",
                 [&] { inline_circle.set_radius(0); });
  EXPECT_EQ(inline_circle.radius(), 3);
  EXPECT_EQ(inline_circle.draw(), kRasterCircle);
}
