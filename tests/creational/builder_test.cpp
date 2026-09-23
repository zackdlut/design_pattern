#include "creational/builder/builder.h"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <type_traits>

using design_pattern::creational::builder::Builder;
using design_pattern::creational::builder::CurlCommandBuilder;
using design_pattern::creational::builder::Director;
using design_pattern::creational::builder::HttpRequestBuilder;
using design_pattern::creational::builder::HttpRequest;
using design_pattern::creational::builder::HttpRequestChainBuilder;

namespace {

void expect_invalid(const char *message, auto &&action) {
  try {
    action();
    FAIL() << "expected std::invalid_argument: " << message;
  } catch (const std::invalid_argument &error) {
    EXPECT_STREQ(error.what(), message);
  }
}

}  // namespace

TEST(BuilderTest, HttpRequestBuilderBuildsRequest) {
  HttpRequestBuilder builder;
  builder.setMethod("POST");
  builder.setUrl("/login");
  builder.setHeader("Authorization", "Bearer token");
  builder.setBody(R"({"user":"alice"})");

  HttpRequest request = builder.build();
  EXPECT_EQ(request.method(), "POST");
  EXPECT_EQ(request.url(), "/login");
  ASSERT_EQ(request.headers().size(), 1U);
  EXPECT_EQ(request.headers()[0].first, "Authorization");
  EXPECT_EQ(request.headers()[0].second, "Bearer token");
  EXPECT_EQ(request.body(), R"({"user":"alice"})");
  EXPECT_EQ(request.describe(),
            "POST /login | Authorization=Bearer token | body={\"user\":\"alice\"}");
}

TEST(BuilderTest, CurlCommandBuilderBuildsCurl) {
  CurlCommandBuilder builder;
  builder.setMethod("POST");
  builder.setUrl("/login");
  builder.setHeader("Authorization", "Bearer token");
  builder.setBody(R"({"user":"alice"})");

  EXPECT_EQ(builder.build(),
            "curl -X POST '/login' -H 'Authorization: Bearer token' "
            "-d '{\"user\":\"alice\"}'");
}

TEST(BuilderTest, DirectorConstructsLoginOnBothBuilders) {
  Director director;
  HttpRequestBuilder http;
  CurlCommandBuilder curl;
  director.buildLogin(http);
  director.buildLogin(curl);

  EXPECT_EQ(http.build().describe(),
            "POST /login | Content-Type=application/json | "
            "body={\"user\":\"alice\",\"password\":\"secret\"}");
  EXPECT_EQ(curl.build(),
            "curl -X POST '/login' -H 'Content-Type: application/json' "
            "-d '{\"user\":\"alice\",\"password\":\"secret\"}'");
}

TEST(BuilderTest, DirectorConstructsHealthCheck) {
  Director director;
  HttpRequestBuilder http;
  CurlCommandBuilder curl;
  director.buildHealthCheck(http);
  director.buildHealthCheck(curl);

  EXPECT_EQ(http.build().describe(), "GET /health");
  EXPECT_EQ(curl.build(), "curl -X GET '/health'");
}

TEST(BuilderTest, ClientDependsOnBuilderAbstraction) {
  Director director;
  HttpRequestBuilder http;
  CurlCommandBuilder curl;
  Builder &as_http = http;
  Builder &as_curl = curl;
  director.buildLogin(as_http);
  director.buildLogin(as_curl);

  EXPECT_EQ(http.build().method(), "POST");
  EXPECT_EQ(http.build().url(), "/login");
  EXPECT_EQ(curl.build().substr(0, 14), "curl -X POST '");
}

TEST(BuilderTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<Builder>);
  static_assert(!std::is_move_constructible_v<Builder>);
  static_assert(!std::is_copy_assignable_v<Builder>);
  static_assert(!std::is_move_assignable_v<Builder>);
  static_assert(!std::is_copy_constructible_v<HttpRequestBuilder>);
  static_assert(!std::is_copy_constructible_v<CurlCommandBuilder>);
  static_assert(std::is_copy_constructible_v<HttpRequest>);
  static_assert(std::is_copy_constructible_v<HttpRequestChainBuilder>);
}

TEST(BuilderTest, FluentBuilderBuildsRequest) {
  HttpRequest request = HttpRequestChainBuilder()
                            .method("POST")
                            .url("/login")
                            .header("Authorization", "Bearer token")
                            .body(R"({"user":"alice"})")
                            .build();

  EXPECT_EQ(request.describe(),
            "POST /login | Authorization=Bearer token | body={\"user\":\"alice\"}");
}

TEST(BuilderTest, FluentBuilderDefaultsToGet) {
  HttpRequest request = HttpRequestChainBuilder().url("/health").build();
  EXPECT_EQ(request.method(), "GET");
  EXPECT_EQ(request.url(), "/health");
  EXPECT_TRUE(request.headers().empty());
  EXPECT_TRUE(request.body().empty());
  EXPECT_EQ(request.describe(), "GET /health");
}

TEST(BuilderTest, BuildersRejectMissingUrl) {
  expect_invalid("url is required", [] { HttpRequestBuilder{}.build(); });
  expect_invalid("url is required", [] { CurlCommandBuilder{}.build(); });
  expect_invalid("url is required", [] { HttpRequestChainBuilder{}.build(); });
}

TEST(BuilderTest, SettersRejectInvalidFields) {
  expect_invalid("method is required", [] { HttpRequestBuilder{}.setMethod(""); });
  expect_invalid("unknown HTTP method: FOO",
                 [] { HttpRequestBuilder{}.setMethod("FOO"); });
  expect_invalid("url is required", [] { HttpRequestBuilder{}.setUrl(""); });
  expect_invalid("header name is required",
                 [] { HttpRequestBuilder{}.setHeader("", "v"); });

  expect_invalid("method is required", [] { CurlCommandBuilder{}.setMethod(""); });
  expect_invalid("unknown HTTP method: FOO",
                 [] { CurlCommandBuilder{}.setMethod("FOO"); });
  expect_invalid("url is required", [] { CurlCommandBuilder{}.setUrl(""); });
  expect_invalid("header name is required",
                 [] { CurlCommandBuilder{}.setHeader("", "v"); });

  expect_invalid("method is required",
                 [] { HttpRequestChainBuilder{}.method(""); });
  expect_invalid("unknown HTTP method: FOO",
                 [] { HttpRequestChainBuilder{}.method("FOO"); });
  expect_invalid("url is required", [] { HttpRequestChainBuilder{}.url(""); });
  expect_invalid("header name is required",
                 [] { HttpRequestChainBuilder{}.header("", "v"); });
}

TEST(BuilderTest, BuildRejectsGetWithBody) {
  expect_invalid("GET request must not have a body", [] {
    HttpRequestBuilder builder;
    builder.setMethod("GET");
    builder.setUrl("/health");
    builder.setBody("oops");
    builder.build();
  });
  expect_invalid("GET request must not have a body", [] {
    CurlCommandBuilder builder;
    builder.setMethod("GET");
    builder.setUrl("/health");
    builder.setBody("oops");
    builder.build();
  });
  expect_invalid("GET request must not have a body", [] {
    HttpRequestChainBuilder().method("GET").url("/health").body("oops").build();
  });
}
