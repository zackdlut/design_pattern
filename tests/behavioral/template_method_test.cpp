#include "behavioral/template_method/template_method.h"

#include <gtest/gtest.h>

using design_pattern::behavioral::template_method::MethodB;
using design_pattern::behavioral::template_method::MethodA;

TEST(TemplateMethodTest, MethodARunsCommonThenUnique) {
  MethodA method;
  EXPECT_EQ(method.execute(), "common method-a");
}

TEST(TemplateMethodTest, MethodBRunsCommonThenUnique) {
  MethodB method;
  EXPECT_EQ(method.execute(), "common method-b");
}
