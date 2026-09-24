#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace design_pattern::behavioral::command {

// Command。把请求封装成对象。execute / undo 成对出现，便于撤销与排队。
class Command {
public:
  virtual ~Command() = default;

  Command(const Command &) = delete;
  Command &operator=(const Command &) = delete;
  Command(Command &&) = delete;
  Command &operator=(Command &&) = delete;

  virtual void execute() = 0;
  virtual void undo() = 0;

protected:
  Command() = default;
};

// Receiver。真正干活的对象。命令只持有指针，不拥有接收者。
class Receiver {
public:
  virtual ~Receiver() = default;

  Receiver(const Receiver &) = delete;
  Receiver &operator=(const Receiver &) = delete;
  Receiver(Receiver &&) = delete;
  Receiver &operator=(Receiver &&) = delete;

  virtual std::string_view text() const = 0;
  virtual void append(std::string_view chunk) = 0;
  virtual void removeSuffix(std::size_t count) = 0;

protected:
  Receiver() = default;
};

// 具体接收者。append / removeSuffix 改同一段缓冲。
class TextBuffer final : public Receiver {
public:
  std::string_view text() const override;
  void append(std::string_view chunk) override;
  void removeSuffix(std::size_t count) override;

private:
  std::string text_;
};

// 具体命令。把一段文字 append 到接收者；undo 按长度删掉后缀。
class AppendCommand final : public Command {
public:
  explicit AppendCommand(Receiver *receiver, std::string chunk);

  void execute() override;
  void undo() override;

private:
  Receiver *receiver_;
  std::string chunk_;
};

// 具体命令。不碰接收者，只往外部日志串里写一行；undo 删掉该行。
class LogCommand final : public Command {
public:
  LogCommand(std::string *log, std::string line);

  void execute() override;
  void undo() override;

private:
  std::string *log_;
  std::string line_;
};

// 宏命令 / 队列。按加入顺序 execute，按相反顺序 undo。拥有子命令。
class CommandQueue final : public Command {
public:
  CommandQueue() = default;

  void addCommand(std::unique_ptr<Command> command);
  void execute() override;
  void undo() override;

private:
  std::vector<std::unique_ptr<Command>> commands_;
};

// Invoker。保存待执行的命令，执行后移入历史栈，undo 弹栈。
class Invoker {
public:
  Invoker() = default;
  explicit Invoker(std::unique_ptr<Command> command);

  void setCommand(std::unique_ptr<Command> command);
  void executeCommand();
  void undoCommand();
  bool canUndo() const;

private:
  std::unique_ptr<Command> pending_;
  std::vector<std::unique_ptr<Command>> history_;
};

}  // namespace design_pattern::behavioral::command
