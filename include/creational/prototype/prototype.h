#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace design_pattern::creational::prototype {

class Prototype {
public:
  Prototype() = default;
  virtual ~Prototype() = default;
  Prototype(const Prototype &) = delete;
  Prototype &operator=(const Prototype &) = delete;
  Prototype(Prototype &&) = delete;
  Prototype &operator=(Prototype &&) = delete;

  // 通过拷贝已有实例来创建。动态绑定决定具体类型，避免切片。
  virtual std::unique_ptr<Prototype> clone() const = 0;
  virtual std::string kind() const = 0;
  virtual std::string name() const = 0;
  virtual void setName(std::string_view name) = 0;
  virtual void setPosition(int x, int y) = 0;
  virtual int x() const = 0;
  virtual int y() const = 0;
  virtual std::string describe() const = 0;
};

class Warrior final : public Prototype {
public:
  Warrior(std::string name, int hp, int attack,
          std::vector<std::string> skills = {}, int x = 0, int y = 0);

  std::unique_ptr<Prototype> clone() const override;
  std::string kind() const override;
  std::string name() const override;
  void setName(std::string_view name) override;
  void setPosition(int x, int y) override;
  int x() const override;
  int y() const override;
  std::string describe() const override;

  int hp() const;
  int attack() const;
  const std::vector<std::string> &skills() const;
  void addSkill(std::string_view skill);

private:
  std::string name_;
  int hp_;
  int attack_;
  std::vector<std::string> skills_;
  int x_{0};
  int y_{0};
};

class Mage final : public Prototype {
public:
  Mage(std::string name, int hp, int mana, std::vector<std::string> spells = {},
       int x = 0, int y = 0);

  std::unique_ptr<Prototype> clone() const override;
  std::string kind() const override;
  std::string name() const override;
  void setName(std::string_view name) override;
  void setPosition(int x, int y) override;
  int x() const override;
  int y() const override;
  std::string describe() const override;

  int hp() const;
  int mana() const;
  const std::vector<std::string> &spells() const;
  void addSpell(std::string_view spell);

private:
  std::string name_;
  int hp_;
  int mana_;
  std::vector<std::string> spells_;
  int x_{0};
  int y_{0};
};

// 按名字取出模板并 clone。客户端只认字符串，不认 Warrior / Mage。
class PrototypeRegistry final {
public:
  PrototypeRegistry() = default;
  PrototypeRegistry(const PrototypeRegistry &) = delete;
  PrototypeRegistry &operator=(const PrototypeRegistry &) = delete;
  PrototypeRegistry(PrototypeRegistry &&) = delete;
  PrototypeRegistry &operator=(PrototypeRegistry &&) = delete;

  void registerPrototype(std::string name, std::unique_ptr<Prototype> prototype);
  std::unique_ptr<Prototype> create(std::string_view name) const;
  bool contains(std::string_view name) const;

private:
  std::map<std::string, std::unique_ptr<Prototype>> prototypes_;
};

// 对照：已知具体类型时直接拷贝值对象。没有虚 clone，加种类必须改这个类。
class UnitSpec final {
public:
  UnitSpec(std::string kind, std::string name, int hp);

  void setName(std::string_view name);
  void setPosition(int x, int y);

  const std::string &kind() const;
  const std::string &name() const;
  int hp() const;
  int x() const;
  int y() const;
  std::string describe() const;

private:
  std::string kind_;
  std::string name_;
  int hp_;
  int x_{0};
  int y_{0};
};

}  // namespace design_pattern::creational::prototype
