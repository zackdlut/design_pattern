#pragma once

#include <string>

namespace design_pattern::behavioral::template_method {

// MethodTemplate。execute 是模板方法，顺序写死：先 common，再 unique。
// common 是公共步骤。子类只覆盖 unique。客户端不自己排这两步。
class MethodTemplate {
public:
  virtual ~MethodTemplate() = default;

  std::string execute() const;

protected:
  std::string common() const;
  virtual std::string unique() const = 0;
};

// 具体类。unique 按 A 来。
class MethodA final : public MethodTemplate {
protected:
  std::string unique() const override;
};

// 具体类。unique 按 B 来。
class MethodB final : public MethodTemplate {
protected:
  std::string unique() const override;
};

}  // namespace design_pattern::behavioral::template_method
