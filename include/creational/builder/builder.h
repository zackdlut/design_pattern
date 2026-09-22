#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace design_pattern::creational::builder {

class HttpRequest {
public:
  HttpRequest(std::string method, std::string url,
              std::vector<std::pair<std::string, std::string>> headers,
              std::string body);

  const std::string &method() const;
  const std::string &url() const;
  const std::vector<std::pair<std::string, std::string>> &headers() const;
  const std::string &body() const;
  std::string describe() const;

private:
  std::string method_;
  std::string url_;
  std::vector<std::pair<std::string, std::string>> headers_;
  std::string body_;
};

class Builder {
public:
  Builder() = default;
  virtual ~Builder() = default;
  Builder(const Builder &) = delete;
  Builder &operator=(const Builder &) = delete;
  Builder(Builder &&) = delete;
  Builder &operator=(Builder &&) = delete;

  virtual void setMethod(std::string_view method) = 0;
  virtual void setUrl(std::string_view url) = 0;
  virtual void addHeader(std::string_view name, std::string_view value) = 0;
  virtual void setBody(std::string_view body) = 0;
};

class HttpObjectBuilder final : public Builder {
public:
  void setMethod(std::string_view method) override;
  void setUrl(std::string_view url) override;
  void addHeader(std::string_view name, std::string_view value) override;
  void setBody(std::string_view body) override;

  HttpRequest result() const;

private:
  std::string method_{"GET"};
  std::string url_;
  std::vector<std::pair<std::string, std::string>> headers_;
  std::string body_;
};

class CurlCommandBuilder final : public Builder {
public:
  void setMethod(std::string_view method) override;
  void setUrl(std::string_view url) override;
  void addHeader(std::string_view name, std::string_view value) override;
  void setBody(std::string_view body) override;

  std::string result() const;

private:
  std::string method_{"GET"};
  std::string url_;
  std::vector<std::pair<std::string, std::string>> headers_;
  std::string body_;
};

class Director {
public:
  void constructHealthCheck(Builder &builder) const;
  void constructLogin(Builder &builder) const;
};

// 对照：链式建造者。客户端自己当导演，没有抽象 Builder / Director。
class HttpRequestBuilder final {
public:
  HttpRequestBuilder &method(std::string_view method);
  HttpRequestBuilder &url(std::string_view url);
  HttpRequestBuilder &header(std::string_view name, std::string_view value);
  HttpRequestBuilder &body(std::string_view body);

  HttpRequest build() const;

private:
  std::string method_{"GET"};
  std::string url_;
  std::vector<std::pair<std::string, std::string>> headers_;
  std::string body_;
};

}  // namespace design_pattern::creational::builder
