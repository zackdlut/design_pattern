#include "structural/composite/composite.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

using design_pattern::structural::composite::Directory;
using design_pattern::structural::composite::Document;
using design_pattern::structural::composite::Entry;
using design_pattern::structural::composite::File;
using design_pattern::structural::composite::Folder;
using design_pattern::structural::composite::Node;

namespace {

template <class T>
concept HasNodeAdd = requires(T &node) {
  node.add(std::declval<std::unique_ptr<Node>>());
};

template <class T>
concept HasEntryAdd = requires(T &entry) {
  entry.add(std::declval<std::unique_ptr<Entry>>());
};

void expect_exception(const char *message, const std::type_info &type,
                      auto &&action) {
  try {
    action();
    ADD_FAILURE() << "expected exception: " << message;
  } catch (const std::exception &error) {
    EXPECT_EQ(typeid(error), type);
    EXPECT_STREQ(error.what(), message);
  }
}

int count_nodes(const Node &node) {
  int total = 1;
  for (std::size_t i = 0; i < node.childCount(); ++i) {
    total += count_nodes(*node.child(i));
  }
  return total;
}

int count_entries(const Entry &entry) {
  int total = 1;
  if (const auto *directory = dynamic_cast<const Directory *>(&entry)) {
    for (std::size_t i = 0; i < directory->childCount(); ++i) {
      total += count_entries(*directory->child(i));
    }
  }
  return total;
}

}  // namespace

TEST(CompositeTest, FileReportsOwnSize) {
  File readme("readme.txt", 12);
  EXPECT_EQ(readme.name(), "readme.txt");
  EXPECT_EQ(readme.bytes(), 12);
  EXPECT_EQ(readme.size(), 12);
  EXPECT_EQ(readme.childCount(), 0U);
  EXPECT_EQ(readme.describe(), "File readme.txt 12");

  File empty("empty.txt", 0);
  EXPECT_EQ(empty.size(), 0);
  EXPECT_EQ(empty.describe(), "File empty.txt 0");
}

TEST(CompositeTest, FolderSumsNestedChildren) {
  Folder docs("docs");
  EXPECT_EQ(docs.size(), 0);
  EXPECT_EQ(docs.childCount(), 0U);
  EXPECT_EQ(docs.describe(), "Folder docs");

  docs.add(std::make_unique<File>("readme.txt", 12));
  auto src = std::make_unique<Folder>("src");
  src->add(std::make_unique<File>("main.cpp", 40));
  src->add(std::make_unique<File>("util.h", 8));
  docs.add(std::move(src));

  const auto *nested = dynamic_cast<const Folder *>(docs.child(1));
  ASSERT_NE(nested, nullptr);
  EXPECT_EQ(nested->size(), 48);
  EXPECT_EQ(docs.size(), 60);
  EXPECT_EQ(docs.childCount(), 2U);
}

TEST(CompositeTest, DescribeKeepsInsertionOrder) {
  Folder docs("docs");
  docs.add(std::make_unique<File>("readme.txt", 12));
  auto src = std::make_unique<Folder>("src");
  src->add(std::make_unique<File>("main.cpp", 40));
  src->add(std::make_unique<File>("util.h", 8));
  docs.add(std::move(src));

  EXPECT_EQ(docs.child(0)->name(), "readme.txt");
  EXPECT_EQ(docs.child(1)->name(), "src");
  const auto *nested = dynamic_cast<const Folder *>(docs.child(1));
  ASSERT_NE(nested, nullptr);
  EXPECT_EQ(nested->child(0)->name(), "main.cpp");
  EXPECT_EQ(nested->child(1)->name(), "util.h");
  EXPECT_EQ(docs.describe(),
            "Folder docs\n"
            "  File readme.txt 12\n"
            "  Folder src\n"
            "    File main.cpp 40\n"
            "    File util.h 8");
}

TEST(CompositeTest, ClientTreatsLeafAndFolderUniformly) {
  File readme("readme.txt", 12);
  Folder docs("docs");
  docs.add(std::make_unique<File>("readme.txt", 12));
  auto src = std::make_unique<Folder>("src");
  src->add(std::make_unique<File>("main.cpp", 40));
  docs.add(std::move(src));

  const Node &as_file = readme;
  const Node &as_folder = docs;
  EXPECT_EQ(count_nodes(as_file), 1);
  EXPECT_EQ(count_nodes(as_folder), 4);
  EXPECT_EQ(as_file.size(), 12);
  EXPECT_EQ(as_folder.size(), 52);
  EXPECT_EQ(as_file.describe(), "File readme.txt 12");
  EXPECT_EQ(as_folder.childCount(), 2U);
}

TEST(CompositeTest, AddThroughNodeReference) {
  Folder docs("docs");
  Node &as_folder = docs;
  as_folder.add(std::make_unique<File>("readme.txt", 12));

  File notes("notes.txt", 4);
  Node &as_file = notes;
  expect_exception("cannot contain children", typeid(std::logic_error), [&] {
    as_file.add(std::make_unique<File>("nope.txt", 1));
  });

  EXPECT_EQ(docs.childCount(), 1U);
  EXPECT_EQ(docs.child(0)->name(), "readme.txt");
  EXPECT_EQ(docs.size(), 12);
  EXPECT_EQ(notes.childCount(), 0U);
  EXPECT_EQ(notes.size(), 4);
}

TEST(CompositeTest, RemoveDetachesIndependentSubtree) {
  Folder docs("docs");
  docs.add(std::make_unique<File>("readme.txt", 12));
  auto src = std::make_unique<Folder>("src");
  src->add(std::make_unique<File>("main.cpp", 40));
  docs.add(std::move(src));
  docs.add(std::make_unique<File>("license.txt", 5));

  Node *src_before = docs.child(1);
  auto removed = docs.remove("src");
  auto *src_folder = dynamic_cast<Folder *>(removed.get());
  ASSERT_NE(src_folder, nullptr);
  EXPECT_EQ(removed.get(), src_before);
  EXPECT_EQ(src_folder->size(), 40);
  EXPECT_EQ(src_folder->describe(), "Folder src\n  File main.cpp 40");

  EXPECT_EQ(docs.size(), 17);
  EXPECT_EQ(docs.childCount(), 2U);
  EXPECT_EQ(docs.child(0)->name(), "readme.txt");
  EXPECT_EQ(docs.child(1)->name(), "license.txt");
  EXPECT_EQ(docs.describe(),
            "Folder docs\n"
            "  File readme.txt 12\n"
            "  File license.txt 5");
}

TEST(CompositeTest, ChildPointerStaysValidAcrossAdds) {
  Folder docs("docs");
  docs.add(std::make_unique<File>("a.txt", 1));
  Node *first = docs.child(0);
  for (int i = 0; i < 32; ++i) {
    docs.add(std::make_unique<File>("f" + std::to_string(i) + ".txt", 1));
  }
  EXPECT_EQ(docs.child(0), first);
  EXPECT_EQ(first->name(), "a.txt");
  EXPECT_EQ(docs.size(), 33);
}

TEST(CompositeTest, SameNameAllowedUnderDifferentParents) {
  Folder docs("docs");
  docs.add(std::make_unique<File>("readme.txt", 12));
  auto src = std::make_unique<Folder>("src");
  src->add(std::make_unique<File>("readme.txt", 3));
  docs.add(std::move(src));

  EXPECT_EQ(docs.size(), 15);
  auto removed = docs.remove("readme.txt");
  const auto *file = dynamic_cast<const File *>(removed.get());
  ASSERT_NE(file, nullptr);
  EXPECT_EQ(file->bytes(), 12);
  EXPECT_EQ(docs.size(), 3);
  EXPECT_EQ(docs.childCount(), 1U);
  EXPECT_EQ(docs.child(0)->name(), "src");
  EXPECT_EQ(dynamic_cast<const Folder *>(docs.child(0))->child(0)->name(),
            "readme.txt");
}

TEST(CompositeTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<Node>);
  static_assert(!std::is_move_constructible_v<Node>);
  static_assert(!std::is_copy_assignable_v<Node>);
  static_assert(!std::is_move_assignable_v<Node>);
  static_assert(!std::is_copy_constructible_v<File>);
  static_assert(!std::is_move_constructible_v<File>);
  static_assert(!std::is_copy_constructible_v<Folder>);
  static_assert(!std::is_move_constructible_v<Folder>);
  static_assert(!std::is_copy_constructible_v<Entry>);
  static_assert(!std::is_move_constructible_v<Entry>);
  static_assert(!std::is_copy_constructible_v<Document>);
  static_assert(!std::is_copy_constructible_v<Directory>);
  static_assert(!std::is_move_constructible_v<Directory>);
}

TEST(CompositeTest, ChildApiVisibility) {
  static_assert(HasNodeAdd<Node>);
  static_assert(HasNodeAdd<File>);
  static_assert(HasNodeAdd<Folder>);
  static_assert(!HasEntryAdd<Entry>);
  static_assert(!HasEntryAdd<Document>);
  static_assert(HasEntryAdd<Directory>);
  static_assert(!HasNodeAdd<Directory>);
  static_assert(!HasEntryAdd<Folder>);
}

TEST(CompositeTest, LeafRejectsChildOperations) {
  File readme("readme.txt", 12);
  expect_exception("cannot contain children", typeid(std::logic_error), [&] {
    readme.add(std::make_unique<File>("nope.txt", 1));
  });
  expect_exception("cannot contain children", typeid(std::logic_error),
                   [&] { (void)readme.remove("nope.txt"); });
  expect_exception("cannot contain children", typeid(std::logic_error),
                   [&] { (void)readme.child(0); });
  EXPECT_EQ(readme.childCount(), 0U);
  EXPECT_EQ(readme.size(), 12);
}

TEST(CompositeTest, ContainersRejectInvalidStructure) {
  Folder folder("docs");
  expect_exception("child is required", typeid(std::invalid_argument),
                   [&] { folder.add(nullptr); });
  folder.add(std::make_unique<File>("readme.txt", 1));
  expect_exception("duplicate child: readme.txt", typeid(std::invalid_argument),
                   [&] { folder.add(std::make_unique<File>("readme.txt", 2)); });
  expect_exception("name is required", typeid(std::invalid_argument),
                   [&] { (void)folder.remove(""); });
  expect_exception("unknown child: missing.txt", typeid(std::invalid_argument),
                   [&] { (void)folder.remove("missing.txt"); });
  expect_exception("child index out of range", typeid(std::out_of_range),
                   [&] { (void)folder.child(1); });
  EXPECT_EQ(folder.childCount(), 1U);

  Directory directory("docs");
  expect_exception("child is required", typeid(std::invalid_argument),
                   [&] { directory.add(nullptr); });
  directory.add(std::make_unique<Document>("readme.txt", 1));
  expect_exception(
      "duplicate child: readme.txt", typeid(std::invalid_argument),
      [&] { directory.add(std::make_unique<Document>("readme.txt", 2)); });
  expect_exception("unknown child: missing.txt", typeid(std::invalid_argument),
                   [&] { (void)directory.remove("missing.txt"); });
  expect_exception("child index out of range", typeid(std::out_of_range),
                   [&] { (void)directory.child(3); });
  EXPECT_EQ(directory.size(), 1);
}

TEST(CompositeTest, ConstructorsRejectInvalidFields) {
  expect_exception("name is required", typeid(std::invalid_argument),
                   [] { File("", 1); });
  expect_exception("bytes must be non-negative", typeid(std::invalid_argument),
                   [] { File("readme.txt", -1); });
  expect_exception("name is required", typeid(std::invalid_argument),
                   [] { Folder(""); });
  expect_exception("name is required", typeid(std::invalid_argument),
                   [] { Document("", 1); });
  expect_exception("bytes must be non-negative", typeid(std::invalid_argument),
                   [] { Document("readme.txt", -1); });
  expect_exception("name is required", typeid(std::invalid_argument),
                   [] { Directory(""); });
}

TEST(CompositeTest, SafeClientMeasuresThroughEntry) {
  Document readme("readme.txt", 12);
  Directory docs("docs");
  docs.add(std::make_unique<Document>("readme.txt", 12));
  auto src = std::make_unique<Directory>("src");
  src->add(std::make_unique<Document>("main.cpp", 40));
  src->add(std::make_unique<Document>("util.h", 8));
  docs.add(std::move(src));

  const Entry &as_doc = readme;
  const Entry &as_dir = docs;
  EXPECT_EQ(as_doc.size(), 12);
  EXPECT_EQ(as_dir.size(), 60);
  EXPECT_EQ(as_doc.describe(), "Document readme.txt 12");
  EXPECT_EQ(as_dir.describe(),
            "Directory docs\n"
            "  Document readme.txt 12\n"
            "  Directory src\n"
            "    Document main.cpp 40\n"
            "    Document util.h 8");

  const auto *nested = dynamic_cast<const Directory *>(docs.child(1));
  ASSERT_NE(nested, nullptr);
  EXPECT_EQ(nested->size(), 48);
}

TEST(CompositeTest, SafeWalkCastsToDirectory) {
  Directory docs("docs");
  docs.add(std::make_unique<Document>("readme.txt", 12));
  auto src = std::make_unique<Directory>("src");
  src->add(std::make_unique<Document>("main.cpp", 40));
  docs.add(std::move(src));

  const Entry &as_docs = docs;
  EXPECT_EQ(count_entries(as_docs), 4);
  EXPECT_EQ(count_entries(*docs.child(0)), 1);
  EXPECT_EQ(dynamic_cast<const Directory *>(docs.child(0)), nullptr);
  EXPECT_NE(dynamic_cast<const Directory *>(docs.child(1)), nullptr);
  EXPECT_EQ(dynamic_cast<Directory *>(docs.child(1))->child(0)->name(),
            "main.cpp");
}
