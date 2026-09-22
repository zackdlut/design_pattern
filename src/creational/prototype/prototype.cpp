#include "creational/prototype/prototype.h"

#include <stdexcept>
#include <utility>

namespace design_pattern::creational::prototype {

namespace {

std::string require_name(std::string_view name) {
  if (name.empty()) {
    throw std::invalid_argument("name is required");
  }
  return std::string(name);
}

std::string require_kind(std::string_view kind) {
  if (kind.empty()) {
    throw std::invalid_argument("kind is required");
  }
  return std::string(kind);
}

int require_positive(int value, std::string_view field) {
  if (value <= 0) {
    throw std::invalid_argument(std::string(field) + " must be positive");
  }
  return value;
}

void require_token(std::string_view token, std::string_view field) {
  if (token.empty()) {
    throw std::invalid_argument(std::string(field) + " is required");
  }
}

void require_tokens(const std::vector<std::string> &tokens,
                    std::string_view field) {
  for (const auto &token : tokens) {
    require_token(token, field);
  }
}

std::string join(const std::vector<std::string> &parts) {
  std::string out;
  for (std::size_t i = 0; i < parts.size(); ++i) {
    if (i != 0) {
      out += ",";
    }
    out += parts[i];
  }
  return out;
}

std::string format_unit(std::string_view kind, std::string_view name, int hp,
                        std::string_view extra_label,
                        const std::vector<std::string> &extra_items, int extra,
                        std::string_view extra_field, int x, int y) {
  std::string out;
  out += kind;
  out += " ";
  out += name;
  out += " hp=";
  out += std::to_string(hp);
  out += " ";
  out += extra_field;
  out += "=";
  out += std::to_string(extra);
  if (!extra_items.empty()) {
    out += " ";
    out += extra_label;
    out += "=";
    out += join(extra_items);
  }
  out += " @ (";
  out += std::to_string(x);
  out += ",";
  out += std::to_string(y);
  out += ")";
  return out;
}

}  // namespace

Warrior::Warrior(std::string name, int hp, int attack,
                 std::vector<std::string> skills, int x, int y)
    : name_(require_name(name)), hp_(require_positive(hp, "hp")),
      attack_(require_positive(attack, "attack")), skills_(std::move(skills)),
      x_(x), y_(y) {
  require_tokens(skills_, "skill");
}

std::unique_ptr<Prototype> Warrior::clone() const {
  return std::make_unique<Warrior>(name_, hp_, attack_, skills_, x_, y_);
}

std::string Warrior::kind() const { return "Warrior"; }

std::string Warrior::name() const { return name_; }

void Warrior::setName(std::string_view name) { name_ = require_name(name); }

void Warrior::setPosition(int x, int y) {
  x_ = x;
  y_ = y;
}

int Warrior::x() const { return x_; }

int Warrior::y() const { return y_; }

std::string Warrior::describe() const {
  return format_unit("Warrior", name_, hp_, "skills", skills_, attack_, "atk",
                     x_, y_);
}

int Warrior::hp() const { return hp_; }

int Warrior::attack() const { return attack_; }

const std::vector<std::string> &Warrior::skills() const { return skills_; }

void Warrior::addSkill(std::string_view skill) {
  require_token(skill, "skill");
  skills_.emplace_back(skill);
}

Mage::Mage(std::string name, int hp, int mana, std::vector<std::string> spells,
           int x, int y)
    : name_(require_name(name)), hp_(require_positive(hp, "hp")),
      mana_(require_positive(mana, "mana")), spells_(std::move(spells)), x_(x),
      y_(y) {
  require_tokens(spells_, "spell");
}

std::unique_ptr<Prototype> Mage::clone() const {
  return std::make_unique<Mage>(name_, hp_, mana_, spells_, x_, y_);
}

std::string Mage::kind() const { return "Mage"; }

std::string Mage::name() const { return name_; }

void Mage::setName(std::string_view name) { name_ = require_name(name); }

void Mage::setPosition(int x, int y) {
  x_ = x;
  y_ = y;
}

int Mage::x() const { return x_; }

int Mage::y() const { return y_; }

std::string Mage::describe() const {
  return format_unit("Mage", name_, hp_, "spells", spells_, mana_, "mana", x_,
                     y_);
}

int Mage::hp() const { return hp_; }

int Mage::mana() const { return mana_; }

const std::vector<std::string> &Mage::spells() const { return spells_; }

void Mage::addSpell(std::string_view spell) {
  require_token(spell, "spell");
  spells_.emplace_back(spell);
}

void PrototypeRegistry::registerPrototype(std::string name,
                            std::unique_ptr<Prototype> prototype) {
  if (name.empty()) {
    throw std::invalid_argument("prototype name is required");
  }
  if (!prototype) {
    throw std::invalid_argument("prototype is required");
  }
  if (prototypes_.contains(name)) {
    throw std::invalid_argument("duplicate prototype: " + name);
  }
  prototypes_.emplace(std::move(name), std::move(prototype));
}

std::unique_ptr<Prototype>
PrototypeRegistry::create(std::string_view name) const {
  auto it = prototypes_.find(std::string(name));
  if (it == prototypes_.end()) {
    throw std::invalid_argument("unknown prototype: " + std::string(name));
  }
  return it->second->clone();
}

bool PrototypeRegistry::contains(std::string_view name) const {
  return prototypes_.contains(std::string(name));
}

UnitSpec::UnitSpec(std::string kind, std::string name, int hp)
    : kind_(require_kind(kind)), name_(require_name(name)),
      hp_(require_positive(hp, "hp")) {}

void UnitSpec::setName(std::string_view name) { name_ = require_name(name); }

void UnitSpec::setPosition(int x, int y) {
  x_ = x;
  y_ = y;
}

const std::string &UnitSpec::kind() const { return kind_; }

const std::string &UnitSpec::name() const { return name_; }

int UnitSpec::hp() const { return hp_; }

int UnitSpec::x() const { return x_; }

int UnitSpec::y() const { return y_; }

std::string UnitSpec::describe() const {
  std::string out = "UnitSpec ";
  out += kind_;
  out += " ";
  out += name_;
  out += " hp=";
  out += std::to_string(hp_);
  out += " @ (";
  out += std::to_string(x_);
  out += ",";
  out += std::to_string(y_);
  out += ")";
  return out;
}

}  // namespace design_pattern::creational::prototype
