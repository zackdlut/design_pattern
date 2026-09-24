#include "behavioral/command/command.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using design_pattern::behavioral::command::AppendCommand;
using design_pattern::behavioral::command::Command;
using design_pattern::behavioral::command::CommandQueue;
using design_pattern::behavioral::command::Invoker;
using design_pattern::behavioral::command::LogCommand;
using design_pattern::behavioral::command::TextBuffer;

int main() {
  std::cout << "=== Invoker：按按钮执行，再 undo ===\n";
  TextBuffer buffer;
  Invoker remote(std::make_unique<AppendCommand>(&buffer, "开灯"));
  remote.executeCommand();
  std::cout << "  缓冲: " << buffer.text() << "\n";
  remote.undoCommand();
  std::cout << "  撤销后: " << buffer.text() << "\n";

  std::cout << "\n=== CommandQueue：宏命令顺序执行、逆序撤销 ===\n";
  TextBuffer macro_buffer;
  std::string log;
  CommandQueue macro;
  macro.addCommand(std::make_unique<AppendCommand>(&macro_buffer, "Hello"));
  macro.addCommand(std::make_unique<LogCommand>(&log, "macro done"));
  macro.execute();
  std::cout << "  缓冲: " << macro_buffer.text() << "\n";
  std::cout << "  日志:\n" << log << "\n";
  macro.undo();
  std::cout << "  撤销后缓冲: " << macro_buffer.text() << "\n";
  std::cout << "  撤销后日志: '" << log << "'\n";

  std::cout << "\n=== 空命令在入口拒绝 ===\n";
  try {
    Invoker missing(std::unique_ptr<Command>{nullptr});
    (void)missing;
  } catch (const std::invalid_argument &error) {
    std::cout << "  " << error.what() << "\n";
  }

  return 0;
}
