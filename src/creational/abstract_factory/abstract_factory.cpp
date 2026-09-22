#include "creational/abstract_factory/abstract_factory.h"

#include <stdexcept>

namespace design_pattern::creational::abstract_factory {

std::string WindowsButton::paint() const { return "WindowsButton paint"; }

std::string MacButton::paint() const { return "MacButton paint"; }

std::string WindowsCheckBox::paint() const { return "WindowsCheckBox paint"; }

std::string MacCheckBox::paint() const { return "MacCheckBox paint"; }

std::unique_ptr<Button> WindowsGUIFactory::createButton() const {
  return std::make_unique<WindowsButton>();
}

std::unique_ptr<CheckBox> WindowsGUIFactory::createCheckBox() const {
  return std::make_unique<WindowsCheckBox>();
}

std::unique_ptr<Button> MacGUIFactory::createButton() const {
  return std::make_unique<MacButton>();
}

std::unique_ptr<CheckBox> MacGUIFactory::createCheckBox() const {
  return std::make_unique<MacCheckBox>();
}

std::unique_ptr<AbstractFactory> GuiFactorySelector::create(GuiTheme theme) {
  switch (theme) {
  case GuiTheme::Windows:
    return std::make_unique<WindowsGUIFactory>();
  case GuiTheme::Mac:
    return std::make_unique<MacGUIFactory>();
  }
  throw std::invalid_argument("unknown GuiTheme");
}

}  // namespace design_pattern::creational::abstract_factory
