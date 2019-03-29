#include <gtest/gtest.h>
#include "RfcTransport.h"

class TestRfcTransport : public ::testing::Test {
 public:
  TestRfcTransport() {}
  ~TestRfcTransport() {}
};

TEST_F(TestRfcTransport, RfcHeaderValid) {
  DeviceListener::RfcMessage message;
  message.headerBuffer = {DeviceListener::RfcMessage::kProtocolVersion, 0,
                          4096};
  auto result = message.validateHeaderAndGetLength();
  ASSERT_EQ(result.get(), 4096);
}

TEST_F(TestRfcTransport, RfcHeaderBadVersion) {
  DeviceListener::RfcMessage message;
  message.headerBuffer = {DeviceListener::RfcMessage::kProtocolVersion + 5, 0,
                          4096};
  auto result = message.validateHeaderAndGetLength();

  ASSERT_EQ(result, boost::none);
}

TEST_F(TestRfcTransport, RfcHeaderBadLengthTooBig) {
  DeviceListener::RfcMessage message;
  message.headerBuffer = {DeviceListener::RfcMessage::kProtocolVersion, 0,
                          8192};
  auto result = message.validateHeaderAndGetLength();
  ASSERT_EQ(result, boost::none);
}

TEST_F(TestRfcTransport, RfcHeaderBadLengthTooSmall) {
  DeviceListener::RfcMessage message;
  message.headerBuffer = {DeviceListener::RfcMessage::kProtocolVersion, 0, 4};
  auto result = message.validateHeaderAndGetLength();
  ASSERT_EQ(result, boost::none);
}

TEST_F(TestRfcTransport, PayloadValid) {
  DeviceListener::RfcMessage message;
  const uint16_t testDevId = 5;
  message.payloadBuffer.resize(
      sizeof(DeviceListener::RfcMessage::PayloadHeader));
  message.payloadBuffer[0] = testDevId;
  auto result = message.getDevIdFromBuffer();
  ASSERT_EQ(result.get(), testDevId);
}

TEST_F(TestRfcTransport, PayloadTooSmall) {
  DeviceListener::RfcMessage message;
  const uint16_t testDevId = 5;
  message.payloadBuffer.resize(
      sizeof(DeviceListener::RfcMessage::PayloadHeader) - 1);
  message.payloadBuffer[0] = testDevId;
  auto result = message.getDevIdFromBuffer();
  ASSERT_EQ(result, boost::none);
}