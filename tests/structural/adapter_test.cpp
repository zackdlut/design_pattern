#include "structural/adapter/adapter.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

using design_pattern::structural::adapter::ClassAdapter;
using design_pattern::structural::adapter::LegacyPlayer;
using design_pattern::structural::adapter::MediaPlayer;
using design_pattern::structural::adapter::Mp3Player;
using design_pattern::structural::adapter::ObjectAdapter;
using design_pattern::structural::adapter::VintageWalkman;

namespace {

void expect_invalid(const char *message, auto &&action) {
  try {
    action();
    FAIL() << "expected std::invalid_argument: " << message;
  } catch (const std::invalid_argument &error) {
    EXPECT_STREQ(error.what(), message);
  }
}

std::string play_on(const MediaPlayer &player, std::string_view filename) {
  return player.play(filename);
}

}  // namespace

TEST(AdapterTest, Mp3PlayerPlaysCompatibleFiles) {
  Mp3Player player;
  EXPECT_EQ(player.play("song.mp3"), "Mp3Player plays song.mp3");
  EXPECT_EQ(player.play("my.song.mp3"), "Mp3Player plays my.song.mp3");
}

TEST(AdapterTest, LegacyPlayerUsesIncompatibleApi) {
  LegacyPlayer walkman("walkman");
  EXPECT_EQ(walkman.name(), "walkman");
  EXPECT_EQ(walkman.playWav("moonlight"),
            "LegacyPlayer walkman plays wav: moonlight");
}

TEST(AdapterTest, ObjectAdapterTranslatesPlayToPlayWav) {
  ObjectAdapter adapter(std::make_unique<LegacyPlayer>("walkman"));
  EXPECT_EQ(adapter.play("song.mp3"),
            "ObjectAdapter -> LegacyPlayer walkman plays wav: song");
  EXPECT_EQ(adapter.play("track.wav"),
            "ObjectAdapter -> LegacyPlayer walkman plays wav: track");
}

TEST(AdapterTest, ClassAdapterTranslatesPlayToPlayWav) {
  ClassAdapter adapter("discman");
  EXPECT_EQ(adapter.play("song.mp3"),
            "ClassAdapter -> LegacyPlayer discman plays wav: song");
  EXPECT_EQ(adapter.play("track.wav"),
            "ClassAdapter -> LegacyPlayer discman plays wav: track");
}

TEST(AdapterTest, ClientDependsOnTargetAbstraction) {
  Mp3Player mp3;
  ObjectAdapter object_adapter(std::make_unique<LegacyPlayer>("walkman"));
  ClassAdapter class_adapter("discman");

  const MediaPlayer &as_mp3 = mp3;
  const MediaPlayer &as_object = object_adapter;
  const MediaPlayer &as_class = class_adapter;

  EXPECT_EQ(play_on(as_mp3, "song.mp3"), "Mp3Player plays song.mp3");
  EXPECT_EQ(play_on(as_object, "song.mp3"),
            "ObjectAdapter -> LegacyPlayer walkman plays wav: song");
  EXPECT_EQ(play_on(as_class, "song.mp3"),
            "ClassAdapter -> LegacyPlayer discman plays wav: song");
}

TEST(AdapterTest, ObjectAdapterDispatchesToAdapteeSubclass) {
  ObjectAdapter adapter(std::make_unique<VintageWalkman>());
  EXPECT_EQ(play_on(adapter, "song.mp3"),
            "ObjectAdapter -> LegacyPlayer walkman plays wav: song [tape]");
}

TEST(AdapterTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<MediaPlayer>);
  static_assert(!std::is_move_constructible_v<MediaPlayer>);
  static_assert(!std::is_copy_assignable_v<MediaPlayer>);
  static_assert(!std::is_move_assignable_v<MediaPlayer>);
  static_assert(!std::is_copy_constructible_v<LegacyPlayer>);
  static_assert(!std::is_move_constructible_v<LegacyPlayer>);
  static_assert(!std::is_copy_constructible_v<Mp3Player>);
  static_assert(!std::is_copy_constructible_v<ObjectAdapter>);
  static_assert(!std::is_copy_constructible_v<ClassAdapter>);
  static_assert(!std::is_copy_constructible_v<VintageWalkman>);
}

TEST(AdapterTest, InheritanceAndConvertibility) {
  static_assert(std::is_base_of_v<MediaPlayer, Mp3Player>);
  static_assert(std::is_base_of_v<MediaPlayer, ObjectAdapter>);
  static_assert(std::is_base_of_v<MediaPlayer, ClassAdapter>);
  static_assert(std::is_convertible_v<Mp3Player *, MediaPlayer *>);
  static_assert(std::is_convertible_v<ObjectAdapter *, MediaPlayer *>);
  static_assert(std::is_convertible_v<ClassAdapter *, MediaPlayer *>);

  static_assert(!std::is_base_of_v<LegacyPlayer, ObjectAdapter>);
  static_assert(!std::is_convertible_v<ObjectAdapter *, LegacyPlayer *>);
  static_assert(std::is_base_of_v<LegacyPlayer, ClassAdapter>);
  static_assert(!std::is_convertible_v<ClassAdapter *, LegacyPlayer *>);
  static_assert(!std::is_convertible_v<ClassAdapter *, const LegacyPlayer *>);
}

TEST(AdapterTest, Mp3PlayerRejectsInvalidFiles) {
  expect_invalid("filename is required", [] { Mp3Player{}.play(""); });
  expect_invalid("filename must have an extension",
                 [] { Mp3Player{}.play("song"); });
  expect_invalid("filename must have an extension",
                 [] { Mp3Player{}.play(".mp3"); });
  expect_invalid("filename must have an extension",
                 [] { Mp3Player{}.play("song."); });
  expect_invalid("unsupported format: wav",
                 [] { Mp3Player{}.play("song.wav"); });
}

TEST(AdapterTest, ObjectAdapterRejectsInvalidOperations) {
  expect_invalid("adaptee is required",
                 [] { ObjectAdapter adapter(nullptr); });
  expect_invalid("player name is required",
                 [] { LegacyPlayer player(""); });

  ObjectAdapter adapter(std::make_unique<LegacyPlayer>("walkman"));
  expect_invalid("filename is required", [&] { adapter.play(""); });
  expect_invalid("filename must have an extension",
                 [&] { adapter.play("song"); });
  expect_invalid("unsupported format: flac",
                 [&] { adapter.play("song.flac"); });
}

TEST(AdapterTest, ClassAdapterRejectsInvalidOperations) {
  expect_invalid("player name is required", [] { ClassAdapter adapter(""); });

  ClassAdapter adapter("discman");
  expect_invalid("filename is required", [&] { adapter.play(""); });
  expect_invalid("filename must have an extension",
                 [&] { adapter.play("song"); });
  expect_invalid("unsupported format: flac",
                 [&] { adapter.play("song.flac"); });
}

TEST(AdapterTest, LegacyPlayerRejectsEmptyTitle) {
  LegacyPlayer walkman("walkman");
  expect_invalid("title is required", [&] { walkman.playWav(""); });
}
