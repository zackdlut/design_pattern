#include "structural/facade/facade.h"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <type_traits>

using design_pattern::structural::facade::AudioMixer;
using design_pattern::structural::facade::BitrateReader;
using design_pattern::structural::facade::Codec;
using design_pattern::structural::facade::CodecFactory;
using design_pattern::structural::facade::FileWriter;
using design_pattern::structural::facade::VideoConverter;
using design_pattern::structural::facade::VideoFile;
using design_pattern::structural::facade::VideoFilter;

namespace {

void expect_invalid(const char *message, auto &&action) {
  try {
    action();
    FAIL() << "expected std::invalid_argument: " << message;
  } catch (const std::invalid_argument &error) {
    EXPECT_STREQ(error.what(), message);
  }
}

std::string convert_by_hand(std::string_view filename, std::string_view format,
                            std::string_view preset = {}) {
  const VideoFile file{std::string(filename)};
  CodecFactory codecs;
  BitrateReader reader;
  VideoFilter filter;
  AudioMixer mixer;
  FileWriter writer;

  const Codec source = codecs.extract(file);
  const Codec destination = codecs.compress(format);
  std::string buffer = reader.read(file, source);
  if (!preset.empty()) {
    buffer = filter.apply(buffer, preset);
  }
  buffer = reader.convert(buffer, destination);
  buffer = mixer.fix(buffer);
  return writer.save(file.name(), format, buffer);
}

}  // namespace

TEST(FacadeTest, SubsystemsKeepTheirOwnInterfaces) {
  const VideoFile file("my.demo.mp4");
  EXPECT_EQ(file.filename(), "my.demo.mp4");
  EXPECT_EQ(file.name(), "my.demo");
  EXPECT_EQ(file.extension(), "mp4");

  CodecFactory codecs;
  EXPECT_EQ(codecs.extract(file).name(), "mpeg4");
  EXPECT_EQ(codecs.extract(VideoFile("clip.ogg")).name(), "ogg");
  EXPECT_EQ(codecs.compress("mp4").name(), "mpeg4");
  EXPECT_EQ(codecs.compress("ogg").name(), "ogg");
  EXPECT_EQ(codecs.compress("mp3").name(), "mp3");

  const Codec source = codecs.extract(file);
  const Codec destination("mp3");
  BitrateReader reader;
  const std::string raw = reader.read(file, source);
  EXPECT_EQ(raw, "raw(my.demo.mp4:mpeg4)");
  EXPECT_EQ(reader.convert(raw, destination),
            "encoded[mp3](raw(my.demo.mp4:mpeg4))");

  VideoFilter filter;
  EXPECT_EQ(filter.apply(raw, "sharpen"), "sharpen(raw(my.demo.mp4:mpeg4))");
  EXPECT_EQ(filter.apply(raw, "denoise"), "denoise(raw(my.demo.mp4:mpeg4))");

  AudioMixer mixer;
  EXPECT_EQ(mixer.fix(raw), "mixed(raw(my.demo.mp4:mpeg4))");

  // 写盘不认「支持的格式」这张表。xyz 也能写；外观自己会拒绝。
  FileWriter writer;
  EXPECT_EQ(writer.save(file.name(), "xyz", raw),
            "saved my.demo.xyz <- raw(my.demo.mp4:mpeg4)");
}

TEST(FacadeTest, ConvertHidesThePipeline) {
  VideoConverter converter;
  EXPECT_EQ(converter.convert("demo.mp4", "mp3"),
            "saved demo.mp3 <- mixed(encoded[mp3](raw(demo.mp4:mpeg4)))");
  EXPECT_EQ(converter.convert("clip.ogg", "mp4"),
            "saved clip.mp4 <- mixed(encoded[mpeg4](raw(clip.ogg:ogg)))");
  EXPECT_EQ(converter.convert("demo.mp4", "mp4"),
            "saved demo.mp4 <- mixed(encoded[mpeg4](raw(demo.mp4:mpeg4)))");
}

TEST(FacadeTest, ManualPipelineMatchesFacade) {
  VideoConverter converter;
  EXPECT_EQ(converter.convert("demo.mp4", "mp3"),
            convert_by_hand("demo.mp4", "mp3"));
  EXPECT_EQ(converter.convert("clip.ogg", "mp4", "denoise"),
            convert_by_hand("clip.ogg", "mp4", "denoise"));
}

TEST(FacadeTest, FilterRunsAfterDecodeAndBeforeEncode) {
  VideoConverter converter;
  EXPECT_EQ(converter.convert("demo.mp4", "mp3", "sharpen"),
            "saved demo.mp3 <- "
            "mixed(encoded[mp3](sharpen(raw(demo.mp4:mpeg4))))");
  EXPECT_EQ(converter.convert("clip.ogg", "ogg", "denoise"),
            "saved clip.ogg <- "
            "mixed(encoded[ogg](denoise(raw(clip.ogg:ogg))))");
}

TEST(FacadeTest, ExtractAudioSkipsFilterAndEncode) {
  VideoConverter converter;
  EXPECT_EQ(converter.extractAudio("demo.mp4"),
            "saved demo.mp3 <- mixed(raw(demo.mp4:mpeg4))");
  EXPECT_EQ(converter.extractAudio("my.clip.ogg"),
            "saved my.clip.mp3 <- mixed(raw(my.clip.ogg:ogg))");
}

TEST(FacadeTest, RepeatedCallsDoNotDependOnPreviousOnes) {
  VideoConverter converter;
  EXPECT_EQ(converter.convert("a.mp4", "mp3"), converter.convert("a.mp4", "mp3"));
  EXPECT_EQ(converter.convert("b.ogg", "mp4", "sharpen"),
            VideoConverter{}.convert("b.ogg", "mp4", "sharpen"));
  EXPECT_EQ(converter.extractAudio("a.ogg"),
            VideoConverter{}.extractAudio("a.ogg"));
}

TEST(FacadeTest, FacadeIsNotASubsystemType) {
  static_assert(!std::is_base_of_v<CodecFactory, VideoConverter>);
  static_assert(!std::is_base_of_v<BitrateReader, VideoConverter>);
  static_assert(!std::is_base_of_v<VideoFilter, VideoConverter>);
  static_assert(!std::is_base_of_v<FileWriter, VideoConverter>);
  static_assert(!std::is_convertible_v<const VideoConverter *, const CodecFactory *>);
}

TEST(FacadeTest, SubsystemsRejectBadInput) {
  expect_invalid("filename is required", [] { VideoFile{""}; });
  expect_invalid("filename must have an extension", [] { VideoFile{"demo"}; });
  expect_invalid("filename must have an extension", [] { VideoFile{".mp4"}; });
  expect_invalid("filename must have an extension", [] { VideoFile{"demo."}; });
  expect_invalid("codec is required", [] { Codec{""}; });

  CodecFactory codecs;
  expect_invalid("unsupported source: avi",
                 [] { CodecFactory{}.extract(VideoFile{"demo.avi"}); });
  expect_invalid("unsupported source: mp3",
                 [] { CodecFactory{}.extract(VideoFile{"demo.mp3"}); });
  expect_invalid("format is required", [] { CodecFactory{}.compress(""); });
  expect_invalid("unsupported format: gif",
                 [] { CodecFactory{}.compress("gif"); });

  const VideoFile file("demo.mp4");
  const Codec source = codecs.extract(file);
  BitrateReader reader;
  expect_invalid("buffer is required",
                 [&] { reader.convert("", source); });

  VideoFilter filter;
  expect_invalid("buffer is required", [&] { filter.apply("", "sharpen"); });
  expect_invalid("filter preset is required",
                 [&] { filter.apply("raw", ""); });
  expect_invalid("unknown filter: blur",
                 [&] { filter.apply("raw", "blur"); });

  expect_invalid("buffer is required", [] { AudioMixer{}.fix(""); });
  expect_invalid("name is required",
                 [] { FileWriter{}.save("", "mp3", "raw"); });
  expect_invalid("format is required",
                 [] { FileWriter{}.save("demo", "", "raw"); });
  expect_invalid("buffer is required",
                 [] { FileWriter{}.save("demo", "mp3", ""); });
}

TEST(FacadeTest, FacadeValidatesInPipelineOrder) {
  VideoConverter converter;

  expect_invalid("filename is required", [&] { converter.convert("", "mp3"); });
  expect_invalid("filename must have an extension",
                 [&] { converter.convert("demo", "mp3"); });
  expect_invalid("unsupported source: avi",
                 [&] { converter.convert("demo.avi", "mp3"); });
  expect_invalid("format is required",
                 [&] { converter.convert("demo.mp4", ""); });
  expect_invalid("unsupported format: gif",
                 [&] { converter.convert("demo.mp4", "gif"); });
  expect_invalid("unsupported format: xyz",
                 [&] { converter.convert("demo.mp4", "xyz"); });

  // 文件名先于格式，格式先于滤镜。未知滤镜的文案就是 VideoFilter 自己的。
  expect_invalid("filename is required",
                 [&] { converter.convert("", "gif", "blur"); });
  expect_invalid("unsupported format: gif",
                 [&] { converter.convert("demo.mp4", "gif", "blur"); });
  expect_invalid("filter preset is required",
                 [&] { converter.convert("demo.mp4", "mp3", ""); });
  expect_invalid("unknown filter: blur",
                 [&] { converter.convert("demo.mp4", "mp3", "blur"); });

  expect_invalid("unsupported source: mp3",
                 [&] { converter.extractAudio("demo.mp3"); });
}
