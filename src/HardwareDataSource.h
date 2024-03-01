#pragma once

#include "DataDispatcher.h"
#include <libsigrokcxx/libsigrokcxx.hpp>
#include <memory>

struct DataSourceConfiguration {
  long unsigned int sampleRate = 12000000;
  std::optional<std::string> driverName = std::optional<std::string>();
  std::set<int> enabledChannels = std::set<int>{0, 1, 2};
};

class HardwareDataSource {
public:
  HardwareDataSource(
    DataDispatcher& dataDispatcher,
    DataSourceConfiguration config
  );
  void operator()();

private:
  [[nodiscard]] std::shared_ptr<sigrok::HardwareDevice> getDevice(std::optional<std::string> driverName) const;
  [[nodiscard]] bool isValidSampleRate(long unsigned int sampleRate) const;
  void handlePacket(
    [[maybe_unused]] std::shared_ptr<sigrok::Device> device,
    std::shared_ptr<sigrok::Packet> packet
  );

  DataDispatcher& mDataDispatcher;
  const DataSourceConfiguration mConfig;

  std::shared_ptr<sigrok::Context> context = nullptr;
  std::shared_ptr<sigrok::Device> device = nullptr;
  std::shared_ptr<sigrok::Session> session = nullptr;
};
