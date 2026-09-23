#include "structural/flyweight/flyweight.h"

#include <iostream>
#include <stdexcept>
#include <string>

using design_pattern::structural::flyweight::Forest;
using design_pattern::structural::flyweight::InlineTree;
using design_pattern::structural::flyweight::Shrine;
using design_pattern::structural::flyweight::TreeFactory;
using design_pattern::structural::flyweight::TreeGlyph;
using design_pattern::structural::flyweight::TreeType;

namespace {

std::string paint(const TreeGlyph &glyph, int x, int y) {
  return glyph.draw(x, y);
}

}  // namespace

int main() {
  std::cout << std::boolalpha;

  std::cout << "=== 苗圃：同一树种只保留一份纹理 ===\n";
  TreeFactory nursery;
  const TreeType &oak = nursery.get("oak", "green", "oak-bark");
  const TreeType &oak_again = nursery.get("oak", "green", "oak-bark");
  const TreeType &pine = nursery.get("pine", "dark", "pine-needle");
  std::cout << "  oak == oak again: " << (&oak == &oak_again) << "\n";
  std::cout << "  oak == pine:      " << (&oak == &pine) << "\n";
  std::cout << "  shared types=" << nursery.size() << "\n";
  std::cout << "  " << oak.draw(0, 0) << "\n";

  std::cout << "\n=== 林子只记坐标，绘制时把坐标传给同一份树种 ===\n";
  Forest forest;
  forest.plant(1, 2, oak);
  forest.plant(4, 1, oak);
  forest.plant(8, 5, pine);
  std::cout << forest.render() << "\n";
  std::cout << "  trees=" << forest.size()
            << " types=" << nursery.size()
            << " same oak type="
            << (&forest.tree(0).type() == &forest.tree(1).type()) << "\n";

  forest.tree(0).move_to(1, 9);
  std::cout << "  after move: " << forest.tree(0).draw() << "\n";
  std::cout << "  neighbor:   " << forest.tree(1).draw() << "\n";
  std::cout << "  type still: " << oak.draw(1, 2) << "\n";

  std::cout << "\n=== 客户端拿的是 TreeGlyph&，石碑不进苗圃 ===\n";
  const Shrine well("ancient well");
  const TreeGlyph &as_oak = oak;
  const TreeGlyph &as_shrine = well;
  std::cout << "  " << paint(as_oak, 1, 2) << "\n";
  std::cout << "  " << paint(as_shrine, 0, 0) << "\n";
  std::cout << "  contains shrine=" << nursery.contains("shrine")
            << " types=" << nursery.size() << "\n";

  std::cout << "\n=== 另一座苗圃不和这一座共享 ===\n";
  TreeFactory other;
  const TreeType &other_oak = other.get("oak", "green", "oak-bark");
  std::cout << "  same object=" << (&oak == &other_oak) << "\n";
  std::cout << "  same text=" << (oak.draw(1, 2) == other_oak.draw(1, 2))
            << "\n";

  std::cout << "\n=== 同一树种不能再登记另一套颜色或纹理 ===\n";
  try {
    nursery.get("oak", "yellow", "oak-bark");
  } catch (const std::invalid_argument &error) {
    std::cout << "  color: " << error.what() << "\n";
  }
  try {
    nursery.get("oak", "green", "other-bark");
  } catch (const std::invalid_argument &error) {
    std::cout << "  texture: " << error.what() << "\n";
  }
  std::cout << "  types=" << nursery.size()
            << " color=" << nursery.get("oak").color() << "\n";

  std::cout << "\n=== 对照：每棵树自己带一份纹理，改颜色互不影响 ===\n";
  InlineTree local(1, 2, "oak", "green", "oak-bark");
  InlineTree copied = local;
  std::cout << "  same text as shared=" << (local.draw() == oak.draw(1, 2))
            << "\n";
  copied.move_to(4, 1);
  copied.set_color("yellow");
  std::cout << "  original: " << local.draw() << "\n";
  std::cout << "  copy:     " << copied.draw() << "\n";

  std::cout << "\n=== 空树种在进苗圃之前拒绝 ===\n";
  const std::size_t types = nursery.size();
  try {
    nursery.get("", "green", "oak-bark");
  } catch (const std::invalid_argument &error) {
    std::cout << "  " << error.what() << "\n";
  }
  std::cout << "  types unchanged=" << (nursery.size() == types) << "\n";

  return 0;
}
