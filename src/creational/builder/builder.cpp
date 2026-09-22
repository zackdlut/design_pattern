#include "creational/builder/builder.h"

#include <stdexcept>
#include <utility>

namespace design_pattern::creational::builder {

namespace {

std::string default_method(std::string method) {
  return method.empty() ? "GET" : std::move(method);
}

}  // namespace

HttpRequest::HttpRequest(std::string method, std::string url,
                         std::vector<std::pair<std::string, std::string>> headers,
                         std::string body)
    : method_(default_method(std::move(method))), url_(std::move(url)),
      headers_(std::move(headers)), body_(std::move(body)) {
  if (url_.empty()) {
    throw std::invalid_argument("url is required");
  }
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

void HttpObjectBuilder::setMethod(std::string_view method) {
  method_ = default_method(std::string(method));
}

void HttpObjectBuilder::setUrl(std::string_view url) { url_ = std::string(url); }

void HttpObjectBuilder::addHeader(std::string_view name,
                                  std::string_view value) {
  headers_.emplace_back(std::string(name), std::string(value));
}

void HttpObjectBuilder::setBody(std::string_view body) {
  body_ = std::string(body);
}

HttpRequest HttpObjectBuilder::result() const {
  return HttpRequest(method_, url_, headers_, body_);
}

void CurlCommandBuilder::setMethod(std::string_view method) {
  method_ = default_method(std::string(method));
}

void CurlCommandBuilder::setUrl(std::string_view url) { url_ = std::string(url); }

void CurlCommandBuilder::addHeader(std::string_view name,
                                   std::string_view value) {
  headers_.emplace_back(std::string(name), std::string(value));
}

void CurlCommandBuilder::setBody(std::string_view body) {
  body_ = std::string(body);
}

std::string CurlCommandBuilder::result() const {
  if (url_.empty()) {
    throw std::invalid_argument("url is required");
  }
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

void Director::constructHealthCheck(Builder &builder) const {
  builder.setMethod("GET");
  builder.setUrl("/health");
}

void Director::constructLogin(Builder &builder) const {
  builder.setMethod("POST");
  builder.setUrl("/login");
  builder.addHeader("Content-Type", "application/json");
  builder.setBody(R"({"user":"alice","password":"secret"})");
}

HttpRequestBuilder &HttpRequestBuilder::method(std::string_view method) {
  method_ = default_method(std::string(method));
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::url(std::string_view url) {
  url_ = std::string(url);
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::header(std::string_view name,
                                               std::string_view value) {
  headers_.emplace_back(std::string(name), std::string(value));
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::body(std::string_view body) {
  body_ = std::string(body);
  return *this;
}

HttpRequest HttpRequestBuilder::build() const {
  return HttpRequest(method_, url_, headers_, body_);
}

}  // namespace design_pattern::creational::builder
