#include "behavioral/strategy/strategy.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace design_pattern::behavioral::strategy {

namespace {

std::string route(std::string_view mode, std::string_view from,
                  std::string_view to) {
  return std::string(mode) + " " + std::string(from) + " -> " + std::string(to);
}

std::unique_ptr<Strategy> require_strategy(std::unique_ptr<Strategy> strategy) {
  if (!strategy) {
    throw std::invalid_argument("strategy is required");
  }
  return strategy;
}

TravelMode require_mode(TravelMode mode) {
  switch (mode) {
  case TravelMode::Walk:
  case TravelMode::Drive:
    return mode;
  }
  throw std::invalid_argument("mode is unknown");
}

}  // namespace

std::string WalkStrategy::execute(std::string_view from,
                                  std::string_view to) const {
  return route("walk", from, to);
}

std::string DriveStrategy::execute(std::string_view from,
                                   std::string_view to) const {
  return route("drive", from, to);
}

Navigator::Navigator(std::unique_ptr<Strategy> strategy)
    : strategy_(require_strategy(std::move(strategy))) {}

void Navigator::setStrategy(std::unique_ptr<Strategy> strategy) {
  strategy_ = require_strategy(std::move(strategy));
}

std::string Navigator::execute(std::string_view from,
                               std::string_view to) const {
  if (!strategy_) {
    throw std::invalid_argument("strategy is required");
  }
  return strategy_->execute(from, to);
}

InlineNavigator::InlineNavigator(TravelMode mode)
    : mode_(require_mode(mode)) {}

void InlineNavigator::setMode(TravelMode mode) { mode_ = require_mode(mode); }

std::string InlineNavigator::execute(std::string_view from,
                                     std::string_view to) const {
  switch (mode_) {
  case TravelMode::Walk:
    return route("walk", from, to);
  case TravelMode::Drive:
    return route("drive", from, to);
  }
  throw std::invalid_argument("mode is unknown");
}

}  // namespace design_pattern::behavioral::strategy
