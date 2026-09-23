#include "structural/proxy/proxy.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

using design_pattern::structural::proxy::CachedImage;
using design_pattern::structural::proxy::GuardedImage;
using design_pattern::structural::proxy::Image;
using design_pattern::structural::proxy::ImageArchive;
using design_pattern::structural::proxy::LazyImage;
using design_pattern::structural::proxy::RealImage;

namespace {

void expect_invalid(const char *message, auto &&action) {
  try {
    action();
    FAIL() << "expected std::invalid_argument: " << message;
  } catch (const std::invalid_argument &error) {
    EXPECT_STREQ(error.what(), message);
  }
}

void expect_denied(auto &&action) {
  try {
    action();
    FAIL() << "expected std::runtime_error: access denied";
  } catch (const std::runtime_error &error) {
    EXPECT_STREQ(error.what(), "access denied");
  }
}

std::string show(Image &image) { return image.display(); }

const char *kCover = "RealImage(cover.png) shows bitmap(cover.png)";

template <typename T>
constexpr bool has_loaded = requires(T &image) { image.loaded(); };

template <typename T>
constexpr bool has_render_count = requires(T &image) { image.render_count(); };

template <typename T>
constexpr bool has_grant = requires(T &image) { image.grant(); };

template <typename T>
constexpr bool has_revoke = requires(T &image) { image.revoke(); };

template <typename T>
constexpr bool has_allowed = requires(T &image) { image.allowed(); };

template <typename T>
constexpr bool has_cached = requires(T &image) { image.cached(); };

}  // namespace

TEST(ProxyTest, ArchiveCountsEachOpen) {
  ImageArchive archive;
  EXPECT_EQ(archive.open_count(), 0);
  EXPECT_EQ(archive.open_count("cover.png"), 0);

  EXPECT_EQ(archive.open("cover.png"), "bitmap(cover.png)");
  EXPECT_EQ(archive.open("page.png"), "bitmap(page.png)");
  EXPECT_EQ(archive.open("cover.png"), "bitmap(cover.png)");

  EXPECT_EQ(archive.open_count(), 3);
  EXPECT_EQ(archive.open_count("cover.png"), 2);
  EXPECT_EQ(archive.open_count("page.png"), 1);
  EXPECT_EQ(archive.open_count("missing.png"), 0);
}

TEST(ProxyTest, RealImageOpensDuringConstructionAndRendersEveryDisplay) {
  ImageArchive archive;
  RealImage image("cover.png", archive);

  EXPECT_EQ(archive.open_count(), 1);
  EXPECT_EQ(archive.open_count("cover.png"), 1);
  EXPECT_EQ(image.filename(), "cover.png");
  EXPECT_EQ(image.render_count(), 0);

  EXPECT_EQ(image.display(), kCover);
  EXPECT_EQ(image.render_count(), 1);
  EXPECT_EQ(archive.open_count(), 1);

  EXPECT_EQ(show(image), kCover);
  EXPECT_EQ(image.render_count(), 2);
  EXPECT_EQ(archive.open_count("cover.png"), 1);
}

TEST(ProxyTest, LazyImageDefersOpenUntilDisplay) {
  ImageArchive archive;
  LazyImage image("cover.png", archive);

  EXPECT_FALSE(image.loaded());
  EXPECT_EQ(image.render_count(), 0);
  EXPECT_EQ(image.filename(), "cover.png");
  EXPECT_EQ(archive.open_count(), 0);

  EXPECT_EQ(image.display(), kCover);
  EXPECT_TRUE(image.loaded());
  EXPECT_EQ(image.render_count(), 1);
  EXPECT_EQ(archive.open_count("cover.png"), 1);

  EXPECT_EQ(show(image), kCover);
  EXPECT_EQ(image.render_count(), 2);
  EXPECT_EQ(archive.open_count(), 1);

  LazyImage untouched("page.png", archive);
  EXPECT_EQ(untouched.filename(), "page.png");
  EXPECT_FALSE(untouched.loaded());
  EXPECT_EQ(archive.open_count("page.png"), 0);
}

TEST(ProxyTest, LazyImageMatchesADirectRealImage) {
  ImageArchive archive;
  RealImage real("cover.png", archive);
  LazyImage lazy("cover.png", archive);

  EXPECT_EQ(archive.open_count("cover.png"), 1);
  EXPECT_EQ(lazy.display(), real.display());
  EXPECT_EQ(archive.open_count("cover.png"), 2);
  EXPECT_EQ(lazy.display(), real.display());
  EXPECT_EQ(archive.open_count("cover.png"), 2);
}

TEST(ProxyTest, SeparateLazyImagesDoNotShareTheRealSubject) {
  ImageArchive archive;
  LazyImage first("same.png", archive);
  LazyImage second("same.png", archive);

  EXPECT_EQ(first.display(), second.display());
  EXPECT_EQ(archive.open_count("same.png"), 2);
  EXPECT_EQ(first.render_count(), 1);
  EXPECT_EQ(second.render_count(), 1);
}

TEST(ProxyTest, GuardedImageHidesDisplayButNotTheFilename) {
  ImageArchive archive;
  auto real = std::make_unique<RealImage>("cover.png", archive);
  RealImage *raw = real.get();
  GuardedImage guarded(std::move(real));

  EXPECT_FALSE(guarded.allowed());
  EXPECT_EQ(guarded.filename(), "cover.png");
  EXPECT_EQ(archive.open_count("cover.png"), 1);
  EXPECT_EQ(raw->render_count(), 0);

  expect_denied([&] { guarded.display(); });
  EXPECT_EQ(raw->render_count(), 0);

  guarded.grant();
  EXPECT_TRUE(guarded.allowed());
  EXPECT_EQ(show(guarded), kCover);
  EXPECT_EQ(raw->render_count(), 1);

  guarded.revoke();
  expect_denied([&] { show(guarded); });
  EXPECT_EQ(raw->render_count(), 1);
  EXPECT_EQ(archive.open_count(), 1);
}

TEST(ProxyTest, GuardedLazyImageDoesNotOpenUntilGranted) {
  ImageArchive archive;
  auto lazy = std::make_unique<LazyImage>("secret.png", archive);
  LazyImage *raw = lazy.get();
  GuardedImage guarded(std::move(lazy));

  EXPECT_EQ(guarded.filename(), "secret.png");
  EXPECT_FALSE(raw->loaded());
  expect_denied([&] { guarded.display(); });
  EXPECT_FALSE(raw->loaded());
  EXPECT_EQ(raw->render_count(), 0);
  EXPECT_EQ(archive.open_count(), 0);

  guarded.grant();
  EXPECT_FALSE(raw->loaded());
  EXPECT_EQ(guarded.display(), "RealImage(secret.png) shows bitmap(secret.png)");
  EXPECT_TRUE(raw->loaded());
  EXPECT_EQ(archive.open_count("secret.png"), 1);
  EXPECT_EQ(raw->render_count(), 1);
}

TEST(ProxyTest, CachedImageForwardsDisplayOnce) {
  ImageArchive archive;
  auto real = std::make_unique<RealImage>("cover.png", archive);
  RealImage *raw = real.get();
  CachedImage cached(std::move(real));

  EXPECT_FALSE(cached.cached());
  EXPECT_EQ(cached.filename(), "cover.png");
  EXPECT_EQ(raw->render_count(), 0);

  EXPECT_EQ(cached.display(), kCover);
  EXPECT_TRUE(cached.cached());
  EXPECT_EQ(raw->render_count(), 1);

  EXPECT_EQ(show(cached), kCover);
  EXPECT_EQ(show(cached), kCover);
  EXPECT_EQ(raw->render_count(), 1);
  EXPECT_EQ(archive.open_count("cover.png"), 1);
}

TEST(ProxyTest, FailedDisplayIsNotCached) {
  ImageArchive archive;
  auto real = std::make_unique<RealImage>("cover.png", archive);
  RealImage *photo = real.get();
  auto guarded = std::make_unique<GuardedImage>(std::move(real));
  GuardedImage *gate = guarded.get();
  CachedImage cached(std::move(guarded));

  expect_denied([&] { cached.display(); });
  EXPECT_FALSE(cached.cached());
  EXPECT_EQ(photo->render_count(), 0);

  gate->grant();
  EXPECT_EQ(cached.display(), kCover);
  EXPECT_TRUE(cached.cached());
  EXPECT_EQ(photo->render_count(), 1);
}

TEST(ProxyTest, ProtectionOutsideCacheStillRejectsAfterAHit) {
  ImageArchive archive;
  auto lazy = std::make_unique<LazyImage>("private.png", archive);
  LazyImage *lazy_raw = lazy.get();
  auto cached = std::make_unique<CachedImage>(std::move(lazy));
  CachedImage *cached_raw = cached.get();
  GuardedImage guarded(std::move(cached));

  expect_denied([&] { guarded.display(); });
  EXPECT_FALSE(lazy_raw->loaded());
  EXPECT_FALSE(cached_raw->cached());
  EXPECT_EQ(archive.open_count("private.png"), 0);

  guarded.grant();
  const std::string first = guarded.display();
  EXPECT_EQ(first, "RealImage(private.png) shows bitmap(private.png)");
  EXPECT_EQ(guarded.display(), first);
  EXPECT_TRUE(lazy_raw->loaded());
  EXPECT_TRUE(cached_raw->cached());
  EXPECT_EQ(archive.open_count("private.png"), 1);
  EXPECT_EQ(lazy_raw->render_count(), 1);

  guarded.revoke();
  expect_denied([&] { guarded.display(); });
  EXPECT_EQ(lazy_raw->render_count(), 1);
  EXPECT_TRUE(cached_raw->cached());
  EXPECT_EQ(archive.open_count(), 1);
}

TEST(ProxyTest, CacheOutsideProtectionServesTheHitAfterRevoke) {
  ImageArchive archive;
  auto real = std::make_unique<RealImage>("banner.png", archive);
  RealImage *raw = real.get();
  auto guarded = std::make_unique<GuardedImage>(std::move(real));
  GuardedImage *gate = guarded.get();
  CachedImage cached(std::move(guarded));

  gate->grant();
  EXPECT_EQ(cached.display(), "RealImage(banner.png) shows bitmap(banner.png)");
  EXPECT_EQ(raw->render_count(), 1);

  gate->revoke();
  EXPECT_FALSE(gate->allowed());
  EXPECT_EQ(cached.display(), "RealImage(banner.png) shows bitmap(banner.png)");
  EXPECT_EQ(raw->render_count(), 1);
}

TEST(ProxyTest, AlbumUsesTheImageInterface) {
  ImageArchive archive;
  RealImage real("cover.png", archive);
  LazyImage lazy("cover.png", archive);

  Image &direct = real;
  Image &proxy = lazy;
  EXPECT_EQ(direct.filename(), proxy.filename());
  EXPECT_EQ(archive.open_count("cover.png"), 1);

  EXPECT_EQ(show(proxy), show(direct));
  EXPECT_EQ(archive.open_count("cover.png"), 2);
}

TEST(ProxyTest, ControlQueriesStayOffTheImageInterface) {
  static_assert(!has_loaded<Image>);
  static_assert(!has_render_count<Image>);
  static_assert(!has_grant<Image>);
  static_assert(!has_revoke<Image>);
  static_assert(!has_allowed<Image>);
  static_assert(!has_cached<Image>);

  static_assert(has_loaded<LazyImage>);
  static_assert(has_render_count<LazyImage>);
  static_assert(has_render_count<RealImage>);
  static_assert(!has_render_count<GuardedImage>);
  static_assert(has_grant<GuardedImage>);
  static_assert(has_revoke<GuardedImage>);
  static_assert(has_allowed<GuardedImage>);
  static_assert(has_cached<CachedImage>);
  static_assert(!has_cached<LazyImage>);
  static_assert(!has_grant<CachedImage>);
}

TEST(ProxyTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<Image>);
  static_assert(!std::is_move_constructible_v<Image>);
  static_assert(!std::is_copy_assignable_v<Image>);
  static_assert(!std::is_move_assignable_v<Image>);

  static_assert(!std::is_copy_constructible_v<ImageArchive>);
  static_assert(!std::is_move_constructible_v<ImageArchive>);
  static_assert(!std::is_copy_constructible_v<RealImage>);
  static_assert(!std::is_move_constructible_v<RealImage>);
  static_assert(!std::is_copy_constructible_v<LazyImage>);
  static_assert(!std::is_move_constructible_v<LazyImage>);
  static_assert(!std::is_copy_constructible_v<GuardedImage>);
  static_assert(!std::is_move_constructible_v<GuardedImage>);
  static_assert(!std::is_copy_constructible_v<CachedImage>);
  static_assert(!std::is_move_constructible_v<CachedImage>);
}

TEST(ProxyTest, InheritanceAndConvertibility) {
  static_assert(std::is_base_of_v<Image, RealImage>);
  static_assert(std::is_base_of_v<Image, LazyImage>);
  static_assert(std::is_base_of_v<Image, GuardedImage>);
  static_assert(std::is_base_of_v<Image, CachedImage>);
  static_assert(std::has_virtual_destructor_v<Image>);

  static_assert(!std::is_base_of_v<RealImage, LazyImage>);
  static_assert(!std::is_base_of_v<RealImage, GuardedImage>);
  static_assert(!std::is_base_of_v<RealImage, CachedImage>);
  static_assert(!std::is_base_of_v<LazyImage, GuardedImage>);
  static_assert(!std::is_base_of_v<GuardedImage, CachedImage>);

  static_assert(std::is_convertible_v<RealImage *, Image *>);
  static_assert(std::is_convertible_v<LazyImage *, Image *>);
  static_assert(std::is_convertible_v<GuardedImage *, Image *>);
  static_assert(std::is_convertible_v<CachedImage *, Image *>);
  static_assert(!std::is_convertible_v<LazyImage *, RealImage *>);
  static_assert(!std::is_convertible_v<GuardedImage *, LazyImage *>);
  static_assert(!std::is_convertible_v<Image *, RealImage *>);
}

TEST(ProxyTest, RejectsInvalidInput) {
  ImageArchive archive;
  expect_invalid("filename is required", [&] { (void)archive.open(""); });
  EXPECT_EQ(archive.open_count(), 0);

  expect_invalid("filename is required", [&] { RealImage image("", archive); });
  expect_invalid("filename is required", [&] { LazyImage image("", archive); });
  EXPECT_EQ(archive.open_count(), 0);

  expect_invalid("image is required", [] { GuardedImage image(nullptr); });
  expect_invalid("image is required", [] { CachedImage image(nullptr); });
}
