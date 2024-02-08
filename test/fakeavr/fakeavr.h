#ifndef FAKEAVR_FAKEAVR_H
#define FAKEAVR_FAKEAVR_H

class FakeAvrTestEventListener : public testing::EmptyTestEventListener {
	void OnTestStart(const testing::TestInfo& test_info) override;
};

#endif
