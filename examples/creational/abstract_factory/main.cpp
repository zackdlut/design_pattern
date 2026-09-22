#include "creational/abstract_factory/abstract_factory.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using design_pattern::creational::abstract_factory::AbstractFactory;
using design_pattern::creational::abstract_factory::GuiFactorySelector;
using design_pattern::creational::abstract_factory::GuiTheme;
using design_pattern::creational::abstract_factory::MacGUIFactory;
using design_pattern::creational::abstract_factory::WindowsGUIFactory;

namespace {

struct UiJob {
  std::string screen;
  std::string theme; // "windows" -> Windows 族，"mac" -> Mac 族
};

std::unique_ptr<AbstractFactory> make_gui_factory(std::string_view theme) {
  if (theme == "windows") {
    return std::make_unique<WindowsGUIFactory>();
  }
  if (theme == "mac") {
    return std::make_unique<MacGUIFactory>();
  }
  throw std::invalid_argument("unknown theme: " + std::string(theme));
}

GuiTheme parse_gui_theme(std::string_view name) {
  if (name == "windows") {
    return GuiTheme::Windows;
  }
  if (name == "mac") {
    return GuiTheme::Mac;
  }
  throw std::invalid_argument("unknown gui theme: " + std::string(name));
}

// 客户端只依赖 AbstractFactory：换工厂即换整族控件，不会混用 Win 按钮 + Mac 复选框。
void render_dialog(const AbstractFactory &factory, std::string_view screen) {
  auto button = factory.createButton();
  auto checkbox = factory.createCheckBox();
  std::cout << "  [" << screen << "] " << button->paint() << " | "
            << checkbox->paint() << "\n";
}

} // namespace

int main() {
  std::cout << "=== 抽象工厂：设置页按主题渲染对话框，控件必须同族 ===\n";

  const std::vector<UiJob> jobs{
      {"login", "windows"},
      {"settings", "mac"},
      {"about", "windows"},
  };

  for (const auto &job : jobs) {
    auto factory = make_gui_factory(job.theme);
    std::cout << "[" << job.theme << "]\n";
    render_dialog(*factory, job.screen);
  }

  std::cout << "\n=== 同一抽象，换具体工厂即换产品族 ===\n";
  WindowsGUIFactory windows;
  MacGUIFactory mac;
  const AbstractFactory &as_windows = windows;
  const AbstractFactory &as_mac = mac;
  render_dialog(as_windows, "via AbstractFactory& (windows)");
  render_dialog(as_mac, "via AbstractFactory& (mac)");

  std::cout << "\n=== 对照：按配置枚举选工厂，渲染流程仍然只依赖抽象 ===\n";
  for (std::string_view theme_name : {"windows", "mac"}) {
    auto factory = GuiFactorySelector::create(parse_gui_theme(theme_name));
    std::cout << "  config=" << theme_name << "\n";
    render_dialog(*factory, "via GuiFactorySelector");
  }

  return 0;
}
