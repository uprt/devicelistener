#define BOOST_ASIO_DISABLE_THREADS

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>

#include "MsgCounter.h"
#include "RfcTransport.h"

using namespace DeviceListener;

boost::optional<uint16_t> RfcMessage::getDevIdFromBuffer() {
  PayloadHeader header;

  // I'm only gathering deviceID from the header
  // No validation for other fields, because our job is only to count messages

  if (payloadBuffer.size() < sizeof(header)) return boost::none;

  // reinterpret_cast will cause UB due to breaking aliasing rules,
  // using union type will also break "active member" rule,
  // so let's use std::memcpy. Modern compilers should be able to optimize it.
  std::memcpy(&header, &payloadBuffer[0], sizeof(header));

  return header.deviceId;
}

boost::optional<size_t> RfcMessage::validateHeaderAndGetLength() {
  if (headerBuffer.version != kProtocolVersion) return boost::none;
  if ((headerBuffer.length > kMaxPayloadLength) ||
      (headerBuffer.length < sizeof(PayloadHeader)))
    return boost::none;

  return static_cast<size_t>(headerBuffer.length);
}

void RfcTransport::handleHeaderRead(TcpServer::ConHandle conHandle,
                                    MessagePtr msg,
                                    boost::system::error_code const &err,
                                    size_t bytesTransfered) {
  if (bytesTransfered > 0) {
    auto validationResult = msg->validateHeaderAndGetLength();
    if (validationResult.is_initialized()) {
      performPayloadAsyncRead(validationResult.get(), conHandle, msg);
    } else {
      std::cerr << "Error occured: invalid header" << std::endl;
    }
  }

  if (err) {
    std::cerr << "Error occured during 'read' call: " << err.message()
              << std::endl;
  }
}

void RfcTransport::handlePayloadRead(TcpServer::ConHandle conHandle,
                                     MessagePtr msg,
                                     boost::system::error_code const &err,
                                     size_t bytesTransfered) {
  if (bytesTransfered > 0) {
    auto devId = msg->getDevIdFromBuffer();
    if (devId.is_initialized()) MsgCounter::get().incrementCounter(devId.get());
  }

  if (!err) {
    startPacketAsyncRead(conHandle);
  } else {
    std::cerr << "Error occured during 'read' call: " << err.message()
              << std::endl;
  }
}

void RfcTransport::performPayloadAsyncRead(size_t length,
                                           TcpServer::ConHandle conHandle,
                                           MessagePtr msg) {
  auto handler = boost::bind(&RfcTransport::handlePayloadRead, this, conHandle,
                             msg, boost::asio::placeholders::error,
                             boost::asio::placeholders::bytes_transferred);
  msg->payloadBuffer.resize(length);
  boost::asio::async_read(conHandle->socket,
                          boost::asio::buffer(msg->payloadBuffer, length),
                          boost::asio::transfer_exactly(length), handler);
}

void RfcTransport::startPacketAsyncRead(TcpServer::ConHandle conHandle) {
  auto msg = std::make_shared<RfcMessage>();
  // This can be optimized, actually: don't create new RfcMessage every time,
  // just pass it like ConHandle through the chain of callbacks
  auto handler = boost::bind(&RfcTransport::handleHeaderRead, this, conHandle,
                             msg, boost::asio::placeholders::error,
                             boost::asio::placeholders::bytes_transferred);
  boost::asio::async_read(
      conHandle->socket,
      boost::asio::buffer(&msg->headerBuffer,
                          sizeof(RfcMessage::Rfc1006Header)),
      boost::asio::transfer_exactly(sizeof(RfcMessage::Rfc1006Header)),
      handler);
}
