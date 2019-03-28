#ifndef RfcTransport_H
#define RfcTransport_H
#define BOOST_ASIO_DISABLE_THREADS

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <vector>

#include "TcpServer.h"

namespace DeviceListener {

struct RfcMessage {
  struct Rfc1006Header {
    uint8_t version;
    uint8_t reserved;
    uint16_t length;
  } __attribute__((packed));

  struct PayloadHeader {
    uint16_t deviceId;
    uint16_t measurementTag;
    uint32_t timestamp;
    uint16_t measurementType;
    uint16_t dataLength;
  } __attribute__((packed));

  static const size_t kMaxPayloadLength = 4096;
  static const uint8_t kProtocolVersion = 0x0;

  Rfc1006Header headerBuffer;
  std::vector<uint8_t> payloadBuffer;

  /**
   * \brief checks if the header is valid and return the length of the payload
   * \param header RFC1006 header structure
   * \result: length of the payload if header is valid, boost::none if no
   */
  boost::optional<size_t> validateHeaderAndGetLength();

  /**
   * \brief checks if the buffer is valid and extracts device ID from payload
   * raw buffer
   * \param buffer reference to the raw payload buffer
   * \result device ID if buffer is valid, boost::none if no
   */
  boost::optional<uint16_t> getDevIdFromBuffer();
};

class RfcTransport {
 public:
  using MessagePtr = std::shared_ptr<RfcMessage>;
  explicit RfcTransport(boost::asio::io_service &ioservice)
      : ioService_(ioservice) {}

  /**
   * \brief read RFC1006 header asynchronously and schedule header and payload
   * processing if everything is fine
   * \param conHandle object representing a connection we work with
   */
  void startPacketAsyncRead(TcpServer::ConHandle conHandle);

 protected:
  boost::asio::io_service &ioService_;

  /**
   * \brief validate RFC1006 header and schedule receiving the packet payloadd
   * \param conHandle pointer to the Connection object we are working with
   * \param err error code of 'read' call
   * \param message pointer to the Message object for this connection
   * \param bytesTransfered - number of received bytes, must be equal to the
   * RFC1006 header length
   */
  void handleHeaderRead(TcpServer::ConHandle conHandle, MessagePtr msg,
                        boost::system::error_code const &err,
                        size_t bytesTransfered);
  /**
   * \brief validate payload structure and pass it to the data counter
   * \param conHandle pointer to the Connection object we are working with
   * \param err error code of 'read' call
   * \param message pointer to the Message object for this connection
   * \param bytesTransfered - number of received bytes, must be bigger than
   * payload header length
   */
  void handlePayloadRead(TcpServer::ConHandle conHandle, MessagePtr msg,
                         boost::system::error_code const &err,
                         size_t bytesTransfered);
  void performPayloadAsyncRead(size_t length, TcpServer::ConHandle conHandle,
                               MessagePtr msg);
};

}  // namespace DeviceListener

#endif
