#pragma once

#include "DataSource.h"
#include <libsigrokcxx/libsigrokcxx.hpp>
#include <memory>

class HardwareDataSource final : public DataSource {
public:
  HardwareDataSource(
    SampleDataDispatcher& dataDispatcher,
    const DataSourceConfiguration& config
  );
  virtual void run();

private:
  [[nodiscard]] std::shared_ptr<sigrok::HardwareDevice> getDevice(std::optional<std::string> driverName) const;

  std::shared_ptr<sigrok::Device> device = nullptr;
};
