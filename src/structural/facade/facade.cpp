#include "structural/facade/facade.h"

#include <stdexcept>
#include <utility>

namespace design_pattern::structural::facade {

namespace {

std::string require_text(std::string_view value, const char *message) {
  if (value.empty()) {
    throw std::invalid_argument(message);
  }
  return std::string(value);
}

std::string source_codec(std::string_view extension) {
  if (extension == "mp4") {
    return "mpeg4";
  }
  if (extension == "ogg") {
    return "ogg";
  }
  throw std::invalid_argument("unsupported source: " + std::string(extension));
}

std::string destination_codec(std::string_view format) {
  if (format.empty()) {
    throw std::invalid_argument("format is required");
  }
  if (format == "mp4") {
    return "mpeg4";
  }
  if (format == "ogg" || format == "mp3") {
    return std::string(format);
  }
  throw std::invalid_argument("unsupported format: " + std::string(format));
}

}  // namespace

VideoFile::VideoFile(std::string filename) {
  if (filename.empty()) {
    throw std::invalid_argument("filename is required");
  }

  const auto dot = filename.rfind('.');
  if (dot == std::string::npos || dot == 0 || dot + 1 == filename.size()) {
    throw std::invalid_argument("filename must have an extension");
  }

  filename_ = std::move(filename);
  name_ = filename_.substr(0, dot);
  extension_ = filename_.substr(dot + 1);
}

const std::string &VideoFile::filename() const { return filename_; }

const std::string &VideoFile::name() const { return name_; }

const std::string &VideoFile::extension() const { return extension_; }

Codec::Codec(std::string name) : name_(require_text(name, "codec is required")) {}

const std::string &Codec::name() const { return name_; }

Codec CodecFactory::extract(const VideoFile &file) const {
  return Codec(source_codec(file.extension()));
}

Codec CodecFactory::compress(std::string_view format) const {
  return Codec(destination_codec(format));
}

std::string BitrateReader::read(const VideoFile &file,
                                const Codec &source) const {
  return "raw(" + file.filename() + ":" + source.name() + ")";
}

std::string BitrateReader::convert(std::string_view buffer,
                                   const Codec &destination) const {
  const auto payload = require_text(buffer, "buffer is required");
  return "encoded[" + destination.name() + "](" + payload + ")";
}

std::string VideoFilter::apply(std::string_view buffer,
                               std::string_view preset) const {
  const auto payload = require_text(buffer, "buffer is required");
  if (preset.empty()) {
    throw std::invalid_argument("filter preset is required");
  }
  if (preset != "sharpen" && preset != "denoise") {
    throw std::invalid_argument("unknown filter: " + std::string(preset));
  }
  return std::string(preset) + "(" + payload + ")";
}

std::string AudioMixer::fix(std::string_view buffer) const {
  return "mixed(" + require_text(buffer, "buffer is required") + ")";
}

std::string FileWriter::save(std::string_view name, std::string_view format,
                             std::string_view buffer) const {
  const auto stem = require_text(name, "name is required");
  const auto extension = require_text(format, "format is required");
  const auto payload = require_text(buffer, "buffer is required");
  return "saved " + stem + "." + extension + " <- " + payload;
}

std::string VideoConverter::convert(std::string_view filename,
                                    std::string_view format) const {
  return convert_to(filename, format, std::nullopt);
}

std::string VideoConverter::convert(std::string_view filename,
                                    std::string_view format,
                                    std::string_view preset) const {
  return convert_to(filename, format, preset);
}

std::string VideoConverter::extractAudio(std::string_view filename) const {
  const VideoFile file{std::string(filename)};
  const Codec source = codecs_.extract(file);
  const std::string buffer = mixer_.fix(reader_.read(file, source));
  return writer_.save(file.name(), "mp3", buffer);
}

std::string VideoConverter::convert_to(
    std::string_view filename, std::string_view format,
    std::optional<std::string_view> preset) const {
  const VideoFile file{std::string(filename)};
  const Codec source = codecs_.extract(file);
  const Codec destination = codecs_.compress(format);

  std::string buffer = reader_.read(file, source);
  if (preset.has_value()) {
    buffer = filter_.apply(buffer, *preset);
  }
  buffer = reader_.convert(buffer, destination);
  buffer = mixer_.fix(buffer);
  return writer_.save(file.name(), format, buffer);
}

}  // namespace design_pattern::structural::facade
