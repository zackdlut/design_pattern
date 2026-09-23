#include "structural/flyweight/flyweight.h"

#include <stdexcept>
#include <utility>

namespace design_pattern::structural::flyweight {

namespace {

void require_text(std::string_view text, const char *message) {
  if (text.empty()) {
    throw std::invalid_argument(message);
  }
}

std::string require_owned(std::string text, const char *message) {
  require_text(text, message);
  return text;
}

std::string format_tree(std::string_view species, std::string_view color,
                        std::string_view texture, int x, int y) {
  return std::string(species) + " (" + std::string(color) + ") [" +
         std::string(texture) + "] @ (" + std::to_string(x) + "," +
         std::to_string(y) + ")";
}

}  // namespace

TreeType::TreeType(std::string species, std::string color, std::string texture)
    : species_(require_owned(std::move(species), "species is required")),
      color_(require_owned(std::move(color), "color is required")),
      texture_(require_owned(std::move(texture), "texture is required")) {}

std::string TreeType::draw(int x, int y) const {
  return format_tree(species_, color_, texture_, x, y);
}

const std::string &TreeType::name() const { return species_; }

const std::string &TreeType::color() const { return color_; }

const std::string &TreeType::texture() const { return texture_; }

Shrine::Shrine(std::string inscription)
    : inscription_(require_owned(std::move(inscription),
                                 "inscription is required")) {}

std::string Shrine::draw(int x, int y) const {
  return name_ + " [" + inscription_ + "] @ (" + std::to_string(x) + "," +
         std::to_string(y) + ")";
}

const std::string &Shrine::name() const { return name_; }

const std::string &Shrine::inscription() const { return inscription_; }

const TreeType &TreeFactory::get(std::string_view species,
                                 std::string_view color,
                                 std::string_view texture) {
  require_text(species, "species is required");
  require_text(color, "color is required");
  require_text(texture, "texture is required");

  const auto found = types_.find(std::string(species));
  if (found != types_.end()) {
    const TreeType &existing = *found->second;
    if (existing.color() != color || existing.texture() != texture) {
      throw std::invalid_argument("species intrinsic state conflicts");
    }
    return existing;
  }

  const std::string key(species);
  auto created =
      std::make_unique<TreeType>(key, std::string(color), std::string(texture));
  const TreeType &stored = *created;
  types_.emplace(key, std::move(created));
  return stored;
}

const TreeType &TreeFactory::get(std::string_view species) const {
  require_text(species, "species is required");
  const auto found = types_.find(std::string(species));
  if (found == types_.end()) {
    throw std::invalid_argument("species is not registered");
  }
  return *found->second;
}

bool TreeFactory::contains(std::string_view species) const {
  require_text(species, "species is required");
  return types_.find(std::string(species)) != types_.end();
}

std::size_t TreeFactory::size() const { return types_.size(); }

PlantedTree::PlantedTree(int x, int y, const TreeType &type)
    : x_(x), y_(y), type_(&type) {}

std::string PlantedTree::draw() const { return type_->draw(x_, y_); }

void PlantedTree::move_to(int x, int y) {
  x_ = x;
  y_ = y;
}

int PlantedTree::x() const { return x_; }

int PlantedTree::y() const { return y_; }

const TreeType &PlantedTree::type() const { return *type_; }

void Forest::plant(int x, int y, const TreeType &type) {
  trees_.emplace_back(x, y, type);
}

std::string Forest::render() const {
  std::string picture;
  for (std::size_t i = 0; i < trees_.size(); ++i) {
    if (i != 0) {
      picture += '\n';
    }
    picture += trees_[i].draw();
  }
  return picture;
}

std::size_t Forest::size() const { return trees_.size(); }

PlantedTree &Forest::tree(std::size_t index) {
  if (index >= trees_.size()) {
    throw std::out_of_range("tree index out of range");
  }
  return trees_[index];
}

const PlantedTree &Forest::tree(std::size_t index) const {
  if (index >= trees_.size()) {
    throw std::out_of_range("tree index out of range");
  }
  return trees_[index];
}

InlineTree::InlineTree(int x, int y, std::string species, std::string color,
                       std::string texture)
    : x_(x),
      y_(y),
      species_(require_owned(std::move(species), "species is required")),
      color_(require_owned(std::move(color), "color is required")),
      texture_(require_owned(std::move(texture), "texture is required")) {}

std::string InlineTree::draw() const {
  return format_tree(species_, color_, texture_, x_, y_);
}

void InlineTree::move_to(int x, int y) {
  x_ = x;
  y_ = y;
}

void InlineTree::set_color(std::string color) {
  color_ = require_owned(std::move(color), "color is required");
}

int InlineTree::x() const { return x_; }

int InlineTree::y() const { return y_; }

const std::string &InlineTree::name() const { return species_; }

const std::string &InlineTree::color() const { return color_; }

const std::string &InlineTree::texture() const { return texture_; }

}  // namespace design_pattern::structural::flyweight
