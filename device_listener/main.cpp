#define BOOST_ASIO_DISABLE_THREADS
#include <getopt.h>
#include <unistd.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>

#include "MsgCounter.h"
#include "RfcTransport.h"
#include "TcpServer.h"

boost::asio::io_service ioService;

/**
 * \brief prints usage information to stdout
 */
void printUsage() {
  std::cout << "Available command line arguments:" << std::endl;
  std::cout << "-f <filename> - path to the file with devices descriptions"
            << std::endl;
  std::cout << "-p <port> - TCP port to run server on" << std::endl;
  std::cout
      << "-i <interval> - interval (in seconds) to print statistics to stdout"
      << std::endl;
}

/**
 * \brief parses app command line arguments
 * \return tuple of parsed params: { device file path, listening port,
 * statistics print interval }
 */
std::tuple<std::string, uint16_t, uint16_t> parseParams(int argc,
                                                        char *argv[]) {
  static struct option longOpts[] = {{"file", required_argument, NULL, 'f'},
                                     {"port", required_argument, NULL, 'p'},
                                     {"interval", required_argument, NULL, 'i'},
                                     {NULL, no_argument, NULL, 0}};

  static char const *optString = "?f:p:i:";
  int opt = 0;
  int longIndex = 0;

  uint16_t port = 5555;
  uint16_t interval = 5;
  std::string deviceFilePath = "./devices.conf";

  opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
  while (opt != -1) {
    switch (opt) {
      case '?':
        printUsage();
        exit(0);
        break;
      case 'f':
        if (optarg) {
          deviceFilePath = optarg;
        } else {
          std::cerr << "Incorrect device descrtiption file path in `-f`"
                    << std::endl;
        }
        break;
      case 'p':
        int parsedPort;
        if (optarg && (parsedPort = atoi(optarg))) {
          port = parsedPort;
        } else {
          std::cerr << "Incorrect device descrtiption file path in `-f`"
                    << std::endl;
        }
        break;
      case 'i':
        int parsedInterval;
        if (optarg && (parsedInterval = atoi(optarg))) {
          interval = parsedInterval;
        } else {
          std::cerr << "Incorrect statistics print interval in `-f`"
                    << std::endl;
        }
        break;
      default:
        break;
    }
    opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
  }
  return {deviceFilePath, port, interval};
}

/**
 * \brief invokes MsgCounters' printStatistics() and schedules the next timer
 * tick for the same
 */
void printStats(const boost::system::error_code &error,
                boost::asio::deadline_timer &timer, uint16_t interval) {
  if (!error) {
    DeviceListener::MsgCounter::get().printStatistics();
    timer.expires_from_now(boost::posix_time::seconds(interval));
    timer.async_wait(boost::bind(printStats, boost::asio::placeholders::error,
                                 boost::ref(timer), interval));
  }
}

int main(int argc, char **argv) {
  uint16_t port;
  uint16_t interval;
  std::string deviceFilePath;
  std::tie(deviceFilePath, port, interval) = parseParams(argc, argv);

  pid_t currentPid = getpid();
  std::cout << "Started DeviceListener with pid " << currentPid << std::endl;
  std::cout << "Add command line key '-?' if you want to see usage information"
            << std::endl;

  DeviceListener::MsgCounter::get().readDevicesFromFile(deviceFilePath);

  DeviceListener::RfcTransport transport(ioService);
  DeviceListener::TcpServer server(port, ioService, transport);

  boost::asio::deadline_timer timer(ioService);
  timer.expires_from_now(boost::posix_time::seconds(interval));
  timer.async_wait(boost::bind(printStats, boost::asio::placeholders::error,
                               boost::ref(timer), interval));

  try {
    server.listen();
    ioService.run();
  } catch (boost::system::system_error &e) {
    std::cerr << "\033[31mSomething went wrong: " << e.what() << "\033[0m"
              << std::endl;
    return 1;
  }

  return 0;
}