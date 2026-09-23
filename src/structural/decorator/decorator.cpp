#include "structural/decorator/decorator.h"

#include <stdexcept>
#include <utility>

namespace design_pattern::structural::decorator {

namespace {

std::string require_path(std::string_view path) {
  if (path.empty()) {
    throw std::invalid_argument("path is required");
  }
  return std::string(path);
}

std::string require_data(std::string_view data) {
  if (data.empty()) {
    throw std::invalid_argument("data is required");
  }
  return std::string(data);
}

std::string quote(std::string_view data) {
  return "\"" + std::string(data) + "\"";
}

std::string wrap(std::string_view tag, std::string_view data) {
  return std::string(tag) + "(" + std::string(data) + ")";
}

std::string unwrap(std::string_view stored, std::string_view tag) {
  if (stored.empty()) {
    return {};
  }

  const std::string prefix = std::string(tag) + "(";
  const bool wrapped = stored.size() >= prefix.size() + 1 &&
                       stored.starts_with(prefix) && stored.back() == ')';
  if (!wrapped) {
    throw std::invalid_argument("invalid " + std::string(tag) + " payload");
  }
  return std::string(
      stored.substr(prefix.size(), stored.size() - prefix.size() - 1));
}

}  // namespace

FileStream::FileStream(std::string path) : path_(require_path(path)) {}

std::string FileStream::write(std::string_view data) {
  content_ = require_data(data);
  return "FileStream(" + path_ + ") writes " + quote(content_);
}

std::string FileStream::read() { return content_; }

const std::string &FileStream::path() const { return path_; }

std::string MemoryStream::write(std::string_view data) {
  content_ = require_data(data);
  return "MemoryStream writes " + quote(content_);
}

std::string MemoryStream::read() { return content_; }

StreamDecorator::StreamDecorator(std::unique_ptr<Stream> inner)
    : inner_(std::move(inner)) {
  if (!inner_) {
    throw std::invalid_argument("stream is required");
  }
}

Stream &StreamDecorator::inner() { return *inner_; }

const Stream &StreamDecorator::inner() const { return *inner_; }

std::string StreamDecorator::write(std::string_view data) {
  return inner().write(data);
}

std::string StreamDecorator::read() { return inner().read(); }

BufferedStream::BufferedStream(std::unique_ptr<Stream> inner)
    : StreamDecorator(std::move(inner)) {}

std::string BufferedStream::write(std::string_view data) {
  buffer_.append(require_data(data));
  return "BufferedStream buffers " + quote(data);
}

std::string BufferedStream::read() {
  if (!buffer_.empty()) {
    (void)flush();
  }
  return inner().read();
}

std::string BufferedStream::flush() {
  if (buffer_.empty()) {
    return "BufferedStream flushes nothing";
  }

  const std::string payload = buffer_;
  const std::string trace = inner().write(payload);
  buffer_.clear();
  return "BufferedStream flushes -> " + trace;
}

EncryptedStream::EncryptedStream(std::unique_ptr<Stream> inner)
    : StreamDecorator(std::move(inner)) {}

std::string EncryptedStream::write(std::string_view data) {
  const std::string cipher = wrap("enc", require_data(data));
  return "EncryptedStream encrypts -> " + inner().write(cipher);
}

std::string EncryptedStream::read() { return unwrap(inner().read(), "enc"); }

CompressedStream::CompressedStream(std::unique_ptr<Stream> inner)
    : StreamDecorator(std::move(inner)) {}

std::string CompressedStream::write(std::string_view data) {
  const std::string packed = wrap("zip", require_data(data));
  return "CompressedStream compresses -> " + inner().write(packed);
}

std::string CompressedStream::read() { return unwrap(inner().read(), "zip"); }

}  // namespace design_pattern::structural::decorator
