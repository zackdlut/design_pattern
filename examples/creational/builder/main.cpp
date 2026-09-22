#include "creational/builder/builder.h"

#include <iostream>

using design_pattern::creational::builder::Builder;
using design_pattern::creational::builder::CurlCommandBuilder;
using design_pattern::creational::builder::Director;
using design_pattern::creational::builder::HttpObjectBuilder;
using design_pattern::creational::builder::HttpRequest;
using design_pattern::creational::builder::HttpRequestBuilder;

namespace {

// 客户端只依赖 Builder：换建造者即换表示，配方仍由 Director 固定。
void assemble_login(Builder &builder, Director &director) {
  director.buildLogin(builder);
}

}  // namespace

int main() {
  std::cout << "=== 建造者：同一套装配步骤，产出 HTTP 对象或 curl 命令 ===\n";

  Director director;
  HttpObjectBuilder login_http;
  CurlCommandBuilder login_curl;
  director.buildLogin(login_http);
  director.buildLogin(login_curl);
  std::cout << "[login]\n";
  std::cout << "  object: " << login_http.build().describe() << "\n";
  std::cout << "  curl:   " << login_curl.build() << "\n";

  HttpObjectBuilder health_http;
  CurlCommandBuilder health_curl;
  director.buildHealthCheck(health_http);
  director.buildHealthCheck(health_curl);
  std::cout << "[health]\n";
  std::cout << "  object: " << health_http.build().describe() << "\n";
  std::cout << "  curl:   " << health_curl.build() << "\n";

  std::cout << "\n=== 同一抽象 Builder&，换具体建造者即换表示 ===\n";
  HttpObjectBuilder as_object;
  CurlCommandBuilder as_curl;
  Builder &object_ref = as_object;
  Builder &curl_ref = as_curl;
  assemble_login(object_ref, director);
  assemble_login(curl_ref, director);
  std::cout << "  via Builder& (http object): " << as_object.build().describe()
            << "\n";
  std::cout << "  via Builder& (curl):        " << as_curl.build() << "\n";

  std::cout << "\n=== 对照：链式建造者，客户端自己当导演 ===\n";
  HttpRequest req = HttpRequestBuilder()
                        .method("POST")
                        .url("/login")
                        .header("Authorization", "Bearer token")
                        .body(R"({"user":"alice"})")
                        .build();
  std::cout << "  " << req.describe() << "\n";

  HttpRequest health = HttpRequestBuilder().url("/health").build();
  std::cout << "  " << health.describe() << "\n";

  return 0;
}
