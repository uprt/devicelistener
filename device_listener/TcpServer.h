#ifndef TcpServer_H
#define TcpServer_H
#define BOOST_ASIO_DISABLE_THREADS
#include <boost/asio.hpp>

namespace DeviceListener {

class RfcTransport;

struct Connection {
  boost::asio::ip::tcp::socket socket;
  explicit Connection(boost::asio::io_service &io_service)
      : socket(io_service) {}
  Connection(const Connection &) = delete;
  Connection &operator=(Connection const &) = delete;
  ~Connection();
};

class TcpServer {
 private:
  boost::asio::io_service &ioService_;
  boost::asio::ip::tcp::acceptor acceptor_;
  RfcTransport &transport_;
  uint16_t port_;

 public:
  using ConHandle = std::shared_ptr<Connection>;
  explicit TcpServer(uint16_t port, boost::asio::io_service &ioservice,
                     RfcTransport &transport)
      : ioService_(ioservice),
        acceptor_(ioservice),
        transport_(transport),
        port_(port) {}
  /**
   * \brief starts receiving of the packets and schedules accepting next
   * connection
   * \param err - error code of 'accept' call
   */
  void handleAccept(ConHandle conHandle, boost::system::error_code const &err);

  /**
   * opens socket, bind TCP port for it and call startAccepting()
   */
  void listen();

  /**
   * \brief schedules asynchronous accepting of new connection
   */
  void startAccepting();
};

}  // namespace DeviceListener

#endif