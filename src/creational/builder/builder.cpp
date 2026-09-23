#include "creational/builder/builder.h"

#include <stdexcept>
#include <utility>

namespace design_pattern::creational::builder {

namespace {

std::string require_method(std::string_view method) {
  if (method.empty()) {
    throw std::invalid_argument("method is required");
  }
  static constexpr const char *kAllowed[] = {"GET", "POST", "PUT", "PATCH",
                                             "DELETE", "HEAD"};
  for (const char *allowed : kAllowed) {
    if (method == allowed) {
      return std::string(method);
    }
  }
  throw std::invalid_argument("unknown HTTP method: " + std::string(method));
}

void require_url(std::string_view url) {
  if (url.empty()) {
    throw std::invalid_argument("url is required");
  }
}

void require_header_name(std::string_view name) {
  if (name.empty()) {
    throw std::invalid_argument("header name is required");
  }
}

void validate_request(std::string_view method, std::string_view url,
                      std::string_view body) {
  require_url(url);
  if ((method == "GET" || method == "HEAD") && !body.empty()) {
    throw std::invalid_argument(std::string(method) +
                                " request must not have a body");
  }
}

}  // namespace

HttpRequest::HttpRequest(std::string method, std::string url,
                         std::vector<std::pair<std::string, std::string>> headers,
                         std::string body)
    : method_(method.empty() ? "GET" : std::move(method)), url_(std::move(url)),
      headers_(std::move(headers)), body_(std::move(body)) {
  method_ = require_method(method_);
  for (const auto &[name, value] : headers_) {
    require_header_name(name);
  }
  validate_request(method_, url_, body_);
}

const std::string &HttpRequest::method() const { return method_; }

const std::string &HttpRequest::url() const { return url_; }

const std::vector<std::pair<std::string, std::string>> &
HttpRequest::headers() const {
  return headers_;
}

const std::string &HttpRequest::body() const { return body_; }

std::string HttpRequest::describe() const {
  std::string out = method_ + " " + url_;
  for (const auto &[name, value] : headers_) {
    out += " | ";
    out += name;
    out += "=";
    out += value;
  }
  if (!body_.empty()) {
    out += " | body=";
    out += body_;
  }
  return out;
}

void HttpRequestBuilder::setMethod(std::string_view method) {
  method_ = require_method(method);
}

void HttpRequestBuilder::setUrl(std::string_view url) {
  require_url(url);
  url_ = std::string(url);
}

void HttpRequestBuilder::setHeader(std::string_view name,
                                   std::string_view value) {
  require_header_name(name);
  headers_.emplace_back(std::string(name), std::string(value));
}

void HttpRequestBuilder::setBody(std::string_view body) {
  body_ = std::string(body);
}

HttpRequest HttpRequestBuilder::build() const {
  return HttpRequest(method_, url_, headers_, body_);
}

void CurlCommandBuilder::setMethod(std::string_view method) {
  method_ = require_method(method);
}

void CurlCommandBuilder::setUrl(std::string_view url) {
  require_url(url);
  url_ = std::string(url);
}

void CurlCommandBuilder::setHeader(std::string_view name,
                                   std::string_view value) {
  require_header_name(name);
  headers_.emplace_back(std::string(name), std::string(value));
}

void CurlCommandBuilder::setBody(std::string_view body) {
  body_ = std::string(body);
}

std::string CurlCommandBuilder::build() const {
  validate_request(method_, url_, body_);
  std::string cmd = "curl -X ";
  cmd += method_;
  cmd += " '";
  cmd += url_;
  cmd += "'";
  for (const auto &[name, value] : headers_) {
    cmd += " -H '";
    cmd += name;
    cmd += ": ";
    cmd += value;
    cmd += "'";
  }
  if (!body_.empty()) {
    cmd += " -d '";
    cmd += body_;
    cmd += "'";
  }
  return cmd;
}

void Director::buildHealthCheck(Builder &builder) const {
  builder.setMethod("GET");
  builder.setUrl("/health");
}

void Director::buildLogin(Builder &builder) const {
  builder.setMethod("POST");
  builder.setUrl("/login");
  builder.setHeader("Content-Type", "application/json");
  builder.setBody(R"({"user":"alice","password":"secret"})");
}

HttpRequestChainBuilder &
HttpRequestChainBuilder::method(std::string_view method) {
  method_ = require_method(method);
  return *this;
}

HttpRequestChainBuilder &HttpRequestChainBuilder::url(std::string_view url) {
  require_url(url);
  url_ = std::string(url);
  return *this;
}

HttpRequestChainBuilder &
HttpRequestChainBuilder::header(std::string_view name, std::string_view value) {
  require_header_name(name);
  headers_.emplace_back(std::string(name), std::string(value));
  return *this;
}

HttpRequestChainBuilder &HttpRequestChainBuilder::body(std::string_view body) {
  body_ = std::string(body);
  return *this;
}

HttpRequest HttpRequestChainBuilder::build() const {
  return HttpRequest(method_, url_, headers_, body_);
}

}  // namespace design_pattern::creational::builder
