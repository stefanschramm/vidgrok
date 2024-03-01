#pragma once

#include "DataDispatcher.h"
#include <cstdint>
#include <libsigrokcxx/libsigrokcxx.hpp>
#include <memory>

struct DataSourceConfiguration {
  uint64_t sampleRate = 12000000;
  std::optional<std::string> driverName = std::optional<std::string>();
  std::set<uint8_t> enabledChannels = std::set<uint8_t>{0, 1, 2};
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
  [[nodiscard]] bool isValidSampleRate(uint64_t sampleRate) const;
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
