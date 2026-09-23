#include "structural/facade/facade.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

using design_pattern::structural::facade::AudioMixer;
using design_pattern::structural::facade::BitrateReader;
using design_pattern::structural::facade::CodecFactory;
using design_pattern::structural::facade::FileWriter;
using design_pattern::structural::facade::VideoConverter;
using design_pattern::structural::facade::VideoFile;
using design_pattern::structural::facade::VideoFilter;

namespace {

// 不用外观时，客户端自己排这六步。顺序错了，落地结果就变了。
std::string convert_by_hand(std::string_view filename, std::string_view format,
                            std::string_view preset = {}) {
  const VideoFile file{std::string(filename)};
  CodecFactory codecs;
  BitrateReader reader;
  VideoFilter filter;
  AudioMixer mixer;
  FileWriter writer;

  const auto source = codecs.extract(file);
  const auto destination = codecs.compress(format);
  std::string buffer = reader.read(file, source);
  if (!preset.empty()) {
    buffer = filter.apply(buffer, preset);
  }
  buffer = reader.convert(buffer, destination);
  buffer = mixer.fix(buffer);
  return writer.save(file.name(), format, buffer);
}

}  // namespace

int main() {
  VideoConverter converter;

  std::cout << "=== 外观：客户端只说「把这个文件转成那种格式」 ===\n";
  std::cout << "  " << converter.convert("demo.mp4", "mp3") << "\n";
  std::cout << "  " << converter.convert("clip.ogg", "mp4") << "\n";

  std::cout << "\n=== 同一套子系统，手写流水线得到同样的结果 ===\n";
  std::cout << "  facade: " << converter.convert("demo.mp4", "mp3") << "\n";
  std::cout << "  manual: " << convert_by_hand("demo.mp4", "mp3") << "\n";

  std::cout << "\n=== 滤镜是可选的一步，夹在解码和编码之间 ===\n";
  std::cout << "  " << converter.convert("demo.mp4", "mp3", "sharpen") << "\n";
  std::cout << "  " << converter.convert("clip.ogg", "ogg", "denoise") << "\n";

  std::cout << "\n=== 抽音轨是另一条粗粒度用例，不走滤镜和转码 ===\n";
  std::cout << "  " << converter.extractAudio("demo.mp4") << "\n";
  std::cout << "  " << converter.extractAudio("my.clip.ogg") << "\n";

  std::cout << "\n=== 写盘子系统不拦格式；外观的用例会拦 ===\n";
  FileWriter writer;
  std::cout << "  writer: " << writer.save("demo", "xyz", "raw") << "\n";
  try {
    (void)converter.convert("demo.mp4", "xyz");
  } catch (const std::invalid_argument &error) {
    std::cout << "  facade: " << error.what() << "\n";
  }

  std::cout << "\n=== 错误沿着流水线抛：文件名、源格式、目标格式、滤镜 ===\n";
  try {
    (void)converter.convert("", "mp3");
  } catch (const std::invalid_argument &error) {
    std::cout << "  filename: " << error.what() << "\n";
  }
  try {
    (void)converter.convert("demo.avi", "mp3");
  } catch (const std::invalid_argument &error) {
    std::cout << "  source: " << error.what() << "\n";
  }
  try {
    (void)converter.convert("demo.mp4", "gif");
  } catch (const std::invalid_argument &error) {
    std::cout << "  format: " << error.what() << "\n";
  }
  try {
    (void)converter.convert("demo.mp4", "mp3", "blur");
  } catch (const std::invalid_argument &error) {
    std::cout << "  filter: " << error.what() << "\n";
  }

  return 0;
}
