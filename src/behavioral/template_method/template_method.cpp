#include "behavioral/template_method/template_method.h"

#include <string>

namespace design_pattern::behavioral::template_method {

std::string MethodTemplate::execute() const {
  return common() + " " + unique();
}

std::string MethodTemplate::common() const { return "common"; }

std::string MethodA::unique() const { return "method-a"; }

std::string MethodB::unique() const { return "method-b"; }

}  // namespace design_pattern::behavioral::template_method
