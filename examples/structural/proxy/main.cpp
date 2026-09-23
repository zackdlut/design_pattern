#include "structural/proxy/proxy.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using design_pattern::structural::proxy::CachedImage;
using design_pattern::structural::proxy::GuardedImage;
using design_pattern::structural::proxy::Image;
using design_pattern::structural::proxy::ImageArchive;
using design_pattern::structural::proxy::LazyImage;
using design_pattern::structural::proxy::RealImage;

namespace {

std::string show(Image &image) { return image.display(); }

}  // namespace

int main() {
  ImageArchive archive;
  std::cout << std::boolalpha;

  std::cout << "=== 相册：虚代理先只放文件名，打开时才读盘 ===\n";
  LazyImage cover("cover.png", archive);
  LazyImage page("page.png", archive);
  std::cout << "  " << cover.filename() << " loaded=" << cover.loaded()
            << "\n";
  std::cout << "  " << page.filename() << " loaded=" << page.loaded() << "\n";
  std::cout << "  opens=" << archive.open_count() << "\n";

  std::cout << "\n=== 客户端拿的是 Image&，第一次 display 才出现真图 ===\n";
  std::cout << "  " << show(cover) << "\n";
  std::cout << "  opens(cover)=" << archive.open_count("cover.png")
            << " renders=" << cover.render_count() << "\n";
  std::cout << "  " << show(cover) << "\n";
  std::cout << "  opens(cover)=" << archive.open_count("cover.png")
            << " renders=" << cover.render_count() << "\n";
  std::cout << "  page loaded=" << page.loaded()
            << " opens=" << archive.open_count() << "\n";

  std::cout << "\n=== 直接构造真图：还没 display，盘已经读过了 ===\n";
  RealImage eager("cover.png", archive);
  std::cout << "  opens(cover)=" << archive.open_count("cover.png") << "\n";
  const std::string from_real = show(eager);
  const std::string from_proxy = show(cover);
  std::cout << "  real:  " << from_real << "\n";
  std::cout << "  proxy: " << from_proxy << "\n";
  std::cout << "  same text=" << (from_real == from_proxy) << "\n";

  std::cout << "\n=== 保护代理：没权限时 display 到不了内层 ===\n";
  auto secret_lazy = std::make_unique<LazyImage>("secret.png", archive);
  LazyImage *secret = secret_lazy.get();
  GuardedImage locked(std::move(secret_lazy));
  std::cout << "  filename=" << locked.filename()
            << " loaded=" << secret->loaded() << "\n";
  try {
    (void)show(locked);
  } catch (const std::runtime_error &error) {
    std::cout << "  display: " << error.what() << "\n";
  }
  std::cout << "  opens(secret)=" << archive.open_count("secret.png")
            << " loaded=" << secret->loaded() << "\n";
  locked.grant();
  std::cout << "  " << show(locked) << "\n";
  std::cout << "  opens(secret)=" << archive.open_count("secret.png")
            << " renders=" << secret->render_count() << "\n";

  std::cout << "\n=== 缓存代理：第二次 display 不再进入真图 ===\n";
  auto banner = std::make_unique<RealImage>("banner.png", archive);
  RealImage *banner_raw = banner.get();
  CachedImage cached(std::move(banner));
  std::cout << "  " << show(cached) << "\n";
  std::cout << "  " << show(cached) << "\n";
  std::cout << "  renders=" << banner_raw->render_count()
            << " cached=" << cached.cached() << "\n";

  std::cout << "\n=== 权限在外，缓存居中，懒加载在里 ===\n";
  auto inner = std::make_unique<LazyImage>("private.png", archive);
  LazyImage *inner_raw = inner.get();
  auto middle = std::make_unique<CachedImage>(std::move(inner));
  CachedImage *middle_raw = middle.get();
  GuardedImage album(std::move(middle));
  try {
    (void)show(album);
  } catch (const std::runtime_error &error) {
    std::cout << "  before grant: " << error.what()
              << " loaded=" << inner_raw->loaded()
              << " cached=" << middle_raw->cached() << "\n";
  }
  album.grant();
  std::cout << "  " << show(album) << "\n";
  std::cout << "  " << show(album) << "\n";
  std::cout << "  opens=" << archive.open_count("private.png")
            << " renders=" << inner_raw->render_count() << "\n";
  album.revoke();
  try {
    (void)show(album);
  } catch (const std::runtime_error &error) {
    std::cout << "  after revoke: " << error.what()
              << " renders=" << inner_raw->render_count() << "\n";
  }

  std::cout << "\n=== 空文件名在构造时拒绝，不会去读盘 ===\n";
  const int opens = archive.open_count();
  try {
    LazyImage bad("", archive);
    (void)bad;
  } catch (const std::invalid_argument &error) {
    std::cout << "  lazy: " << error.what() << "\n";
  }
  try {
    RealImage bad("", archive);
    (void)bad;
  } catch (const std::invalid_argument &error) {
    std::cout << "  real: " << error.what() << "\n";
  }
  std::cout << "  opens unchanged=" << (archive.open_count() == opens) << "\n";

  return 0;
}
