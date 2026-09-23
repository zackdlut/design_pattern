#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace design_pattern::structural::proxy {

// 图库。open 才读盘，并按文件名计数。
// 不能拷贝、不能移动：虚代理保存的是对这一份的引用。
class ImageArchive {
public:
  ImageArchive() = default;
  ImageArchive(const ImageArchive &) = delete;
  ImageArchive &operator=(const ImageArchive &) = delete;
  ImageArchive(ImageArchive &&) = delete;
  ImageArchive &operator=(ImageArchive &&) = delete;

  std::string open(std::string_view filename);
  int open_count() const;
  int open_count(std::string_view filename) const;

private:
  int total_ = 0;
  std::map<std::string, int> by_name_;
};

// Subject。相册只认这两个操作。真图和各种代理都能放进 Image&。
class Image {
public:
  Image() = default;
  virtual ~Image() = default;
  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;
  Image(Image &&) = delete;
  Image &operator=(Image &&) = delete;

  virtual std::string display() = 0;
  virtual const std::string &filename() const = 0;
};

// RealSubject。构造函数里立刻 open。之后每次 display 都重新绘制。
class RealImage final : public Image {
public:
  RealImage(std::string filename, ImageArchive &archive);
  std::string display() override;
  const std::string &filename() const override;
  int render_count() const;

private:
  std::string filename_;
  std::string pixels_;
  int render_count_ = 0;
};

// 虚代理。构造时只留下文件名；第一次 display 才创建 RealImage。
// loaded / render_count 不在 Image 上。只拿接口的客户端不关心图在不在内存里。
// archive 必须比本对象活得更久。
class LazyImage final : public Image {
public:
  LazyImage(std::string filename, ImageArchive &archive);
  std::string display() override;
  const std::string &filename() const override;
  bool loaded() const;
  int render_count() const;

private:
  std::string filename_;
  ImageArchive &archive_;
  std::unique_ptr<RealImage> real_;
};

// 保护代理。未授权时 display 直接拒绝，内层不会被调用。
// filename 仍然转发：相册可以列出名字，画面另算。
class GuardedImage final : public Image {
public:
  explicit GuardedImage(std::unique_ptr<Image> inner);
  std::string display() override;
  const std::string &filename() const override;
  void grant();
  void revoke();
  bool allowed() const;

private:
  std::unique_ptr<Image> inner_;
  bool allowed_ = false;
};

// 缓存代理。第一次 display 的结果留下来，之后不再进入内层。
// 返回值和内层那一次相同，调用方看不出这次有没有画到真图。
class CachedImage final : public Image {
public:
  explicit CachedImage(std::unique_ptr<Image> inner);
  std::string display() override;
  const std::string &filename() const override;
  bool cached() const;

private:
  std::unique_ptr<Image> inner_;
  std::optional<std::string> cached_;
};

}  // namespace design_pattern::structural::proxy
