#include "structural/proxy/proxy.h"

#include <stdexcept>
#include <utility>

namespace design_pattern::structural::proxy {

namespace {

std::string require_filename(std::string_view filename) {
  if (filename.empty()) {
    throw std::invalid_argument("filename is required");
  }
  return std::string(filename);
}

std::unique_ptr<Image> require_image(std::unique_ptr<Image> image) {
  if (!image) {
    throw std::invalid_argument("image is required");
  }
  return image;
}

}  // namespace

std::string ImageArchive::open(std::string_view filename) {
  const std::string name = require_filename(filename);
  ++total_;
  ++by_name_[name];
  return "bitmap(" + name + ")";
}

int ImageArchive::open_count() const { return total_; }

int ImageArchive::open_count(std::string_view filename) const {
  const auto found = by_name_.find(std::string(filename));
  if (found == by_name_.end()) {
    return 0;
  }
  return found->second;
}

RealImage::RealImage(std::string filename, ImageArchive &archive)
    : filename_(require_filename(filename)), pixels_(archive.open(filename_)) {}

std::string RealImage::display() {
  ++render_count_;
  return "RealImage(" + filename_ + ") shows " + pixels_;
}

const std::string &RealImage::filename() const { return filename_; }

int RealImage::render_count() const { return render_count_; }

LazyImage::LazyImage(std::string filename, ImageArchive &archive)
    : filename_(require_filename(filename)), archive_(archive) {}

std::string LazyImage::display() {
  if (!real_) {
    real_ = std::make_unique<RealImage>(filename_, archive_);
  }
  return real_->display();
}

const std::string &LazyImage::filename() const { return filename_; }

bool LazyImage::loaded() const { return static_cast<bool>(real_); }

int LazyImage::render_count() const {
  if (!real_) {
    return 0;
  }
  return real_->render_count();
}

GuardedImage::GuardedImage(std::unique_ptr<Image> inner)
    : inner_(require_image(std::move(inner))) {}

std::string GuardedImage::display() {
  if (!allowed_) {
    throw std::runtime_error("access denied");
  }
  return inner_->display();
}

const std::string &GuardedImage::filename() const { return inner_->filename(); }

void GuardedImage::grant() { allowed_ = true; }

void GuardedImage::revoke() { allowed_ = false; }

bool GuardedImage::allowed() const { return allowed_; }

CachedImage::CachedImage(std::unique_ptr<Image> inner)
    : inner_(require_image(std::move(inner))) {}

std::string CachedImage::display() {
  if (!cached_) {
    cached_ = inner_->display();
  }
  return *cached_;
}

const std::string &CachedImage::filename() const { return inner_->filename(); }

bool CachedImage::cached() const { return cached_.has_value(); }

}  // namespace design_pattern::structural::proxy
