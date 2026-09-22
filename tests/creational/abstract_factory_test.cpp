#include "creational/abstract_factory/abstract_factory.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

using design_pattern::creational::abstract_factory::AbstractFactory;
using design_pattern::creational::abstract_factory::Button;
using design_pattern::creational::abstract_factory::CheckBox;
using design_pattern::creational::abstract_factory::GuiFactorySelector;
using design_pattern::creational::abstract_factory::GuiTheme;
using design_pattern::creational::abstract_factory::MacButton;
using design_pattern::creational::abstract_factory::MacCheckBox;
using design_pattern::creational::abstract_factory::MacGUIFactory;
using design_pattern::creational::abstract_factory::WindowsButton;
using design_pattern::creational::abstract_factory::WindowsCheckBox;
using design_pattern::creational::abstract_factory::WindowsGUIFactory;

namespace {

std::string render(const AbstractFactory &factory) {
  auto button = factory.createButton();
  auto checkbox = factory.createCheckBox();
  return button->paint() + " | " + checkbox->paint();
}

} // namespace

TEST(AbstractFactoryTest, WindowsFactoryCreatesWindowsFamily) {
  WindowsGUIFactory factory;
  std::unique_ptr<Button> button = factory.createButton();
  std::unique_ptr<CheckBox> checkbox = factory.createCheckBox();
  ASSERT_NE(button, nullptr);
  ASSERT_NE(checkbox, nullptr);
  EXPECT_NE(dynamic_cast<WindowsButton *>(button.get()), nullptr);
  EXPECT_EQ(dynamic_cast<MacButton *>(button.get()), nullptr);
  EXPECT_NE(dynamic_cast<WindowsCheckBox *>(checkbox.get()), nullptr);
  EXPECT_EQ(dynamic_cast<MacCheckBox *>(checkbox.get()), nullptr);
  EXPECT_EQ(button->paint(), "WindowsButton paint");
  EXPECT_EQ(checkbox->paint(), "WindowsCheckBox paint");
}

TEST(AbstractFactoryTest, MacFactoryCreatesMacFamily) {
  MacGUIFactory factory;
  auto button = factory.createButton();
  auto checkbox = factory.createCheckBox();
  ASSERT_NE(button, nullptr);
  ASSERT_NE(checkbox, nullptr);
  EXPECT_NE(dynamic_cast<MacButton *>(button.get()), nullptr);
  EXPECT_EQ(dynamic_cast<WindowsButton *>(button.get()), nullptr);
  EXPECT_NE(dynamic_cast<MacCheckBox *>(checkbox.get()), nullptr);
  EXPECT_EQ(dynamic_cast<WindowsCheckBox *>(checkbox.get()), nullptr);
  EXPECT_EQ(button->paint(), "MacButton paint");
  EXPECT_EQ(checkbox->paint(), "MacCheckBox paint");
}

TEST(AbstractFactoryTest, ClientDependsOnAbstractFactory) {
  WindowsGUIFactory windows;
  MacGUIFactory mac;
  const AbstractFactory &as_windows = windows;
  const AbstractFactory &as_mac = mac;
  EXPECT_EQ(render(as_windows), "WindowsButton paint | WindowsCheckBox paint");
  EXPECT_EQ(render(as_mac), "MacButton paint | MacCheckBox paint");
}

TEST(AbstractFactoryTest, CopyAndMoveAreDeleted) {
  static_assert(!std::is_copy_constructible_v<Button>);
  static_assert(!std::is_move_constructible_v<Button>);
  static_assert(!std::is_copy_assignable_v<Button>);
  static_assert(!std::is_move_assignable_v<Button>);
  static_assert(!std::is_copy_constructible_v<CheckBox>);
  static_assert(!std::is_move_constructible_v<CheckBox>);
  static_assert(!std::is_copy_constructible_v<AbstractFactory>);
  static_assert(!std::is_move_constructible_v<AbstractFactory>);
  static_assert(!std::is_copy_constructible_v<WindowsButton>);
  static_assert(!std::is_copy_constructible_v<WindowsGUIFactory>);
  static_assert(!std::is_default_constructible_v<GuiFactorySelector>);
}

TEST(AbstractFactoryTest, GuiFactorySelectorCreatesWindowsFamily) {
  auto factory = GuiFactorySelector::create(GuiTheme::Windows);
  ASSERT_NE(factory, nullptr);
  EXPECT_NE(dynamic_cast<WindowsGUIFactory *>(factory.get()), nullptr);
  EXPECT_EQ(dynamic_cast<MacGUIFactory *>(factory.get()), nullptr);
  EXPECT_EQ(render(*factory), "WindowsButton paint | WindowsCheckBox paint");
}

TEST(AbstractFactoryTest, GuiFactorySelectorCreatesMacFamily) {
  auto factory = GuiFactorySelector::create(GuiTheme::Mac);
  ASSERT_NE(factory, nullptr);
  EXPECT_NE(dynamic_cast<MacGUIFactory *>(factory.get()), nullptr);
  EXPECT_EQ(dynamic_cast<WindowsGUIFactory *>(factory.get()), nullptr);
  EXPECT_EQ(render(*factory), "MacButton paint | MacCheckBox paint");
}

TEST(AbstractFactoryTest, GuiFactorySelectorRejectsUnknownTheme) {
  EXPECT_THROW(GuiFactorySelector::create(static_cast<GuiTheme>(99)),
               std::invalid_argument);
}
