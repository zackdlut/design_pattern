#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace design_pattern::structural::facade {

// 源文件。只把文件名拆开，不知道编码、滤镜和写盘。
class VideoFile {
public:
  explicit VideoFile(std::string filename);
  const std::string &filename() const;
  const std::string &name() const;
  const std::string &extension() const;

private:
  std::string filename_;
  std::string name_;
  std::string extension_;
};

// 编解码器的名字。扩展名和 codec 可以不一样：mp4 对应 mpeg4。
class Codec {
public:
  explicit Codec(std::string name);
  const std::string &name() const;

private:
  std::string name_;
};

// 扩展名 → 解码器，目标格式 → 编码器。这张表只放在这里。
class CodecFactory {
public:
  Codec extract(const VideoFile &file) const;
  Codec compress(std::string_view format) const;
};

// 按源 codec 读出码流，再按目标 codec 转码。
class BitrateReader {
public:
  std::string read(const VideoFile &file, const Codec &source) const;
  std::string convert(std::string_view buffer, const Codec &destination) const;
};

// 滤镜。预设只有 sharpen / denoise。调不调用由外观决定。
class VideoFilter {
public:
  std::string apply(std::string_view buffer, std::string_view preset) const;
};

// 修音轨。不关心前面刚解码完，还是已经转过码。
class AudioMixer {
public:
  std::string fix(std::string_view buffer) const;
};

// 写成「主文件名.目标格式」。不判断这个格式在不在支持列表里。
class FileWriter {
public:
  std::string save(std::string_view name, std::string_view format,
                   std::string_view buffer) const;
};

// 外观。客户端只调 convert / extractAudio，步骤留在内部。
// 子系统仍是公开类型：要单步微调时直接用它们，不必改外观。
class VideoConverter {
public:
  std::string convert(std::string_view filename,
                      std::string_view format) const;
  std::string convert(std::string_view filename, std::string_view format,
                      std::string_view preset) const;
  std::string extractAudio(std::string_view filename) const;

private:
  std::string convert_to(std::string_view filename, std::string_view format,
                         std::optional<std::string_view> preset) const;

  CodecFactory codecs_;
  BitrateReader reader_;
  VideoFilter filter_;
  AudioMixer mixer_;
  FileWriter writer_;
};

}  // namespace design_pattern::structural::facade
