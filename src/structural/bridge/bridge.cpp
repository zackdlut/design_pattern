#include "structural/bridge/bridge.h"

#include <stdexcept>
#include <string_view>
#include <utility>

namespace design_pattern::structural::bridge {

namespace {

std::unique_ptr<Renderer> require_renderer(std::unique_ptr<Renderer> renderer) {
  if (!renderer) {
    throw std::invalid_argument("renderer is required");
  }
  return renderer;
}

int require_positive(int value, std::string_view field) {
  if (value <= 0) {
    throw std::invalid_argument(std::string(field) + " must be positive");
  }
  return value;
}

std::string format_line(std::string_view backend, int x1, int y1, int x2,
                        int y2) {
  return std::string(backend) + " line (" + std::to_string(x1) + "," +
         std::to_string(y1) + ")-(" + std::to_string(x2) + "," +
         std::to_string(y2) + ")";
}

std::string format_circle(std::string_view backend, int cx, int cy,
                          int radius) {
  return std::string(backend) + " circle (" + std::to_string(cx) + "," +
         std::to_string(cy) + ",r=" + std::to_string(radius) + ")";
}

std::string_view backend_name(DrawBackend backend) {
  switch (backend) {
  case DrawBackend::Vector:
    return "vector";
  case DrawBackend::Raster:
    return "raster";
  }
  throw std::invalid_argument("backend is unknown");
}

DrawBackend require_backend(DrawBackend backend) {
  (void)backend_name(backend);
  return backend;
}

}  // namespace

std::string VectorRenderer::draw_line(int x1, int y1, int x2, int y2) const {
  return format_line(name_, x1, y1, x2, y2);
}

std::string VectorRenderer::draw_circle(int cx, int cy, int radius) const {
  return format_circle(name_, cx, cy, radius);
}

const std::string &VectorRenderer::name() const { return name_; }

std::string RasterRenderer::draw_line(int x1, int y1, int x2, int y2) const {
  return format_line(name_, x1, y1, x2, y2);
}

std::string RasterRenderer::draw_circle(int cx, int cy, int radius) const {
  return format_circle(name_, cx, cy, radius);
}

const std::string &RasterRenderer::name() const { return name_; }

Shape::Shape(int x, int y, std::unique_ptr<Renderer> renderer)
    : x_(x), y_(y), renderer_(require_renderer(std::move(renderer))) {}

const Renderer &Shape::renderer() const { return *renderer_; }

const std::string &Shape::backend() const { return renderer_->name(); }

int Shape::x() const { return x_; }

int Shape::y() const { return y_; }

void Shape::set_position(int x, int y) {
  x_ = x;
  y_ = y;
}

Circle::Circle(int x, int y, int radius, std::unique_ptr<Renderer> renderer)
    : Shape(x, y, std::move(renderer)),
      radius_(require_positive(radius, "radius")) {}

std::string Circle::draw() const {
  return "Circle -> " + renderer().draw_circle(x(), y(), radius_);
}

int Circle::radius() const { return radius_; }

void Circle::set_radius(int radius) {
  radius_ = require_positive(radius, "radius");
}

Rectangle::Rectangle(int x, int y, int width, int height,
                     std::unique_ptr<Renderer> renderer)
    : Shape(x, y, std::move(renderer)),
      width_(require_positive(width, "width")),
      height_(require_positive(height, "height")) {}

std::string Rectangle::draw() const {
  const int left = x();
  const int top = y();
  const int right = left + width_;
  const int bottom = top + height_;
  const Renderer &pen = renderer();
  return "Rectangle -> " + pen.draw_line(left, top, right, top) + " | " +
         pen.draw_line(right, top, right, bottom) + " | " +
         pen.draw_line(right, bottom, left, bottom) + " | " +
         pen.draw_line(left, bottom, left, top);
}

int Rectangle::width() const { return width_; }

int Rectangle::height() const { return height_; }

void Rectangle::set_size(int width, int height) {
  const int checked_width = require_positive(width, "width");
  const int checked_height = require_positive(height, "height");
  width_ = checked_width;
  height_ = checked_height;
}

InlineCircle::InlineCircle(int x, int y, int radius, DrawBackend backend)
    : x_(x),
      y_(y),
      radius_(require_positive(radius, "radius")),
      backend_(require_backend(backend)) {}

std::string InlineCircle::draw() const {
  return "Circle -> " +
         format_circle(backend_name(backend_), x_, y_, radius_);
}

void InlineCircle::set_position(int x, int y) {
  x_ = x;
  y_ = y;
}

void InlineCircle::set_radius(int radius) {
  radius_ = require_positive(radius, "radius");
}

DrawBackend InlineCircle::backend() const { return backend_; }

int InlineCircle::x() const { return x_; }

int InlineCircle::y() const { return y_; }

int InlineCircle::radius() const { return radius_; }

}  // namespace design_pattern::structural::bridge
