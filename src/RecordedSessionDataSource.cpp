#include "RecordedSessionDataSource.h"
#include <iostream>
#include <libsigrokcxx/libsigrokcxx.hpp>
#include <stdexcept>

RecordedSessionDataSource::RecordedSessionDataSource(
  SampleDataDispatcher& dataDispatcher,
  const DataSourceConfiguration& config
) : DataSource(dataDispatcher, config) {
  if (!mConfig.inputFile) {
    throw std::runtime_error("No input file was passed.");
  }
  session = context->load_session(mConfig.inputFile.value());

  try {
    auto gvar = session->devices().at(0)->config_get(sigrok::ConfigKey::SAMPLERATE);
    sampleRate = Glib::VariantBase::cast_dynamic<Glib::Variant<guint64>>(gvar).get();
  } catch (sigrok::Error& error) {
    throw std::runtime_error("Unable to determine sample rate.");
  }
}

void RecordedSessionDataSource::run() {
  try {
    session->add_datafeed_callback([this](std::shared_ptr<sigrok::Device> device, std::shared_ptr<sigrok::Packet> packet) {
      handlePacket(device, packet);
    });

    while (!mDataDispatcher.isClosed()) {
      session->start();
      session->run(); // event loop, exited when session->stop() is called or end of data is reached
    }
  } catch (std::exception& e) {
    std::cerr << "Exception in data source thread: " << e.what() << std::endl;
  }
}
