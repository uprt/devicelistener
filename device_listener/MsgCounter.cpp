#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <vector>

#include "MsgCounter.h"

using namespace DeviceListener;

const std::string &MsgCounter::getDeviceNameById(uint16_t id) const {
  static const std::string kUknownDevice = std::string("Unknown device");
  auto deviceNameIt = deviceNames_.find(id);
  if (deviceNameIt == deviceNames_.end())
    return kUknownDevice;
  else
    return deviceNameIt->second;
}

void MsgCounter::incrementCounter(uint16_t devId) {
  if (stats_[devId].first < kCounterMax)
    stats_[devId].first++;
  else
    stats_[devId].second = true;
}

void MsgCounter::printStatistics() const {
  std::cout << "\033[32m-----------------------------------------------------"
            << std::endl;
  std::cout << "Current statistics of received messages from devices:"
            << std::endl;
  std::cout << "[device id] - [number of valid messages]" << std::endl;
  for (auto &i : stats_) {
    uint64_t counterValue;
    bool overflow;
    std::tie(counterValue, overflow) = i.second;
    std::cout << getDeviceNameById(i.first) << " - " << (overflow ? ">" : "")
              << counterValue << std::endl;
  }
  std::cout << "-----------------------------------------------------\033[0m"
            << std::endl;
}

void MsgCounter::readDevicesFromFile(const std::string &filename) {
  std::ifstream infile(filename);
  if (infile.is_open()) {
    std::string line;
    size_t lineNum = 0;
    while (std::getline(infile, line)) {
      lineNum++;
      std::vector<std::string> parts;
      boost::split(parts, line, boost::is_any_of(":"));
      if (parts.size() < 2)
        std::cerr << "Wrong device description format at line " << lineNum
                  << std::endl;
      else {
        try {
          uint16_t devId = std::stoi(parts[0]);
          std::string devName = parts[1];
          std::cout << "Added device '" << devName << "' with id = " << devId
                    << std::endl;
          deviceNames_[devId] = devName;
          stats_[devId] = std::make_pair(0, false);
        } catch (const std::invalid_argument &) {
          std::cerr << "Wrong device ID at line " << lineNum << std::endl;
        }
      }
    }
  } else
    std::cerr << "Failed to open device description file: " << filename
              << std::endl;
}