#include "behavioral/strategy/strategy.h"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

using design_pattern::behavioral::strategy::DriveStrategy;
using design_pattern::behavioral::strategy::InlineNavigator;
using design_pattern::behavioral::strategy::Navigator;
using design_pattern::behavioral::strategy::StaticStrategy;
using design_pattern::behavioral::strategy::Strategy;
using design_pattern::behavioral::strategy::TravelMode;
using design_pattern::behavioral::strategy::WalkStrategy;

namespace {

void expect_invalid(const char *message, auto &&action) {
  try {
    action();
    FAIL() << "expected std::invalid_argument: " << message;
  } catch (const std::invalid_argument &error) {
    EXPECT_STREQ(error.what(), message);
  }
}

const char *kWalk = "walk home -> office";
const char *kDrive = "drive home -> office";

}  // namespace

TEST(StrategyTest, WalkAndDriveExecuteDifferently) {
  WalkStrategy walk;
  DriveStrategy drive;
  EXPECT_EQ(walk.execute("home", "office"), kWalk);
  EXPECT_EQ(drive.execute("home", "office"), kDrive);
}

TEST(StrategyTest, NavigatorForwardsAndSwapsStrategy) {
  Navigator navigator(std::make_unique<WalkStrategy>());
  EXPECT_EQ(navigator.execute("home", "office"), kWalk);

  navigator.setStrategy(std::make_unique<DriveStrategy>());
  EXPECT_EQ(navigator.execute("home", "office"), kDrive);
}

TEST(StrategyTest, NullStrategyIsRejected) {
  expect_invalid("strategy is required", [] { Navigator navigator(nullptr); });

  Navigator navigator(std::make_unique<WalkStrategy>());
  expect_invalid("strategy is required",
                 [&] { navigator.setStrategy(nullptr); });
  EXPECT_EQ(navigator.execute("home", "office"), kWalk);
}

TEST(StrategyTest, StaticStrategySelectsExecuteByType) {
  StaticStrategy<WalkStrategy> walk;
  StaticStrategy<DriveStrategy> drive;
  EXPECT_EQ(walk.execute("home", "office"), kWalk);
  EXPECT_EQ(drive.execute("home", "office"), kDrive);

  static_assert(std::derived_from<WalkStrategy, Strategy>);
  static_assert(std::derived_from<DriveStrategy, Strategy>);
  static_assert(!std::is_base_of_v<Strategy, StaticStrategy<WalkStrategy>>);
}

TEST(StrategyTest, InlineNavigatorMatchesTheClosedCase) {
  InlineNavigator walking(TravelMode::Walk);
  InlineNavigator driving(TravelMode::Drive);
  EXPECT_EQ(walking.execute("home", "office"),
            WalkStrategy{}.execute("home", "office"));
  EXPECT_EQ(driving.execute("home", "office"),
            DriveStrategy{}.execute("home", "office"));
}

TEST(StrategyTest, InlineNavigatorCopiesByValue) {
  InlineNavigator spec(TravelMode::Walk);
  InlineNavigator copy = spec;
  copy.setMode(TravelMode::Drive);

  EXPECT_EQ(spec.execute("home", "office"), kWalk);
  EXPECT_EQ(copy.execute("home", "office"), kDrive);
}

TEST(StrategyTest, InlineNavigatorRejectsUnknownMode) {
  expect_invalid("mode is unknown", [] {
    InlineNavigator navigator(static_cast<TravelMode>(7));
  });

  InlineNavigator navigator(TravelMode::Walk);
  expect_invalid("mode is unknown",
                 [&] { navigator.setMode(static_cast<TravelMode>(7)); });
  EXPECT_EQ(navigator.execute("home", "office"), kWalk);
}
