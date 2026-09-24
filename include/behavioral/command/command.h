#pragma once
#include <string>
#include <vector>
#include <iostream>
// TODO: implement the Command (命令) pattern yourself.

namespace design_pattern::behavioral::command {

class Command {
public:
  virtual ~Command() = default;
  virtual void execute() = 0;
};

class Receiver {
public:
  virtual ~Receiver() = default;
  std::string action() { return "Receiver: action"; }
};

class ComplexCommand : public Command {
public:
  ComplexCommand(Receiver *receiver) : receiver_(receiver) {}
  void execute() override { receiver_->action(); }

private:
  Receiver *receiver_;
};

class SimpleCommand : public Command {
public:
  SimpleCommand(const std::string &payload) : payload_(payload) {}
  void execute() override { std::cout << "SimpleCommand: " << payload_ << std::endl; }

private:
  std::string payload_;
};

class Invoker {
public:
  Invoker(Command *command) : command_(command) {}
  void setCommand(Command *command) { command_ = command; }
  void executeCommand() { command_->execute(); }
private:
  Command *command_;
};

class CommandQueue {
public:
  CommandQueue() : commands_() {}
  void addCommand(Command *command) { commands_.push_back(command); }
  void executeCommands() {
    for (auto command : commands_) {
      command->execute();
    }
  }
private:
  std::vector<Command *> commands_;
};

} // namespace design_pattern::behavioral::command
