#include "structural/decorator/decorator.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

using design_pattern::structural::decorator::BufferedStream;
using design_pattern::structural::decorator::CompressedStream;
using design_pattern::structural::decorator::EncryptedStream;
using design_pattern::structural::decorator::FileStream;
using design_pattern::structural::decorator::MemoryStream;
using design_pattern::structural::decorator::Stream;

namespace {

// 客户端只依赖 Component。手里是 Stream&，不知道外面套了几层。
void round_trip(Stream &stream, std::string_view data) {
  std::cout << "  write: " << stream.write(data) << "\n";
  std::cout << "  read:  " << stream.read() << "\n";
}

}  // namespace

int main() {
  std::cout << "=== 装饰器：核心流不变，按需叠上缓冲 / 加密 / 压缩 ===\n";

  FileStream plain("notes.txt");
  std::cout << "[裸 FileStream]\n";
  round_trip(plain, "hello");

  std::cout << "\n=== 同一抽象 Stream&，换的是职责组合，不是子类爆炸 ===\n";
  std::unique_ptr<Stream> memory = std::make_unique<EncryptedStream>(
      std::make_unique<MemoryStream>());
  std::unique_ptr<Stream> file = std::make_unique<CompressedStream>(
      std::make_unique<FileStream>("archive.txt"));

  std::cout << "[Encrypted + MemoryStream]\n";
  round_trip(*memory, "hello");
  std::cout << "[Compressed + FileStream]\n";
  round_trip(*file, "hello");

  std::cout << "\n=== 后包上的先处理 write：加密在外，缓冲在内 ===\n";
  auto raw_file = std::make_unique<FileStream>("a.txt");
  FileStream *raw = raw_file.get();
  auto buffered = std::make_unique<BufferedStream>(std::move(raw_file));
  BufferedStream *buffer = buffered.get();
  std::unique_ptr<Stream> stacked =
      std::make_unique<EncryptedStream>(std::move(buffered));

  std::cout << "  write: " << stacked->write("hi") << "\n";
  std::cout << "  file 还是空的: \"" << raw->read() << "\"\n";
  std::cout << "  flush: " << buffer->flush() << "\n";
  std::cout << "  file 落地的是密文: \"" << raw->read() << "\"\n";
  std::cout << "  客户端 read 仍是原文: " << stacked->read() << "\n";

  std::cout << "\n=== 叠加顺序会改变落地字节，读回来仍是原文 ===\n";
  auto encrypt_outside_file = std::make_unique<FileStream>("order-a.txt");
  FileStream *order_a = encrypt_outside_file.get();
  std::unique_ptr<Stream> encrypt_outside =
      std::make_unique<EncryptedStream>(std::make_unique<CompressedStream>(
          std::move(encrypt_outside_file)));
  std::cout << "[外层加密：先加密，再压缩]\n";
  round_trip(*encrypt_outside, "hi");
  std::cout << "  stored: " << order_a->read() << "\n";

  auto compress_outside_file = std::make_unique<FileStream>("order-b.txt");
  FileStream *order_b = compress_outside_file.get();
  std::unique_ptr<Stream> compress_outside =
      std::make_unique<CompressedStream>(
          std::make_unique<EncryptedStream>(std::move(compress_outside_file)));
  std::cout << "[外层压缩：先压缩，再加密]\n";
  round_trip(*compress_outside, "hi");
  std::cout << "  stored: " << order_b->read() << "\n";

  std::cout << "\n=== 同一种装饰器可以叠两次 ===\n";
  std::unique_ptr<Stream> twice = std::make_unique<EncryptedStream>(
      std::make_unique<EncryptedStream>(
          std::make_unique<MemoryStream>()));
  round_trip(*twice, "hi");

  std::cout << "\n=== 错误处理：空路径、空流、空数据、对不上的载荷 ===\n";
  try {
    FileStream missing("");
  } catch (const std::invalid_argument &error) {
    std::cout << "  path: " << error.what() << "\n";
  }
  try {
    EncryptedStream missing(nullptr);
  } catch (const std::invalid_argument &error) {
    std::cout << "  stream: " << error.what() << "\n";
  }
  try {
    (void)plain.write("");
  } catch (const std::invalid_argument &error) {
    std::cout << "  data: " << error.what() << "\n";
  }
  try {
    auto bare = std::make_unique<FileStream>("plain.txt");
    bare->write("hello");
    EncryptedStream corrupt(std::move(bare));
    (void)corrupt.read();
  } catch (const std::invalid_argument &error) {
    std::cout << "  payload: " << error.what() << "\n";
  }

  return 0;
}
