#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace design_pattern::structural::adapter {

// Target：客户端只认这个接口。
class MediaPlayer {
public:
  MediaPlayer() = default;
  virtual ~MediaPlayer() = default;
  MediaPlayer(const MediaPlayer &) = delete;
  MediaPlayer &operator=(const MediaPlayer &) = delete;
  MediaPlayer(MediaPlayer &&) = delete;
  MediaPlayer &operator=(MediaPlayer &&) = delete;

  virtual std::string play(std::string_view filename) const = 0;
};

// 已经兼容 Target 的播放器。对照：不需要适配。
class Mp3Player final : public MediaPlayer {
public:
  std::string play(std::string_view filename) const override;
};

// Adaptee：已有实现，接口对不上。不要改它来迁就客户端。
class LegacyPlayer {
public:
  explicit LegacyPlayer(std::string name);
  virtual ~LegacyPlayer() = default;
  LegacyPlayer(const LegacyPlayer &) = delete;
  LegacyPlayer &operator=(const LegacyPlayer &) = delete;
  LegacyPlayer(LegacyPlayer &&) = delete;
  LegacyPlayer &operator=(LegacyPlayer &&) = delete;

  virtual std::string playWav(std::string_view title) const;
  const std::string &name() const;

private:
  std::string name_;
};

// Adaptee 的子类。对象适配器能包它；类适配器绑死在 LegacyPlayer 上，包不了。
class VintageWalkman final : public LegacyPlayer {
public:
  VintageWalkman();
  std::string playWav(std::string_view title) const override;
};

// 对象适配器：组合 Adaptee。可包 LegacyPlayer 及其子类。
class ObjectAdapter final : public MediaPlayer {
public:
  explicit ObjectAdapter(std::unique_ptr<LegacyPlayer> adaptee);
  std::string play(std::string_view filename) const override;

private:
  std::unique_ptr<LegacyPlayer> adaptee_;
};

// 类适配器：公有继承 Target，私有继承 Adaptee。绑死在 LegacyPlayer。
class ClassAdapter final : public MediaPlayer, private LegacyPlayer {
public:
  explicit ClassAdapter(std::string player_name);
  std::string play(std::string_view filename) const override;
};

}  // namespace design_pattern::structural::adapter
