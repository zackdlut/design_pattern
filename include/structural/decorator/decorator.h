#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace design_pattern::structural::decorator {

// Component：客户端只认这个接口。具体流和装饰器都可以放进 Stream&。
class Stream {
public:
  Stream() = default;
  virtual ~Stream() = default;
  Stream(const Stream &) = delete;
  Stream &operator=(const Stream &) = delete;
  Stream(Stream &&) = delete;
  Stream &operator=(Stream &&) = delete;

  // write 返回调用链描述，方便测试断言装饰顺序。
  // read 返回载荷。缓冲装饰器的 read 会先把未刷出的数据写进内层，所以不能是 const。
  virtual std::string write(std::string_view data) = 0;
  virtual std::string read() = 0;
};

// ConcreteComponent：真正落地的流。装饰器不关心它是文件还是内存。
class FileStream final : public Stream {
public:
  explicit FileStream(std::string path);
  std::string write(std::string_view data) override;
  std::string read() override;
  const std::string &path() const;

private:
  std::string path_;
  std::string content_;
};

class MemoryStream final : public Stream {
public:
  std::string write(std::string_view data) override;
  std::string read() override;

private:
  std::string content_;
};

// Decorator：本身也是 Stream，并持有另一个 Stream。默认把调用原样转发。
// 构造是 protected：客户端叠的是具体装饰器，不是这个空转发层。
class StreamDecorator : public Stream {
public:
  std::string write(std::string_view data) override;
  std::string read() override;

protected:
  explicit StreamDecorator(std::unique_ptr<Stream> inner);
  Stream &inner();
  const Stream &inner() const;

private:
  std::unique_ptr<Stream> inner_;
};

// 写进缓冲区，flush 或 read 时才交给内层。flush 不在 Stream 上。
class BufferedStream final : public StreamDecorator {
public:
  explicit BufferedStream(std::unique_ptr<Stream> inner);
  std::string write(std::string_view data) override;
  std::string read() override;
  std::string flush();

private:
  std::string buffer_;
};

// 写出前套上 enc(...)，读回时拆掉。对只拿 Stream& 的客户端，读到的仍是原文。
class EncryptedStream final : public StreamDecorator {
public:
  explicit EncryptedStream(std::unique_ptr<Stream> inner);
  std::string write(std::string_view data) override;
  std::string read() override;
};

// 与加密同一形状的第二种职责，用来说明叠加顺序会改变落地字节。
class CompressedStream final : public StreamDecorator {
public:
  explicit CompressedStream(std::unique_ptr<Stream> inner);
  std::string write(std::string_view data) override;
  std::string read() override;
};

}  // namespace design_pattern::structural::decorator
