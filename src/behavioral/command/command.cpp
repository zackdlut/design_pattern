#include "behavioral/command/command.h"

#include <stdexcept>
#include <utility>

namespace design_pattern::behavioral::command {

namespace {

Receiver *require_receiver(Receiver *receiver) {
  if (receiver == nullptr) {
    throw std::invalid_argument("receiver is required");
  }
  return receiver;
}

std::string *require_log(std::string *log) {
  if (log == nullptr) {
    throw std::invalid_argument("log is required");
  }
  return log;
}

std::unique_ptr<Command> require_command(std::unique_ptr<Command> command) {
  if (!command) {
    throw std::invalid_argument("command is required");
  }
  return command;
}

void require_suffix(std::string_view text, std::size_t count) {
  if (count > text.size()) {
    throw std::invalid_argument("suffix length exceeds text");
  }
}

}  // namespace

std::string_view TextBuffer::text() const { return text_; }

void TextBuffer::append(std::string_view chunk) { text_.append(chunk); }

void TextBuffer::removeSuffix(std::size_t count) {
  require_suffix(text_, count);
  text_.resize(text_.size() - count);
}

AppendCommand::AppendCommand(Receiver *receiver, std::string chunk)
    : receiver_(require_receiver(receiver)), chunk_(std::move(chunk)) {}

void AppendCommand::execute() { receiver_->append(chunk_); }

void AppendCommand::undo() { receiver_->removeSuffix(chunk_.size()); }

LogCommand::LogCommand(std::string *log, std::string line)
    : log_(require_log(log)), line_(std::move(line)) {}

void LogCommand::execute() {
  if (!log_->empty()) {
    log_->push_back('\n');
  }
  log_->append(line_);
}

void LogCommand::undo() {
  if (log_->size() < line_.size()) {
    throw std::invalid_argument("log is shorter than line");
  }
  const std::size_t trailing_newline =
      log_->size() == line_.size() ? 0 : 1;
  log_->resize(log_->size() - line_.size() - trailing_newline);
}

void CommandQueue::addCommand(std::unique_ptr<Command> command) {
  commands_.push_back(require_command(std::move(command)));
}

void CommandQueue::execute() {
  for (const auto &command : commands_) {
    command->execute();
  }
}

void CommandQueue::undo() {
  for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
    (*it)->undo();
  }
}

Invoker::Invoker(std::unique_ptr<Command> command)
    : pending_(require_command(std::move(command))) {}

void Invoker::setCommand(std::unique_ptr<Command> command) {
  pending_ = require_command(std::move(command));
}

void Invoker::executeCommand() {
  auto command = require_command(std::move(pending_));
  command->execute();
  history_.push_back(std::move(command));
}

void Invoker::undoCommand() {
  if (history_.empty()) {
    throw std::invalid_argument("nothing to undo");
  }
  std::unique_ptr<Command> command = std::move(history_.back());
  history_.pop_back();
  command->undo();
  pending_ = std::move(command);
}

bool Invoker::canUndo() const { return !history_.empty(); }

}  // namespace design_pattern::behavioral::command
