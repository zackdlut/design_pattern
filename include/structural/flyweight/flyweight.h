#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace design_pattern::structural::flyweight {

// Flyweight。绘制时由调用方传入坐标。
// 对象自己只保留和这一份图元有关、并且可以共用的数据。
class TreeGlyph {
public:
  TreeGlyph() = default;
  virtual ~TreeGlyph() = default;
  TreeGlyph(const TreeGlyph &) = delete;
  TreeGlyph &operator=(const TreeGlyph &) = delete;
  TreeGlyph(TreeGlyph &&) = delete;
  TreeGlyph &operator=(TreeGlyph &&) = delete;

  virtual std::string draw(int x, int y) const = 0;
  virtual const std::string &name() const = 0;
};

// ConcreteFlyweight。树种、颜色、纹理是内在状态，构造之后不再改。
// 自己构造两份相同数据，仍然是两个对象。共享只发生在 TreeFactory 里。
class TreeType final : public TreeGlyph {
public:
  TreeType(std::string species, std::string color, std::string texture);

  std::string draw(int x, int y) const override;
  const std::string &name() const override;
  const std::string &color() const;
  const std::string &texture() const;

private:
  std::string species_;
  std::string color_;
  std::string texture_;
};

// UnsharedConcreteFlyweight。铭文每座石碑一份，放进工厂也几乎不会命中。
// 接口仍然是 TreeGlyph，绘制时坐标照样从外面传进来。
class Shrine final : public TreeGlyph {
public:
  explicit Shrine(std::string inscription);

  std::string draw(int x, int y) const override;
  const std::string &name() const override;
  const std::string &inscription() const;

private:
  std::string inscription_;
  std::string name_{"shrine"};
};

// FlyweightFactory。按树种名字保留唯一一份 TreeType。
// 不是单例：两座苗圃各有自己的池。已经交出去的引用在本对象存活期间一直有效。
class TreeFactory final {
public:
  TreeFactory() = default;
  TreeFactory(const TreeFactory &) = delete;
  TreeFactory &operator=(const TreeFactory &) = delete;
  TreeFactory(TreeFactory &&) = delete;
  TreeFactory &operator=(TreeFactory &&) = delete;

  const TreeType &get(std::string_view species, std::string_view color,
                      std::string_view texture);
  const TreeType &get(std::string_view species) const;
  bool contains(std::string_view species) const;
  std::size_t size() const;

private:
  std::map<std::string, std::unique_ptr<TreeType>> types_;
};

// 上下文。坐标是外在状态，图元不归这棵树所有。
// type 指向的 TreeType 必须比本对象活得更久，通常就是创建它的那座工厂。
class PlantedTree final {
public:
  PlantedTree(int x, int y, const TreeType &type);

  std::string draw() const;
  void move_to(int x, int y);

  int x() const;
  int y() const;
  const TreeType &type() const;

private:
  int x_;
  int y_;
  const TreeType *type_;
};

// 客户端。林子里记的是每棵树的坐标，树种纹理仍在工厂那一份上。
class Forest final {
public:
  void plant(int x, int y, const TreeType &type);
  std::string render() const;

  std::size_t size() const;
  PlantedTree &tree(std::size_t index);
  const PlantedTree &tree(std::size_t index) const;

private:
  std::vector<PlantedTree> trees_;
};

// 对照，不是享元。每棵树自己带着树种、颜色和纹理。
// 可以拷贝。不能放进 TreeGlyph&，也没有工厂。
class InlineTree final {
public:
  InlineTree(int x, int y, std::string species, std::string color,
             std::string texture);

  std::string draw() const;
  void move_to(int x, int y);
  void set_color(std::string color);

  int x() const;
  int y() const;
  const std::string &name() const;
  const std::string &color() const;
  const std::string &texture() const;

private:
  int x_;
  int y_;
  std::string species_;
  std::string color_;
  std::string texture_;
};

}  // namespace design_pattern::structural::flyweight
