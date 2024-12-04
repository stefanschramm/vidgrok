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
  auto run() -> void override;

private:
  [[nodiscard]] auto getDevice(std::optional<std::string> driverName) -> std::shared_ptr<sigrok::HardwareDevice> const;

  std::shared_ptr<sigrok::Device> device = nullptr;
};
