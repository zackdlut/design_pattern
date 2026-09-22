#include "structural/adapter/adapter.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using design_pattern::structural::adapter::ClassAdapter;
using design_pattern::structural::adapter::LegacyPlayer;
using design_pattern::structural::adapter::MediaPlayer;
using design_pattern::structural::adapter::Mp3Player;
using design_pattern::structural::adapter::ObjectAdapter;
using design_pattern::structural::adapter::VintageWalkman;

namespace {

// 客户端只依赖 Target：手里是 MediaPlayer&，不感知 Adaptee。
void play_on(const MediaPlayer &player, std::string_view filename) {
  std::cout << "  " << player.play(filename) << "\n";
}

}  // namespace

int main() {
  std::cout << "=== 适配器：旧播放器接口对不上，包一层再交给客户端 ===\n";

  Mp3Player mp3;
  ObjectAdapter walkman(std::make_unique<LegacyPlayer>("walkman"));
  ClassAdapter discman("discman");

  const std::vector<std::pair<std::string, const MediaPlayer *>> playlist{
      {"song.mp3", &mp3},
      {"moonlight.wav", &walkman},
      {"nocturne.mp3", &discman},
  };

  for (const auto &[file, player] : playlist) {
    std::cout << "[" << file << "]\n";
    play_on(*player, file);
  }

  std::cout << "\n=== 同一抽象 MediaPlayer&，换适配器即换旧设备 ===\n";
  const MediaPlayer &as_mp3 = mp3;
  const MediaPlayer &as_object = walkman;
  const MediaPlayer &as_class = discman;
  play_on(as_mp3, "song.mp3");
  play_on(as_object, "song.mp3");
  play_on(as_class, "song.mp3");

  std::cout << "\n=== 对象适配器可包 Adaptee 子类；类适配器绑死 LegacyPlayer ===\n";
  ObjectAdapter vintage(std::make_unique<VintageWalkman>());
  play_on(vintage, "tape.wav");

  std::cout << "\n=== 错误处理：空适配对象、坏文件名、不支持的格式 ===\n";
  try {
    ObjectAdapter missing(nullptr);
  } catch (const std::invalid_argument &error) {
    std::cout << "  object: " << error.what() << "\n";
  }
  try {
    (void)walkman.play("song.flac");
  } catch (const std::invalid_argument &error) {
    std::cout << "  format: " << error.what() << "\n";
  }
  try {
    (void)mp3.play("song.wav");
  } catch (const std::invalid_argument &error) {
    std::cout << "  native: " << error.what() << "\n";
  }

  return 0;
}
