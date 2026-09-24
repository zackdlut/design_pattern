#include "behavioral/command/command.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

using design_pattern::behavioral::command::AppendCommand;
using design_pattern::behavioral::command::Command;
using design_pattern::behavioral::command::CommandQueue;
using design_pattern::behavioral::command::Invoker;
using design_pattern::behavioral::command::LogCommand;
using design_pattern::behavioral::command::Receiver;
using design_pattern::behavioral::command::TextBuffer;

namespace {

void expect_invalid(const char *message, auto &&action) {
  try {
    action();
    FAIL() << "expected std::invalid_argument: " << message;
  } catch (const std::invalid_argument &error) {
    EXPECT_STREQ(error.what(), message);
  }
}

}  // namespace

TEST(CommandTest, AppendCommandMutatesReceiver) {
  TextBuffer buffer;
  AppendCommand command(&buffer, "hello");
  EXPECT_EQ(buffer.text(), "");
  command.execute();
  EXPECT_EQ(buffer.text(), "hello");
  command.undo();
  EXPECT_EQ(buffer.text(), "");
}

TEST(CommandTest, LogCommandWritesExternalLog) {
  std::string log;
  LogCommand command(&log, "started");
  command.execute();
  EXPECT_EQ(log, "started");
  command.undo();
  EXPECT_EQ(log, "");
}

TEST(CommandTest, CommandQueueRunsMacroInOrder) {
  TextBuffer buffer;
  CommandQueue macro;
  macro.addCommand(std::make_unique<AppendCommand>(&buffer, "a"));
  macro.addCommand(std::make_unique<AppendCommand>(&buffer, "b"));
  macro.execute();
  EXPECT_EQ(buffer.text(), "ab");
  macro.undo();
  EXPECT_EQ(buffer.text(), "");
}

TEST(CommandTest, InvokerExecutesAndUndoes) {
  TextBuffer buffer;
  Invoker remote(std::make_unique<AppendCommand>(&buffer, "on"));
  remote.executeCommand();
  EXPECT_EQ(buffer.text(), "on");
  EXPECT_TRUE(remote.canUndo());
  remote.undoCommand();
  EXPECT_EQ(buffer.text(), "");
  EXPECT_FALSE(remote.canUndo());
}

TEST(CommandTest, InvokerSwapsPendingCommand) {
  TextBuffer buffer;
  Invoker remote(std::make_unique<AppendCommand>(&buffer, "a"));
  remote.setCommand(std::make_unique<AppendCommand>(&buffer, "b"));
  remote.executeCommand();
  EXPECT_EQ(buffer.text(), "b");
}

TEST(CommandTest, NullArgumentsAreRejected) {
  TextBuffer buffer;
  expect_invalid("receiver is required", [] { AppendCommand command(nullptr, "x"); });

  std::string log;
  expect_invalid("log is required", [] { LogCommand command(nullptr, "x"); });

  CommandQueue macro;
  expect_invalid("command is required",
                 [&] { macro.addCommand(nullptr); });

  expect_invalid("command is required", [] {
    Invoker remote(std::unique_ptr<Command>{nullptr});
  });

  Invoker empty;
  expect_invalid("command is required", [&] { empty.executeCommand(); });

  Invoker remote(std::make_unique<AppendCommand>(&buffer, "a"));
  expect_invalid("command is required", [&] { remote.setCommand(nullptr); });
  remote.executeCommand();
  expect_invalid("command is required", [&] { remote.executeCommand(); });

  Invoker fresh;
  expect_invalid("nothing to undo", [&] { fresh.undoCommand(); });
}

TEST(CommandTest, RemoveSuffixRejectsOverlongCount) {
  TextBuffer buffer;
  buffer.append("hi");
  expect_invalid("suffix length exceeds text",
                 [&] { buffer.removeSuffix(3); });
  buffer.removeSuffix(2);
  EXPECT_EQ(buffer.text(), "");
}

TEST(CommandTest, PolymorphicBasesRejectCopyAndMove) {
  static_assert(!std::is_copy_constructible_v<Command>);
  static_assert(!std::is_move_constructible_v<Command>);
  static_assert(!std::is_copy_constructible_v<Receiver>);
  static_assert(!std::is_move_constructible_v<Receiver>);
}
