#include "behavioral/observer/observer.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace design_pattern::behavioral::observer {

namespace {

std::string quote(std::string_view name, int price) {
  return std::string(name) + " " + std::to_string(price);
}

std::string require_name(std::string name) {
  if (name.empty()) {
    throw std::invalid_argument("name is required");
  }
  return name;
}

}  // namespace

void Subject::attach(Observer *observer) {
  if (observer == nullptr) {
    throw std::invalid_argument("observer is required");
  }
  const auto found = std::find(observers_.begin(), observers_.end(), observer);
  if (found != observers_.end()) {
    throw std::invalid_argument("observer is already attached");
  }
  observers_.push_back(observer);
}

void Subject::detach(Observer *observer) {
  if (observer == nullptr) {
    throw std::invalid_argument("observer is required");
  }
  const auto found = std::find(observers_.begin(), observers_.end(), observer);
  if (found == observers_.end()) {
    throw std::invalid_argument("observer is not attached");
  }
  observers_.erase(found);
}

void Subject::notify() const {
  const std::vector<Observer *> observers = observers_;
  for (Observer *observer : observers) {
    observer->update(*this);
  }
}

Stock::Stock(std::string name, int price)
    : name_(require_name(std::move(name))), price_(price) {}

void Stock::setPrice(int price) {
  price_ = price;
  notify();
}

std::string_view Stock::name() const { return name_; }

int Stock::price() const { return price_; }

void StockObserver::update(const Subject &subject) {
  text_ = quote(subject.name(), subject.price());
}

std::string StockObserver::text() const { return text_; }

}  // namespace design_pattern::behavioral::observer
