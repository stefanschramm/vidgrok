#include "DataSource.h"
#include "HardwareDataSource.h"
#include "RecordedSessionDataSource.h"
#include <cstdint>
#include <memory>

DataSource::DataSource(
  SampleDataDispatcher& dataDispatcher,
  const DataSourceConfiguration& config
) : mDataDispatcher(dataDispatcher),
    mConfig(config),
    context(sigrok::Context::create()) {
}

auto DataSource::create(
  SampleDataDispatcher& dataDispatcher,
  const DataSourceConfiguration& config
) -> std::unique_ptr<DataSource> {
  if (config.inputFile) {
    return std::make_unique<RecordedSessionDataSource>(dataDispatcher, config);
  } else {
    return std::make_unique<HardwareDataSource>(dataDispatcher, config);
  }
}

auto DataSource::getSampleRate() -> uint64_t {
  return sampleRate;
}

auto DataSource::handlePacket(
  [[maybe_unused]] std::shared_ptr<sigrok::Device> device,
  std::shared_ptr<sigrok::Packet> packet
) -> void {
  if (packet->type()->id() == SR_DF_END) {
    if (!mConfig.keepGoing) {
      mDataDispatcher.close();
    }
    return;
  }
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
  mDataDispatcher.put(Samples((Sample*)logic->data_pointer(), logic->data_length()));
  if (mDataDispatcher.isClosed()) {
    session->stop();
    session->remove_datafeed_callbacks();
  }
}
