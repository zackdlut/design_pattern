#include "behavioral/observer/observer.h"

#include <iostream>
#include <stdexcept>

using design_pattern::behavioral::observer::Stock;
using design_pattern::behavioral::observer::StockObserver;

int main() {
  std::cout << "=== setPrice 通知所有 StockObserver ===\n";
  Stock stock("AAPL", 100);
  StockObserver first;
  StockObserver second;
  stock.attach(&first);
  stock.attach(&second);
  stock.setPrice(120);
  std::cout << "  " << first.text() << "\n";
  std::cout << "  " << second.text() << "\n";

  stock.detach(&second);
  stock.setPrice(125);
  std::cout << "  仍在订阅 " << first.text() << "\n";
  std::cout << "  已摘掉 " << second.text() << "\n";

  std::cout << "\n=== 空观察者在入口拒绝 ===\n";
  try {
    stock.attach(nullptr);
  } catch (const std::invalid_argument &error) {
    std::cout << "  " << error.what() << "\n";
  }

  return 0;
}
