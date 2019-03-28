#ifndef MsgCounter_H
#define MsgCounter_H

#include <limits>
#include <string>
#include <unordered_map>
#include <utility>

namespace DeviceListener {

class MsgCounter {
 public:
  static MsgCounter &get() {
    static MsgCounter instance;
    return instance;
  }
  /**
   * \brief increments the specified device received messages counter
   * \param devId ID of the device to increment stats
   */
  void incrementCounter(uint16_t devId);
  /**
   * \brief prints current statistis to stdout in human-readable form
   */
  void printStatistics() const;
  /**
   * \brief parses "deviceID:deviceName" description file and save it to
   * thedeviceNames_ map
   * \param filename path to the devices descriptions file
   */
  void readDevicesFromFile(const std::string &filename);

  /**
   * \brief gathers human-readable device name by it ID
   * \param id device ID
   * \return human-readable device name if any, "Unknown device" if not found
   */
  const std::string &getDeviceNameById(uint16_t id) const;

 protected:
  static constexpr uint64_t kCounterMax = std::numeric_limits<uint64_t>::max();
  MsgCounter() = default;
  MsgCounter(MsgCounter const &) = delete;
  MsgCounter(MsgCounter &&) = delete;
  MsgCounter &operator=(MsgCounter const &) = delete;
  MsgCounter &operator=(MsgCounter &&) = delete;
  std::unordered_map<uint16_t, std::pair<uint64_t, bool>> stats_;
  std::unordered_map<uint16_t, std::string> deviceNames_;
};

}  // namespace DeviceListener

#endif