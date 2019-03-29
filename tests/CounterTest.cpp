#include <gtest/gtest.h>
#include "MsgCounter.h"

class TestMsgCounter : public ::testing::Test {
 public:
  TestMsgCounter() {}
  ~TestMsgCounter() {}
};

class MsgCounterTestFixture : public DeviceListener::MsgCounter {
 public:
  std::pair<uint64_t, bool> getStatForDevice(uint16_t id) {
    return stats_[id];
  };

  void forceSetCounterForDevice(uint16_t id, uint64_t value, bool overflow) {
    stats_[id] = std::make_pair(value, overflow);
  }
};

TEST_F(TestMsgCounter, EmptyCounter) {
  MsgCounterTestFixture counter;

  const uint16_t testDevId1 = 0;
  const uint16_t testDevId2 = 5;

  ASSERT_EQ(counter.getStatForDevice(testDevId1),
            std::make_pair(static_cast<uint64_t>(0), false));
  ASSERT_EQ(counter.getStatForDevice(testDevId2),
            std::make_pair(static_cast<uint64_t>(0), false));
}

TEST_F(TestMsgCounter, CountSome) {
  MsgCounterTestFixture counter;

  const uint16_t testDevId1 = 0;
  const uint16_t testDevId2 = 5;

  counter.incrementCounter(testDevId1);
  ASSERT_EQ(counter.getStatForDevice(testDevId1),
            std::make_pair(static_cast<uint64_t>(1), false));
  ASSERT_EQ(counter.getStatForDevice(testDevId2),
            std::make_pair(static_cast<uint64_t>(0), false));
}

TEST_F(TestMsgCounter, CountOverflow) {
  MsgCounterTestFixture counter;

  const uint16_t testDevId1 = 0;

  uint64_t countLimit = std::numeric_limits<uint64_t>::max();

  counter.forceSetCounterForDevice(testDevId1, countLimit - 1, false);
  counter.incrementCounter(testDevId1);
  ASSERT_EQ(counter.getStatForDevice(testDevId1),
            std::make_pair(countLimit, false));

  counter.incrementCounter(testDevId1);
  ASSERT_EQ(counter.getStatForDevice(testDevId1),
            std::make_pair(countLimit, true));
}