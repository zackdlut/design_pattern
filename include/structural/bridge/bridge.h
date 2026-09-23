#pragma once

#include <memory>
#include <string>

namespace design_pattern::structural::bridge {

// Implementor。只提供笔画原语，不认识圆和矩形。
// 新形状用这些原语拼出自己的 draw()，不必给每种笔再加一个方法。
class Renderer {
public:
  Renderer() = default;
  virtual ~Renderer() = default;
  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;
  Renderer(Renderer &&) = delete;
  Renderer &operator=(Renderer &&) = delete;

  virtual std::string draw_line(int x1, int y1, int x2, int y2) const = 0;
  virtual std::string draw_circle(int cx, int cy, int radius) const = 0;
  virtual const std::string &name() const = 0;
};

class VectorRenderer final : public Renderer {
public:
  std::string draw_line(int x1, int y1, int x2, int y2) const override;
  std::string draw_circle(int cx, int cy, int radius) const override;
  const std::string &name() const override;

private:
  std::string name_{"vector"};
};

class RasterRenderer final : public Renderer {
public:
  std::string draw_line(int x1, int y1, int x2, int y2) const override;
  std::string draw_circle(int cx, int cy, int radius) const override;
  const std::string &name() const override;

private:
  std::string name_{"raster"};
};

// Abstraction。客户端只调 draw()。
// 笔在构造时注入。换笔不产生新的形状子类，换形状也不产生新的笔。
class Shape {
public:
  virtual ~Shape() = default;
  Shape(const Shape &) = delete;
  Shape &operator=(const Shape &) = delete;
  Shape(Shape &&) = delete;
  Shape &operator=(Shape &&) = delete;

  virtual std::string draw() const = 0;
  const std::string &backend() const;
  int x() const;
  int y() const;
  void set_position(int x, int y);

protected:
  Shape(int x, int y, std::unique_ptr<Renderer> renderer);
  const Renderer &renderer() const;

private:
  int x_;
  int y_;
  std::unique_ptr<Renderer> renderer_;
};

class Circle final : public Shape {
public:
  Circle(int x, int y, int radius, std::unique_ptr<Renderer> renderer);
  std::string draw() const override;
  int radius() const;
  void set_radius(int radius);

private:
  int radius_;
};

// 矩形不是笔的原语。四条边由形状自己用 draw_line 拼出来。
class Rectangle final : public Shape {
public:
  Rectangle(int x, int y, int width, int height,
            std::unique_ptr<Renderer> renderer);
  std::string draw() const override;
  int width() const;
  int height() const;
  void set_size(int width, int height);

private:
  int width_;
  int height_;
};

// 对照用的封闭后端。只有这两种，再加一种就要改 InlineCircle。
enum class DrawBackend { Vector, Raster };

// 对照，不是桥接。后端是枚举，分支写在这一个类里。
// 可以拷贝。不能放进 Shape&，也没有矩形。
class InlineCircle final {
public:
  InlineCircle(int x, int y, int radius, DrawBackend backend);

  std::string draw() const;
  void set_position(int x, int y);
  void set_radius(int radius);

  DrawBackend backend() const;
  int x() const;
  int y() const;
  int radius() const;

private:
  int x_;
  int y_;
  int radius_;
  DrawBackend backend_;
};

}  // namespace design_pattern::structural::bridge
