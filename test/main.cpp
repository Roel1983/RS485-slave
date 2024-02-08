#include <gtest/gtest.h>

#include <avr/io.h>

#include "fakeavr/fakeavr.h"

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  GTEST_FLAG_SET(death_test_style, "fast");
  testing::UnitTest::GetInstance()->listeners().Append(new FakeAvrTestEventListener);
  return RUN_ALL_TESTS();
}
