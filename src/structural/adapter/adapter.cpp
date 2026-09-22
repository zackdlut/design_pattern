#include "structural/adapter/adapter.h"

#include <stdexcept>
#include <utility>

namespace design_pattern::structural::adapter {

namespace {

struct MediaFile {
  std::string title;
  std::string extension;
};

MediaFile parse_media_file(std::string_view filename) {
  if (filename.empty()) {
    throw std::invalid_argument("filename is required");
  }

  const auto dot = filename.rfind('.');
  if (dot == std::string_view::npos || dot == 0 ||
      dot == filename.size() - 1) {
    throw std::invalid_argument("filename must have an extension");
  }

  MediaFile file;
  file.title = std::string(filename.substr(0, dot));
  file.extension = std::string(filename.substr(dot + 1));
  if (file.title.empty()) {
    throw std::invalid_argument("title is required");
  }
  return file;
}

void require_format(const MediaFile &file, std::string_view allowed_a,
                    std::string_view allowed_b = {}) {
  if (file.extension == allowed_a) {
    return;
  }
  if (!allowed_b.empty() && file.extension == allowed_b) {
    return;
  }
  throw std::invalid_argument("unsupported format: " + file.extension);
}

std::string require_name(std::string_view name) {
  if (name.empty()) {
    throw std::invalid_argument("player name is required");
  }
  return std::string(name);
}

std::string require_title(std::string_view title) {
  if (title.empty()) {
    throw std::invalid_argument("title is required");
  }
  return std::string(title);
}

}  // namespace

std::string Mp3Player::play(std::string_view filename) const {
  const auto file = parse_media_file(filename);
  require_format(file, "mp3");
  return "Mp3Player plays " + std::string(filename);
}

LegacyPlayer::LegacyPlayer(std::string name) : name_(require_name(name)) {}

std::string LegacyPlayer::playWav(std::string_view title) const {
  return "LegacyPlayer " + name_ + " plays wav: " + require_title(title);
}

const std::string &LegacyPlayer::name() const { return name_; }

VintageWalkman::VintageWalkman() : LegacyPlayer("walkman") {}

std::string VintageWalkman::playWav(std::string_view title) const {
  return LegacyPlayer::playWav(title) + " [tape]";
}

ObjectAdapter::ObjectAdapter(std::unique_ptr<LegacyPlayer> adaptee)
    : adaptee_(std::move(adaptee)) {
  if (!adaptee_) {
    throw std::invalid_argument("adaptee is required");
  }
}

std::string ObjectAdapter::play(std::string_view filename) const {
  const auto file = parse_media_file(filename);
  require_format(file, "mp3", "wav");
  return "ObjectAdapter -> " + adaptee_->playWav(file.title);
}

ClassAdapter::ClassAdapter(std::string player_name)
    : LegacyPlayer(std::move(player_name)) {}

std::string ClassAdapter::play(std::string_view filename) const {
  const auto file = parse_media_file(filename);
  require_format(file, "mp3", "wav");
  return "ClassAdapter -> " + playWav(file.title);
}

}  // namespace design_pattern::structural::adapter
