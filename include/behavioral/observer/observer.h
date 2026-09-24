#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace design_pattern::behavioral::observer {

class Subject;

// Observer。订阅方。update 里自己读主题。
class Observer {
public:
  virtual ~Observer() = default;
  virtual void update(const Subject &subject) = 0;
};

// Subject。发布方。只认识 Observer。
// 名单里是指针，Subject 不拥有观察者。先 detach，再销毁观察者。
class Subject {
public:
  virtual ~Subject() = default;

  Subject(const Subject &) = delete;
  Subject &operator=(const Subject &) = delete;
  Subject(Subject &&) = delete;
  Subject &operator=(Subject &&) = delete;

  void attach(Observer *observer);
  void detach(Observer *observer);
  void notify() const;

  virtual std::string_view name() const = 0;
  virtual int price() const = 0;

protected:
  Subject() = default;

private:
  std::vector<Observer *> observers_;
};

// 具体主题。setPrice 写下价格再 notify。构造时不通知。
class Stock final : public Subject {
public:
  Stock(std::string name, int price);

  void setPrice(int price);

  std::string_view name() const override;
  int price() const override;

private:
  std::string name_;
  int price_ = 0;
};

// 具体观察者。记下最近一次读到的代码和价格。
class StockObserver final : public Observer {
public:
  void update(const Subject &subject) override;
  std::string text() const;

private:
  std::string text_;
};

}  // namespace design_pattern::behavioral::observer
