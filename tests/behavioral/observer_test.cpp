#include "behavioral/observer/observer.h"

#include <gtest/gtest.h>

#include <stdexcept>
#include <type_traits>

using design_pattern::behavioral::observer::Stock;
using design_pattern::behavioral::observer::StockObserver;

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

TEST(ObserverTest, StockObserverReadsTheSubject) {
  Stock stock("AAPL", 120);
  StockObserver observer;
  observer.update(stock);
  EXPECT_EQ(observer.text(), "AAPL 120");
}

TEST(ObserverTest, StockNotifiesAttachedObservers) {
  Stock stock("AAPL", 100);
  StockObserver first;
  StockObserver second;
  stock.attach(&first);
  stock.attach(&second);

  EXPECT_EQ(first.text(), "");
  EXPECT_EQ(second.text(), "");

  stock.setPrice(120);
  EXPECT_EQ(first.text(), "AAPL 120");
  EXPECT_EQ(second.text(), "AAPL 120");
}

TEST(ObserverTest, DetachStopsFurtherUpdates) {
  Stock stock("AAPL", 100);
  StockObserver first;
  StockObserver second;
  stock.attach(&first);
  stock.attach(&second);

  stock.detach(&second);
  stock.setPrice(120);

  EXPECT_EQ(first.text(), "AAPL 120");
  EXPECT_EQ(second.text(), "");
}

TEST(ObserverTest, OneObserverCanWatchTwoSubjects) {
  Stock aapl("AAPL", 100);
  Stock msft("MSFT", 100);
  StockObserver observer;
  aapl.attach(&observer);
  msft.attach(&observer);

  aapl.setPrice(120);
  EXPECT_EQ(observer.text(), "AAPL 120");
  msft.setPrice(300);
  EXPECT_EQ(observer.text(), "MSFT 300");
}

TEST(ObserverTest, BadObserverIsRejected) {
  Stock stock("AAPL", 100);
  StockObserver observer;
  stock.attach(&observer);

  expect_invalid("observer is required", [&] { stock.attach(nullptr); });
  expect_invalid("observer is required", [&] { stock.detach(nullptr); });
  expect_invalid("observer is already attached",
                 [&] { stock.attach(&observer); });

  StockObserver other;
  expect_invalid("observer is not attached", [&] { stock.detach(&other); });

  stock.setPrice(120);
  EXPECT_EQ(observer.text(), "AAPL 120");
}

TEST(ObserverTest, CopiedObserverIsNotSubscribed) {
  Stock stock("AAPL", 100);
  StockObserver observer;
  stock.attach(&observer);
  StockObserver copy = observer;

  stock.setPrice(120);
  EXPECT_EQ(observer.text(), "AAPL 120");
  EXPECT_EQ(copy.text(), "");

  static_assert(!std::is_copy_constructible_v<Stock>);
  static_assert(!std::is_move_constructible_v<Stock>);
}

TEST(ObserverTest, EmptyNameIsRejected) {
  expect_invalid("name is required", [] { Stock stock("", 1); });
}
