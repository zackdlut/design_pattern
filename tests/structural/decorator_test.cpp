#include "structural/decorator/decorator.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

using design_pattern::structural::decorator::BufferedStream;
using design_pattern::structural::decorator::CompressedStream;
using design_pattern::structural::decorator::EncryptedStream;
using design_pattern::structural::decorator::FileStream;
using design_pattern::structural::decorator::MemoryStream;
using design_pattern::structural::decorator::Stream;
using design_pattern::structural::decorator::StreamDecorator;

namespace {

void expect_invalid(const char *message, auto &&action) {
  try {
    action();
    FAIL() << "expected std::invalid_argument: " << message;
  } catch (const std::invalid_argument &error) {
    EXPECT_STREQ(error.what(), message);
  }
}

std::string write_on(Stream &stream, std::string_view data) {
  return stream.write(data);
}

std::string read_on(Stream &stream) { return stream.read(); }

// 测试里新增的装饰器：只加一层标记，write / read 仍走基类转发。
// 不改 FileStream，也不改已有装饰器。
class MarkingStream final : public StreamDecorator {
public:
  explicit MarkingStream(std::unique_ptr<Stream> inner)
      : StreamDecorator(std::move(inner)) {}

  std::string write(std::string_view data) override {
    return "MarkingStream -> " + StreamDecorator::write(data);
  }
};

}  // namespace

TEST(DecoratorTest, FileStreamWritesAndReads) {
  FileStream file("a.txt");
  EXPECT_EQ(file.path(), "a.txt");
  EXPECT_EQ(file.read(), "");
  EXPECT_EQ(file.write("hello"), "FileStream(a.txt) writes \"hello\"");
  EXPECT_EQ(file.read(), "hello");
  EXPECT_EQ(file.write("next"), "FileStream(a.txt) writes \"next\"");
  EXPECT_EQ(file.read(), "next");
}

TEST(DecoratorTest, MemoryStreamUsesTheSameInterface) {
  MemoryStream memory;
  EXPECT_EQ(memory.write("hello"), "MemoryStream writes \"hello\"");
  EXPECT_EQ(memory.read(), "hello");
}

TEST(DecoratorTest, EncryptedStreamIsTransparentButStoresCiphertext) {
  auto file = std::make_unique<FileStream>("a.txt");
  FileStream *raw = file.get();
  EncryptedStream encrypted(std::move(file));

  EXPECT_EQ(encrypted.write("hello"),
            "EncryptedStream encrypts -> FileStream(a.txt) writes "
            "\"enc(hello)\"");
  EXPECT_EQ(read_on(encrypted), "hello");
  EXPECT_EQ(raw->read(), "enc(hello)");
}

TEST(DecoratorTest, CompressedStreamIsTransparentButStoresPackedBytes) {
  auto memory = std::make_unique<MemoryStream>();
  MemoryStream *raw = memory.get();
  CompressedStream compressed(std::move(memory));

  EXPECT_EQ(compressed.write("hello"),
            "CompressedStream compresses -> MemoryStream writes "
            "\"zip(hello)\"");
  EXPECT_EQ(compressed.read(), "hello");
  EXPECT_EQ(raw->read(), "zip(hello)");
}

TEST(DecoratorTest, BufferedStreamDefersWriteUntilFlushOrRead) {
  auto file = std::make_unique<FileStream>("a.txt");
  FileStream *raw = file.get();
  BufferedStream buffered(std::move(file));

  EXPECT_EQ(buffered.write("ab"), "BufferedStream buffers \"ab\"");
  EXPECT_EQ(buffered.write("cd"), "BufferedStream buffers \"cd\"");
  EXPECT_EQ(raw->read(), "");
  EXPECT_EQ(buffered.flush(),
            "BufferedStream flushes -> FileStream(a.txt) writes \"abcd\"");
  EXPECT_EQ(raw->read(), "abcd");
  EXPECT_EQ(buffered.flush(), "BufferedStream flushes nothing");

  EXPECT_EQ(buffered.write("ef"), "BufferedStream buffers \"ef\"");
  EXPECT_EQ(raw->read(), "abcd");
  EXPECT_EQ(buffered.read(), "ef");
  EXPECT_EQ(raw->read(), "ef");
}

TEST(DecoratorTest, StackOrderChangesStoredBytes) {
  auto encrypt_outside = std::make_unique<FileStream>("a.txt");
  FileStream *raw_encrypt_outside = encrypt_outside.get();
  std::unique_ptr<Stream> encrypt_then_compress =
      std::make_unique<EncryptedStream>(
          std::make_unique<CompressedStream>(std::move(encrypt_outside)));

  EXPECT_EQ(encrypt_then_compress->write("hi"),
            "EncryptedStream encrypts -> CompressedStream compresses -> "
            "FileStream(a.txt) writes \"zip(enc(hi))\"");
  EXPECT_EQ(encrypt_then_compress->read(), "hi");
  EXPECT_EQ(raw_encrypt_outside->read(), "zip(enc(hi))");

  auto compress_outside = std::make_unique<FileStream>("b.txt");
  FileStream *raw_compress_outside = compress_outside.get();
  std::unique_ptr<Stream> compress_then_encrypt =
      std::make_unique<CompressedStream>(
          std::make_unique<EncryptedStream>(std::move(compress_outside)));

  EXPECT_EQ(compress_then_encrypt->write("hi"),
            "CompressedStream compresses -> EncryptedStream encrypts -> "
            "FileStream(b.txt) writes \"enc(zip(hi))\"");
  EXPECT_EQ(compress_then_encrypt->read(), "hi");
  EXPECT_EQ(raw_compress_outside->read(), "enc(zip(hi))");
}

TEST(DecoratorTest, SameDecoratorCanBeStacked) {
  auto file = std::make_unique<FileStream>("a.txt");
  FileStream *raw = file.get();
  std::unique_ptr<Stream> twice = std::make_unique<EncryptedStream>(
      std::make_unique<EncryptedStream>(std::move(file)));

  EXPECT_EQ(twice->write("hi"),
            "EncryptedStream encrypts -> EncryptedStream encrypts -> "
            "FileStream(a.txt) writes \"enc(enc(hi))\"");
  EXPECT_EQ(twice->read(), "hi");
  EXPECT_EQ(raw->read(), "enc(enc(hi))");
}

TEST(DecoratorTest, OutermostDecoratorRunsBeforeInnerOnes) {
  auto file = std::make_unique<FileStream>("a.txt");
  FileStream *raw = file.get();
  auto buffered =
      std::make_unique<BufferedStream>(std::move(file));
  BufferedStream *buffer = buffered.get();
  std::unique_ptr<Stream> stream =
      std::make_unique<EncryptedStream>(std::move(buffered));

  EXPECT_EQ(stream->write("hi"),
            "EncryptedStream encrypts -> BufferedStream buffers \"enc(hi)\"");
  EXPECT_EQ(raw->read(), "");
  EXPECT_EQ(buffer->flush(),
            "BufferedStream flushes -> FileStream(a.txt) writes \"enc(hi)\"");
  EXPECT_EQ(raw->read(), "enc(hi)");
  EXPECT_EQ(stream->read(), "hi");
}

TEST(DecoratorTest, ClientDependsOnStreamAbstraction) {
  std::unique_ptr<Stream> plain = std::make_unique<FileStream>("a.txt");
  std::unique_ptr<Stream> encrypted = std::make_unique<EncryptedStream>(
      std::make_unique<MemoryStream>());
  std::unique_ptr<Stream> compressed = std::make_unique<CompressedStream>(
      std::make_unique<BufferedStream>(std::make_unique<FileStream>("b.txt")));

  EXPECT_EQ(write_on(*plain, "hi"), "FileStream(a.txt) writes \"hi\"");
  EXPECT_EQ(read_on(*plain), "hi");

  EXPECT_EQ(write_on(*encrypted, "hi"),
            "EncryptedStream encrypts -> MemoryStream writes \"enc(hi)\"");
  EXPECT_EQ(read_on(*encrypted), "hi");

  EXPECT_EQ(write_on(*compressed, "hi"),
            "CompressedStream compresses -> BufferedStream buffers "
            "\"zip(hi)\"");
  EXPECT_EQ(read_on(*compressed), "hi");
}

TEST(DecoratorTest, NewDecoratorForwardsThroughTheBase) {
  auto file = std::make_unique<FileStream>("a.txt");
  FileStream *raw = file.get();
  MarkingStream marking(std::move(file));

  EXPECT_EQ(marking.write("hi"),
            "MarkingStream -> FileStream(a.txt) writes \"hi\"");
  EXPECT_EQ(marking.read(), "hi");
  EXPECT_EQ(raw->read(), "hi");
}

TEST(DecoratorTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<Stream>);
  static_assert(!std::is_move_constructible_v<Stream>);
  static_assert(!std::is_copy_assignable_v<Stream>);
  static_assert(!std::is_move_assignable_v<Stream>);
  static_assert(!std::is_copy_constructible_v<FileStream>);
  static_assert(!std::is_move_constructible_v<FileStream>);
  static_assert(!std::is_copy_constructible_v<MemoryStream>);
  static_assert(!std::is_copy_constructible_v<StreamDecorator>);
  static_assert(!std::is_copy_constructible_v<BufferedStream>);
  static_assert(!std::is_copy_constructible_v<EncryptedStream>);
  static_assert(!std::is_copy_constructible_v<CompressedStream>);
  static_assert(!std::is_move_constructible_v<BufferedStream>);
}

TEST(DecoratorTest, InheritanceAndConvertibility) {
  static_assert(std::is_base_of_v<Stream, FileStream>);
  static_assert(std::is_base_of_v<Stream, MemoryStream>);
  static_assert(std::is_base_of_v<Stream, StreamDecorator>);
  static_assert(std::is_base_of_v<Stream, BufferedStream>);
  static_assert(std::is_base_of_v<Stream, EncryptedStream>);
  static_assert(std::is_base_of_v<Stream, CompressedStream>);
  static_assert(std::is_base_of_v<StreamDecorator, BufferedStream>);
  static_assert(std::is_base_of_v<StreamDecorator, EncryptedStream>);
  static_assert(std::is_base_of_v<StreamDecorator, CompressedStream>);

  static_assert(std::is_convertible_v<FileStream *, Stream *>);
  static_assert(std::is_convertible_v<BufferedStream *, Stream *>);
  static_assert(std::is_convertible_v<EncryptedStream *, Stream *>);
  static_assert(!std::is_base_of_v<FileStream, EncryptedStream>);
  static_assert(!std::is_convertible_v<EncryptedStream *, FileStream *>);
  static_assert(!std::is_convertible_v<BufferedStream *, EncryptedStream *>);
}

TEST(DecoratorTest, RejectsInvalidOperations) {
  expect_invalid("path is required", [] { FileStream file(""); });
  expect_invalid("stream is required",
                 [] { BufferedStream stream(nullptr); });
  expect_invalid("stream is required",
                 [] { EncryptedStream stream(nullptr); });
  expect_invalid("stream is required",
                 [] { CompressedStream stream(nullptr); });

  FileStream file("a.txt");
  expect_invalid("data is required", [&] { file.write(""); });

  MemoryStream memory;
  expect_invalid("data is required", [&] { memory.write(""); });

  BufferedStream buffered(std::make_unique<FileStream>("a.txt"));
  expect_invalid("data is required", [&] { buffered.write(""); });
  EXPECT_EQ(buffered.flush(), "BufferedStream flushes nothing");

  EncryptedStream encrypted(std::make_unique<MemoryStream>());
  expect_invalid("data is required", [&] { encrypted.write(""); });

  auto plain = std::make_unique<FileStream>("a.txt");
  plain->write("hello");
  EncryptedStream corrupt(std::move(plain));
  expect_invalid("invalid enc payload", [&] { corrupt.read(); });

  auto packed = std::make_unique<MemoryStream>();
  packed->write("hello");
  CompressedStream bad_zip(std::move(packed));
  expect_invalid("invalid zip payload", [&] { bad_zip.read(); });
}
