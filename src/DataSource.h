#pragma once

#include "DataDispatcher.h"
#include <cstdint>
#include <libsigrokcxx/libsigrokcxx.hpp>

struct DataSourceConfiguration {
  uint64_t sampleRate = 12000000;
  std::optional<std::string> driverName = std::optional<std::string>();
  std::set<uint8_t> enabledChannels = std::set<uint8_t>{0, 1, 2};
  std::optional<std::string> inputFile = std::optional<std::string>();
  bool keepGoing = false;
};

class DataSource {
public:
  virtual ~DataSource() = default;
  auto getSampleRate() -> uint64_t;
  virtual auto run() -> void = 0;
  // Create new DataSource based on configuration
  [[nodiscard]] static auto create(SampleDataDispatcher& dataDispatcher, const DataSourceConfiguration& config) -> std::unique_ptr<DataSource>;

protected:
  DataSource(
    SampleDataDispatcher& dataDispatcher,
    const DataSourceConfiguration& config
  );
  // Packet handling common for all DataSources
  auto handlePacket(
    [[maybe_unused]] std::shared_ptr<sigrok::Device> device,
    std::shared_ptr<sigrok::Packet> packet
  ) -> void;

  SampleDataDispatcher& mDataDispatcher;
  const DataSourceConfiguration& mConfig;

  uint64_t sampleRate = 0;

  std::shared_ptr<sigrok::Context> context = nullptr;
  std::shared_ptr<sigrok::Session> session = nullptr;
};
