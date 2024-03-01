#include "HardwareDataSource.h"
#include "src/DataDispatcher.h"
#include <exception>
#include <iostream>
#include <ostream>
#include <stdexcept>

HardwareDataSource::HardwareDataSource(
  DataDispatcher& dataDispatcher,
  const DataSourceConfiguration& config
) : mDataDispatcher(dataDispatcher),
    mConfig(config),
    context(sigrok::Context::create()),
    device(getDevice(config.driverName)) {
  if (!device) {
    throw std::runtime_error(mConfig.driverName ? "Device not found." : "No device found.");
  }
  if (!isValidSampleRate(mConfig.sampleRate)) {
    throw std::runtime_error("Sample rate is not valid for this device.");
  }
}

void HardwareDataSource::operator()() {
  try {
    for (auto channelIndex : mConfig.enabledChannels) {
      device->channels().at(channelIndex)->set_enabled(true);
    }
    device->open();

    device->config_set(sigrok::ConfigKey::SAMPLERATE, Glib::Variant<guint64>::create(mConfig.sampleRate));

    session = context->create_session();
    session->add_device(device);
    session->add_datafeed_callback([this](std::shared_ptr<sigrok::Device> device, std::shared_ptr<sigrok::Packet> packet) {
      handlePacket(device, packet);
    });
    session->start();

    session->run(); // event loop, exited when session->stop() is called

    device->close();
  } catch (std::exception& e) {
    std::cerr << "Exception in data source thread: " << e.what() << std::endl;
  }
}

bool HardwareDataSource::isValidSampleRate(uint64_t sampleRate) const {
  auto gvarDict = device->config_list(sigrok::ConfigKey::SAMPLERATE);

  const uint64_t* elements = nullptr;
  gsize numElements;

  // TODO: Do some devices require querying "samplerate-steps" instead?
  GVariant* gvarList = g_variant_lookup_value(
    gvarDict.gobj(),
    "samplerates",
    G_VARIANT_TYPE("at")
  );
  if (gvarList) {
    elements = (const uint64_t*)g_variant_get_fixed_array(gvarList, &numElements, sizeof(uint64_t));
    for (gsize i = 0; i < numElements; i++) {
      if (elements[i] == sampleRate) {
        return true;
      }
    }
    g_variant_unref(gvarList);
  } else {
    std::runtime_error("Unable to query valid sample rates.");
  }

  return false;
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

void HardwareDataSource::handlePacket(
  [[maybe_unused]] std::shared_ptr<sigrok::Device> device,
  std::shared_ptr<sigrok::Packet> packet
) {
  if (packet->type()->id() != SR_DF_LOGIC) {
    return; // ignore non-logic packets
  }
  auto logic = std::dynamic_pointer_cast<sigrok::Logic>(packet->payload());
  if (logic->data_length() == 0) {
    throw std::runtime_error("Got packet with 0 samples.");
  }
  if (logic->unit_size() > sizeof(Sample)) {
    throw std::runtime_error("Size of received samples is bigger than expected.");
  }
  bool stopProducing = !mDataDispatcher.put(Samples((Sample*)logic->data_pointer(), logic->data_length()));
  if (stopProducing) {
    session->stop();
  }
}
