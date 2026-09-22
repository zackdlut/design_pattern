#pragma once

#include <memory>
#include <string>

namespace design_pattern::creational::abstract_factory {

class Button {
public:
  Button() = default;
  virtual ~Button() = default;
  Button(const Button &) = delete;
  Button &operator=(const Button &) = delete;
  Button(Button &&) = delete;
  Button &operator=(Button &&) = delete;

  virtual std::string paint() const = 0;
};

class WindowsButton final : public Button {
public:
  std::string paint() const override;
};

class MacButton final : public Button {
public:
  std::string paint() const override;
};

class CheckBox {
public:
  CheckBox() = default;
  virtual ~CheckBox() = default;
  CheckBox(const CheckBox &) = delete;
  CheckBox &operator=(const CheckBox &) = delete;
  CheckBox(CheckBox &&) = delete;
  CheckBox &operator=(CheckBox &&) = delete;

  virtual std::string paint() const = 0;
};

class WindowsCheckBox final : public CheckBox {
public:
  std::string paint() const override;
};

class MacCheckBox final : public CheckBox {
public:
  std::string paint() const override;
};

class AbstractFactory {
public:
  AbstractFactory() = default;
  virtual ~AbstractFactory() = default;
  AbstractFactory(const AbstractFactory &) = delete;
  AbstractFactory &operator=(const AbstractFactory &) = delete;
  AbstractFactory(AbstractFactory &&) = delete;
  AbstractFactory &operator=(AbstractFactory &&) = delete;

  virtual std::unique_ptr<Button> createButton() const = 0;
  virtual std::unique_ptr<CheckBox> createCheckBox() const = 0;
};

class WindowsGUIFactory final : public AbstractFactory {
public:
  std::unique_ptr<Button> createButton() const override;
  std::unique_ptr<CheckBox> createCheckBox() const override;
};

class MacGUIFactory final : public AbstractFactory {
public:
  std::unique_ptr<Button> createButton() const override;
  std::unique_ptr<CheckBox> createCheckBox() const override;
};

enum class GuiTheme { Windows, Mac };

// 对照：按配置选具体工厂。客户端拿到的仍是 AbstractFactory，不加产品不必改渲染流程。
class GuiFactorySelector final {
public:
  GuiFactorySelector() = delete;
  static std::unique_ptr<AbstractFactory> create(GuiTheme theme);
};

}  // namespace design_pattern::creational::abstract_factory
