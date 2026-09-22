#include "creational/builder/builder.h"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <type_traits>

using design_pattern::creational::builder::Builder;
using design_pattern::creational::builder::CurlCommandBuilder;
using design_pattern::creational::builder::Director;
using design_pattern::creational::builder::HttpObjectBuilder;
using design_pattern::creational::builder::HttpRequest;
using design_pattern::creational::builder::HttpRequestBuilder;

TEST(BuilderTest, HttpObjectBuilderBuildsRequest) {
  HttpObjectBuilder builder;
  builder.setMethod("POST");
  builder.setUrl("/login");
  builder.addHeader("Authorization", "Bearer token");
  builder.setBody(R"({"user":"alice"})");

  HttpRequest request = builder.result();
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
  builder.addHeader("Authorization", "Bearer token");
  builder.setBody(R"({"user":"alice"})");

  EXPECT_EQ(builder.result(),
            "curl -X POST '/login' -H 'Authorization: Bearer token' "
            "-d '{\"user\":\"alice\"}'");
}

TEST(BuilderTest, DirectorConstructsLoginOnBothBuilders) {
  Director director;
  HttpObjectBuilder http;
  CurlCommandBuilder curl;
  director.constructLogin(http);
  director.constructLogin(curl);

  EXPECT_EQ(http.result().describe(),
            "POST /login | Content-Type=application/json | "
            "body={\"user\":\"alice\",\"password\":\"secret\"}");
  EXPECT_EQ(curl.result(),
            "curl -X POST '/login' -H 'Content-Type: application/json' "
            "-d '{\"user\":\"alice\",\"password\":\"secret\"}'");
}

TEST(BuilderTest, DirectorConstructsHealthCheck) {
  Director director;
  HttpObjectBuilder http;
  CurlCommandBuilder curl;
  director.constructHealthCheck(http);
  director.constructHealthCheck(curl);

  EXPECT_EQ(http.result().describe(), "GET /health");
  EXPECT_EQ(curl.result(), "curl -X GET '/health'");
}

TEST(BuilderTest, ClientDependsOnBuilderAbstraction) {
  Director director;
  HttpObjectBuilder http;
  CurlCommandBuilder curl;
  Builder &as_http = http;
  Builder &as_curl = curl;
  director.constructLogin(as_http);
  director.constructLogin(as_curl);

  EXPECT_EQ(http.result().method(), "POST");
  EXPECT_EQ(http.result().url(), "/login");
  EXPECT_EQ(curl.result().substr(0, 14), "curl -X POST '");
}

TEST(BuilderTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<Builder>);
  static_assert(!std::is_move_constructible_v<Builder>);
  static_assert(!std::is_copy_assignable_v<Builder>);
  static_assert(!std::is_move_assignable_v<Builder>);
  static_assert(!std::is_copy_constructible_v<HttpObjectBuilder>);
  static_assert(!std::is_copy_constructible_v<CurlCommandBuilder>);
  static_assert(std::is_copy_constructible_v<HttpRequest>);
  static_assert(std::is_copy_constructible_v<HttpRequestBuilder>);
}

TEST(BuilderTest, FluentBuilderBuildsRequest) {
  HttpRequest request = HttpRequestBuilder()
                            .method("POST")
                            .url("/login")
                            .header("Authorization", "Bearer token")
                            .body(R"({"user":"alice"})")
                            .build();

  EXPECT_EQ(request.describe(),
            "POST /login | Authorization=Bearer token | body={\"user\":\"alice\"}");
}

TEST(BuilderTest, FluentBuilderDefaultsToGet) {
  HttpRequest request = HttpRequestBuilder().url("/health").build();
  EXPECT_EQ(request.method(), "GET");
  EXPECT_EQ(request.url(), "/health");
  EXPECT_TRUE(request.headers().empty());
  EXPECT_TRUE(request.body().empty());
  EXPECT_EQ(request.describe(), "GET /health");
}

TEST(BuilderTest, BuildersRejectMissingUrl) {
  EXPECT_THROW(HttpObjectBuilder{}.result(), std::invalid_argument);
  EXPECT_THROW(CurlCommandBuilder{}.result(), std::invalid_argument);
  EXPECT_THROW(HttpRequestBuilder{}.build(), std::invalid_argument);
}
