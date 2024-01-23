#include <gtest/gtest.h>

#include "avr/io.h"

#include "fakeavr.h"


void FakeAvrTestEventListener::OnTestStart(const testing::TestInfo& test_info) {
	FakeIoReset();
}
