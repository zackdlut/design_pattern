#include "structural/bridge/bridge.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
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

std::string paint(const Shape &shape) { return shape.draw(); }

class AsciiRenderer final : public Renderer {
public:
  std::string draw_line(int x1, int y1, int x2, int y2) const override {
    return "ascii line (" + std::to_string(x1) + "," + std::to_string(y1) +
           ")-(" + std::to_string(x2) + "," + std::to_string(y2) + ")";
  }

  std::string draw_circle(int cx, int cy, int radius) const override {
    return "ascii circle (" + std::to_string(cx) + "," + std::to_string(cy) +
           ",r=" + std::to_string(radius) + ")";
  }

  const std::string &name() const override { return name_; }

private:
  std::string name_{"ascii"};
};

// 客户端新增的形状。库里的 VectorRenderer / RasterRenderer 不用改。
class Triangle final : public Shape {
public:
  Triangle(int x, int y, int side, std::unique_ptr<Renderer> renderer)
      : Shape(x, y, std::move(renderer)), side_(side) {
    if (side_ <= 0) {
      throw std::invalid_argument("side must be positive");
    }
  }

  std::string draw() const override {
    const int x2 = x() + side_;
    const int x3 = x() + side_ / 2;
    const int y3 = y() + side_;
    return "Triangle -> " + renderer().draw_line(x(), y(), x2, y()) + " | " +
           renderer().draw_line(x2, y(), x3, y3) + " | " +
           renderer().draw_line(x3, y3, x(), y());
  }

private:
  int side_;
};

}  // namespace

int main() {
  std::cout << "=== 同一个圆，换一只笔：Circle 不必为每种后端再派生子类 ===\n";
  Circle vector_badge(1, 2, 3, std::make_unique<VectorRenderer>());
  Circle raster_badge(1, 2, 3, std::make_unique<RasterRenderer>());
  std::cout << "  " << paint(vector_badge) << "\n";
  std::cout << "  " << paint(raster_badge) << "\n";

  std::cout << "\n=== 同一类笔，圆走 draw_circle，矩形用四条 draw_line ===\n";
  Rectangle frame(0, 0, 4, 3, std::make_unique<VectorRenderer>());
  std::cout << "  " << paint(vector_badge) << "\n";
  std::cout << "  " << paint(frame) << "\n";

  std::cout << "\n=== 几何在形状上改，后端名字不变 ===\n";
  vector_badge.set_position(5, 6);
  vector_badge.set_radius(4);
  std::cout << "  " << paint(vector_badge) << "\n";
  std::cout << "  backend=" << vector_badge.backend() << "\n";

  std::cout << "\n=== 加一种形状、加一种笔，已有的类都不用改 ===\n";
  Triangle mark(0, 0, 4, std::make_unique<VectorRenderer>());
  Circle ascii_badge(1, 1, 2, std::make_unique<AsciiRenderer>());
  Rectangle ascii_frame(0, 0, 4, 3, std::make_unique<AsciiRenderer>());
  std::cout << "  " << paint(mark) << "\n";
  std::cout << "  " << paint(ascii_badge) << "\n";
  std::cout << "  " << paint(ascii_frame) << "\n";

  std::cout << "\n=== 对照：后端集合已经关闭时，枚举分支就够了 ===\n";
  InlineCircle spec(1, 2, 3, DrawBackend::Vector);
  InlineCircle copy = spec;
  copy.set_radius(8);
  std::cout << "  原件 " << spec.draw() << "\n";
  std::cout << "  副本 " << copy.draw() << "\n";
  InlineCircle raster(1, 2, 3, DrawBackend::Raster);
  std::cout << "  光栅 " << raster.draw() << "\n";

  std::cout << "\n=== 空笔、非正尺寸在构造时拒绝 ===\n";
  try {
    Circle bad(0, 0, 1, nullptr);
    (void)bad;
  } catch (const std::invalid_argument &error) {
    std::cout << "  renderer: " << error.what() << "\n";
  }
  try {
    Circle bad(0, 0, 0, std::make_unique<VectorRenderer>());
    (void)bad;
  } catch (const std::invalid_argument &error) {
    std::cout << "  radius: " << error.what() << "\n";
  }
  try {
    Triangle bad(0, 0, 0, std::make_unique<VectorRenderer>());
    (void)bad;
  } catch (const std::invalid_argument &error) {
    std::cout << "  side: " << error.what() << "\n";
  }

  return 0;
}
