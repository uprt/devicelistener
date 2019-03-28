#define BOOST_ASIO_DISABLE_THREADS
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>

#include "RfcTransport.h"
#include "TcpServer.h"

using namespace DeviceListener;

Connection::~Connection() {
  std::cout << "Disconnected device: "
            << socket.remote_endpoint().address().to_string() << std::endl;
}

void TcpServer::handleAccept(ConHandle conHandle,
                             boost::system::error_code const &err) {
  if (!err) {
    std::cout << "Device connected: "
              << conHandle->socket.remote_endpoint().address().to_string()
              << std::endl;
    transport_.startPacketAsyncRead(conHandle);
  } else {
    std::cerr << "Error occured during 'accept' call: " << err.message()
              << std::endl;
  }
  startAccepting();
}

void TcpServer::listen() {
  auto endpoint =
      boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();
  std::cout << "Server is listening on port " << port_ << "..." << std::endl;
  startAccepting();
}

void TcpServer::startAccepting() {
  auto conHandle = std::make_shared<Connection>(ioService_);
  auto handler = boost::bind(&TcpServer::handleAccept, this, conHandle,
                             boost::asio::placeholders::error);
  acceptor_.async_accept(conHandle->socket, handler);
}