#include "structural/composite/composite.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

using design_pattern::structural::composite::Directory;
using design_pattern::structural::composite::Document;
using design_pattern::structural::composite::Entry;
using design_pattern::structural::composite::File;
using design_pattern::structural::composite::Folder;
using design_pattern::structural::composite::Node;

namespace {

// 客户端只依赖 Node。叶子的 childCount 是 0，循环自然结束。
int count_nodes(const Node &node) {
  int total = 1;
  for (std::size_t i = 0; i < node.childCount(); ++i) {
    total += count_nodes(*node.child(i));
  }
  return total;
}

// 安全组合的基类没有 childCount。要往下走，先认出 Directory。
int count_entries(const Entry &entry) {
  int total = 1;
  if (const auto *directory = dynamic_cast<const Directory *>(&entry)) {
    for (std::size_t i = 0; i < directory->childCount(); ++i) {
      total += count_entries(*directory->child(i));
    }
  }
  return total;
}

int measure(const Node &node) { return node.size(); }

int measure_entry(const Entry &entry) { return entry.size(); }

}  // namespace

int main() {
  std::cout << "=== 透明组合：文件和文件夹都是 Node，对根问一次 size ===\n";

  Folder docs("docs");
  docs.add(std::make_unique<File>("readme.txt", 12));
  auto src = std::make_unique<Folder>("src");
  src->add(std::make_unique<File>("main.cpp", 40));
  src->add(std::make_unique<File>("util.h", 8));
  docs.add(std::move(src));

  const Node &as_tree = docs;
  std::cout << as_tree.describe() << "\n";
  std::cout << "  size:  " << measure(as_tree) << "\n";
  std::cout << "  nodes: " << count_nodes(as_tree) << "\n";

  File readme("readme.txt", 12);
  const Node &as_file = readme;
  std::cout << "  same call on a file: size=" << measure(as_file)
            << " nodes=" << count_nodes(as_file) << "\n";

  std::cout << "\n=== 组树也可以只拿 Node&；叶子上的 add 会拒绝 ===\n";
  Folder notes("notes");
  Node &as_folder = notes;
  as_folder.add(std::make_unique<File>("todo.txt", 6));
  std::cout << "  " << notes.describe() << "\n";
  try {
    Node &as_leaf = readme;
    as_leaf.add(std::make_unique<File>("nope.txt", 1));
  } catch (const std::logic_error &error) {
    std::cout << "  leaf: " << error.what() << "\n";
  }

  std::cout << "\n=== 对照：安全组合，Entry 上没有 add ===\n";
  Directory archive("docs");
  archive.add(std::make_unique<Document>("readme.txt", 12));
  auto nested = std::make_unique<Directory>("src");
  nested->add(std::make_unique<Document>("main.cpp", 40));
  nested->add(std::make_unique<Document>("util.h", 8));
  archive.add(std::move(nested));

  const Entry &as_archive = archive;
  // as_archive.add(...) 编译失败：Entry 没有 add。组树必须拿着 Directory。
  std::cout << as_archive.describe() << "\n";
  std::cout << "  size:    " << measure_entry(as_archive) << "\n";
  std::cout << "  entries: " << count_entries(as_archive) << "\n";

  std::cout << "\n=== 错误处理：空名字进不了节点，重名进不了同一个父节点 ===\n";
  try {
    File nameless("", 1);
  } catch (const std::invalid_argument &error) {
    std::cout << "  ctor: " << error.what() << "\n";
  }
  try {
    docs.add(std::make_unique<File>("readme.txt", 1));
  } catch (const std::invalid_argument &error) {
    std::cout << "  add:  " << error.what() << "\n";
  }

  return 0;
}
