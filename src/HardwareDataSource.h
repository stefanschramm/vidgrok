#pragma once

#include "DataSource.h"
#include <cstdint>
#include <libsigrokcxx/libsigrokcxx.hpp>
#include <memory>

class HardwareDataSource : public DataSource {
public:
  HardwareDataSource(
    SampleDataDispatcher& dataDispatcher,
    const DataSourceConfiguration& config
  );
  virtual void run();

private:
  [[nodiscard]] std::shared_ptr<sigrok::HardwareDevice> getDevice(std::optional<std::string> driverName) const;
  [[nodiscard]] bool isValidSampleRate(uint64_t sampleRate) const;

  std::shared_ptr<sigrok::Device> device = nullptr;
};
