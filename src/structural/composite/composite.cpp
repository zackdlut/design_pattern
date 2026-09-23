#include "structural/composite/composite.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace design_pattern::structural::composite {

namespace {

std::string require_name(std::string_view name) {
  if (name.empty()) {
    throw std::invalid_argument("name is required");
  }
  return std::string(name);
}

int require_bytes(int bytes) {
  if (bytes < 0) {
    throw std::invalid_argument("bytes must be non-negative");
  }
  return bytes;
}

std::string indent_block(std::string_view block) {
  std::string out;
  bool line_start = true;
  for (char ch : block) {
    if (line_start) {
      out += "  ";
      line_start = false;
    }
    out += ch;
    if (ch == '\n') {
      line_start = true;
    }
  }
  return out;
}

template <typename NodeT>
int total_size(const std::vector<std::unique_ptr<NodeT>> &children) {
  int sum = 0;
  for (const auto &child : children) {
    sum += child->size();
  }
  return sum;
}

template <typename NodeT>
std::string render_tree(std::string_view kind, std::string_view name,
                        const std::vector<std::unique_ptr<NodeT>> &children) {
  std::string out;
  out += kind;
  out += ' ';
  out += name;
  for (const auto &child : children) {
    out += '\n';
    out += indent_block(child->describe());
  }
  return out;
}

template <typename NodeT>
void adopt_child(std::vector<std::unique_ptr<NodeT>> &children,
                 std::unique_ptr<NodeT> child) {
  if (!child) {
    throw std::invalid_argument("child is required");
  }
  const std::string name = child->name();
  if (name.empty()) {
    throw std::invalid_argument("name is required");
  }
  const auto existing = std::find_if(
      children.begin(), children.end(),
      [&](const auto &item) { return item->name() == name; });
  if (existing != children.end()) {
    throw std::invalid_argument("duplicate child: " + name);
  }
  children.push_back(std::move(child));
}

template <typename NodeT>
std::unique_ptr<NodeT> take_child(std::vector<std::unique_ptr<NodeT>> &children,
                                  std::string_view name) {
  if (name.empty()) {
    throw std::invalid_argument("name is required");
  }
  const auto it = std::find_if(
      children.begin(), children.end(),
      [&](const auto &item) { return item->name() == name; });
  if (it == children.end()) {
    throw std::invalid_argument("unknown child: " + std::string(name));
  }
  auto removed = std::move(*it);
  children.erase(it);
  return removed;
}

template <typename NodeT>
const NodeT *child_at(const std::vector<std::unique_ptr<NodeT>> &children,
                      std::size_t index) {
  if (index >= children.size()) {
    throw std::out_of_range("child index out of range");
  }
  return children[index].get();
}

}  // namespace

void Node::add(std::unique_ptr<Node>) {
  throw std::logic_error("cannot contain children");
}

std::unique_ptr<Node> Node::remove(std::string_view) {
  throw std::logic_error("cannot contain children");
}

std::size_t Node::childCount() const { return 0; }

Node *Node::child(std::size_t index) {
  return const_cast<Node *>(std::as_const(*this).child(index));
}

const Node *Node::child(std::size_t) const {
  throw std::logic_error("cannot contain children");
}

File::File(std::string name, int bytes)
    : name_(require_name(name)), bytes_(require_bytes(bytes)) {}

std::string File::name() const { return name_; }

int File::size() const { return bytes_; }

std::string File::describe() const {
  std::string out = "File ";
  out += name_;
  out += ' ';
  out += std::to_string(bytes_);
  return out;
}

int File::bytes() const { return bytes_; }

Folder::Folder(std::string name) : name_(require_name(name)) {}

std::string Folder::name() const { return name_; }

int Folder::size() const { return total_size(children_); }

std::string Folder::describe() const {
  return render_tree("Folder", name_, children_);
}

void Folder::add(std::unique_ptr<Node> child) {
  adopt_child(children_, std::move(child));
}

std::unique_ptr<Node> Folder::remove(std::string_view name) {
  return take_child(children_, name);
}

std::size_t Folder::childCount() const { return children_.size(); }

Node *Folder::child(std::size_t index) {
  return const_cast<Node *>(std::as_const(*this).child(index));
}

const Node *Folder::child(std::size_t index) const {
  return child_at(children_, index);
}

Document::Document(std::string name, int bytes)
    : name_(require_name(name)), bytes_(require_bytes(bytes)) {}

std::string Document::name() const { return name_; }

int Document::size() const { return bytes_; }

std::string Document::describe() const {
  std::string out = "Document ";
  out += name_;
  out += ' ';
  out += std::to_string(bytes_);
  return out;
}

int Document::bytes() const { return bytes_; }

Directory::Directory(std::string name) : name_(require_name(name)) {}

std::string Directory::name() const { return name_; }

int Directory::size() const { return total_size(children_); }

std::string Directory::describe() const {
  return render_tree("Directory", name_, children_);
}

void Directory::add(std::unique_ptr<Entry> child) {
  adopt_child(children_, std::move(child));
}

std::unique_ptr<Entry> Directory::remove(std::string_view name) {
  return take_child(children_, name);
}

std::size_t Directory::childCount() const { return children_.size(); }

Entry *Directory::child(std::size_t index) {
  return const_cast<Entry *>(std::as_const(*this).child(index));
}

const Entry *Directory::child(std::size_t index) const {
  return child_at(children_, index);
}

}  // namespace design_pattern::structural::composite
