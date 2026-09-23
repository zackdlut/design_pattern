#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace design_pattern::structural::composite {

// 透明组合的 Component。叶子和容器都是 Node，客户端只认这个接口。
// add / remove 也在这里：拿着 Node& 就能组树。叶子不覆盖，走基类默认实现并抛异常。
class Node {
public:
  Node() = default;
  virtual ~Node() = default;
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;
  Node(Node &&) = delete;
  Node &operator=(Node &&) = delete;

  virtual std::string name() const = 0;
  virtual int size() const = 0;
  virtual std::string describe() const = 0;

  virtual void add(std::unique_ptr<Node> child);
  virtual std::unique_ptr<Node> remove(std::string_view name);
  virtual std::size_t childCount() const;
  virtual Node *child(std::size_t index);
  virtual const Node *child(std::size_t index) const;
};

// Leaf。大小就是自己的字节数，没有子节点。
class File final : public Node {
public:
  File(std::string name, int bytes);

  std::string name() const override;
  int size() const override;
  std::string describe() const override;

  int bytes() const;

private:
  std::string name_;
  int bytes_;
};

// Composite。拥有若干 Node，size / describe 递归到整棵子树。
class Folder final : public Node {
public:
  explicit Folder(std::string name);

  std::string name() const override;
  int size() const override;
  std::string describe() const override;

  void add(std::unique_ptr<Node> child) override;
  std::unique_ptr<Node> remove(std::string_view name) override;
  std::size_t childCount() const override;
  Node *child(std::size_t index) override;
  const Node *child(std::size_t index) const override;

private:
  std::string name_;
  std::vector<std::unique_ptr<Node>> children_;
};

// 安全组合的 Component。只放叶子和容器都说得通的操作。
// add / remove 不在这里，Document 编译期就没有这些方法。
class Entry {
public:
  Entry() = default;
  virtual ~Entry() = default;
  Entry(const Entry &) = delete;
  Entry &operator=(const Entry &) = delete;
  Entry(Entry &&) = delete;
  Entry &operator=(Entry &&) = delete;

  virtual std::string name() const = 0;
  virtual int size() const = 0;
  virtual std::string describe() const = 0;
};

class Document final : public Entry {
public:
  Document(std::string name, int bytes);

  std::string name() const override;
  int size() const override;
  std::string describe() const override;

  int bytes() const;

private:
  std::string name_;
  int bytes_;
};

// 安全组合的容器。子节点操作是普通成员，不进 Entry 的虚接口。
class Directory final : public Entry {
public:
  explicit Directory(std::string name);

  std::string name() const override;
  int size() const override;
  std::string describe() const override;

  void add(std::unique_ptr<Entry> child);
  std::unique_ptr<Entry> remove(std::string_view name);
  std::size_t childCount() const;
  Entry *child(std::size_t index);
  const Entry *child(std::size_t index) const;

private:
  std::string name_;
  std::vector<std::unique_ptr<Entry>> children_;
};

}  // namespace design_pattern::structural::composite
