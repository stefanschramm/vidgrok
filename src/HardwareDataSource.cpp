#include "HardwareDataSource.h"
#include "DataDispatcher.h"
#include "DataSource.h"
#include <exception>
#include <iostream>
#include <ostream>
#include <stdexcept>

HardwareDataSource::HardwareDataSource(
  SampleDataDispatcher& dataDispatcher,
  const DataSourceConfiguration& config
) : DataSource(dataDispatcher, config),
    device(getDevice(mConfig.driverName)) {
  if (!device) {
    throw std::runtime_error(mConfig.driverName ? "Device not found." : "No device found.");
  }
  for (auto channelIndex : mConfig.enabledChannels) {
    device->channels().at(channelIndex)->set_enabled(true);
  }
  device->open();

  try {
    device->config_set(sigrok::ConfigKey::SAMPLERATE, Glib::Variant<guint64>::create(mConfig.sampleRate));
  } catch (std::exception& e) {
    throw std::runtime_error("Unable to set sample rate. Use sigrok-cli --scan and sigrok-cli --show -d <drivername> to look up supported sample rates.");
  }
  sampleRate = mConfig.sampleRate;

  session = context->create_session();
  session->add_device(device);
}

void HardwareDataSource::run() {
  try {
    session->add_datafeed_callback([this](std::shared_ptr<sigrok::Device> device, std::shared_ptr<sigrok::Packet> packet) {
      handlePacket(device, packet);
    });

    while (!mDataDispatcher.isClosed()) {
      session->start();
      session->run(); // event loop, exited when session->stop() is called
    }

    device->close();
  } catch (std::exception& e) {
    std::cerr << "Exception in data source thread: " << e.what() << std::endl;
    mDataDispatcher.close();
  }
}

// Return either the first device with matching driverName or the first non-demo device.
std::shared_ptr<sigrok::HardwareDevice> HardwareDataSource::getDevice(std::optional<std::string> driverName) const {
  for (auto& [key, driver] : context->drivers()) {
    const auto keys = driver->config_keys();
    if (!keys.count(sigrok::ConfigKey::LOGIC_ANALYZER)) {
      continue;
    }
    if (driverName) {
      if (driver->name() != driverName.value()) {
        continue;
      }
    } else {
      if (driver->name() == "demo") {
        continue;
      }
    }
    std::map<const sigrok::ConfigKey*, Glib::VariantBase> drvopts;
    auto devices = driver->scan(drvopts);
    if (devices.size() < 1) {
      if (driverName) {
        return nullptr; // requested device not found
      } else {
        continue;
      }
    }

    return devices.at(0);
  }

  return nullptr;
}
