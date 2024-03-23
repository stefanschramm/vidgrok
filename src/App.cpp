#include "App.h"
#include "DataDispatcher.h"
#include "DataSource.h"
#include "DataVisualizer.h"
#include <cxxopts.hpp>
#include <exception>
#include <iostream>
#include <memory>
#include <thread>

int App::run(int argc, char** argv) {
  try {
    if (!processOptions(argc, argv)) {
      return 0; // help has been displayed
    }

    SampleDataDispatcher dataDispatcher;

    auto dataSource = DataSource::create(dataDispatcher, dataSourceConfig);

    visualizerConfig.sampleRate = dataSource->getSampleRate();

    DataVisualizer visualizer(dataDispatcher, visualizerConfig);

    std::thread dataSourceThread([&dataSource]() {
      dataSource->run(); // main loop of data source
    });

    visualizer.run(); // main loop
    dataDispatcher.close();
    dataSourceThread.join();

  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}

bool App::processOptions(int argc, char** argv) {
  using cxxopts::value;
  using std::to_string;

  cxxopts::Options options("vidgrok", "Visualize video data captured by a logic analyzer");

  auto addOption = options.add_options(); // not chaining it because of clang-format :/
  addOption("width", "Window width", value<int>()->default_value(to_string(visualizerConfig.width)));
  addOption("height", "Window height", value<int>()->default_value(to_string(visualizerConfig.height)));
  addOption("vsync", "Vertical sync channel number", value<uint8_t>()->default_value(to_string(visualizerConfig.vSyncChannel)));
  addOption("hsync", "Horizontal sync channel number", value<uint8_t>()->default_value(to_string(visualizerConfig.hSyncChannel)));
  addOption("data", "Data channel number(s). Either a single digit for monochrome or 3 digits for 8 bit RGB-color (e.g. --data 2 or --data 234) ", value<std::string>()->default_value(to_string(visualizerConfig.dataRedChannel)));
  addOption("invert-data", "Invert data channel input", value<bool>());
  addOption("invert-vsync", "Invert vertical sync channel input", value<bool>());
  addOption("invert-hsync", "Invert horizontal sync channel input", value<bool>());
  addOption("no-vsync", "Disable vertical synchronisation", value<bool>());
  addOption("no-hsync", "Disable horizontal synchronisation", value<bool>());
  addOption("highlight-vsync", "Visualize vertical synchronisation", value<bool>());
  addOption("highlight-hsync", "Visualize horizontal synchronisation", value<bool>());
  addOption("hidden-data", "Render (hidden) data in blanking areas", value<bool>());
  addOption("render-synced", "Render image only on vertical syncs", value<bool>());
  addOption("s,sample-rate", "Sample rate in Hz", value<uint64_t>()->default_value(to_string(dataSourceConfig.sampleRate)));
  addOption("d,driver", "libsigrok capturing driver to use. First encountered non-demo device is used by default.", value<std::string>()); // example: fx2lafw
  addOption("i,input-file", "Load recorded session (Pulseview/sigrok-cli) instead of using device directly", value<std::string>());
  addOption("k,keep-going", "Try to continue capturing even after device driver's session has ended. Will loop forever in combination with recorded sessions (--input-file).", value<bool>());
  addOption("h,help", "Print usage");

  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << std::endl;

    return false;
  }

  auto dataChannels = result["data"].as<std::string>();
  if (dataChannels.size() == 1) {
    // monochrome - same channel is mapped to all colors (default)
    visualizerConfig.dataRedChannel = stoi(dataChannels.substr(0, 1));
    visualizerConfig.dataGreenChannel = visualizerConfig.dataRedChannel;
    visualizerConfig.dataBlueChannel = visualizerConfig.dataRedChannel;
  } else if (dataChannels.size() == 3) {
    // individual 3-bit colors
    visualizerConfig.dataRedChannel = stoi(dataChannels.substr(0, 1));
    visualizerConfig.dataGreenChannel = stoi(dataChannels.substr(1, 1));
    visualizerConfig.dataBlueChannel = stoi(dataChannels.substr(2, 1));
  } else {
    throw std::runtime_error("Argument --data can only be a single digit (e.g. --data 2) or three digits (e.g. --data 234)");
  }

  visualizerConfig.width = result["width"].as<int>();
  visualizerConfig.height = result["height"].as<int>();
  visualizerConfig.vSyncChannel = result["vsync"].as<uint8_t>();
  visualizerConfig.hSyncChannel = result["hsync"].as<uint8_t>();
  visualizerConfig.invertData = result["invert-data"].as<bool>();
  visualizerConfig.invertVSync = result["invert-vsync"].as<bool>();
  visualizerConfig.invertHSync = result["invert-hsync"].as<bool>();
  visualizerConfig.disableVSync = result["no-vsync"].as<bool>();
  visualizerConfig.disableHSync = result["no-hsync"].as<bool>();
  visualizerConfig.highlightVSync = result["highlight-vsync"].as<bool>();
  visualizerConfig.highlightHSync = result["highlight-hsync"].as<bool>();
  visualizerConfig.renderHiddenData = result["hidden-data"].as<bool>();
  visualizerConfig.renderSynced = result["render-synced"].as<bool>();

  auto maxChannels = sizeof(Sample) * 8 - 1;
  if (visualizerConfig.dataRedChannel > maxChannels || visualizerConfig.dataGreenChannel > maxChannels || visualizerConfig.dataBlueChannel > maxChannels) {
    throw std::runtime_error(std::string("Maximum channel number (digit) to pass via --data is ") + std::to_string(maxChannels));
  }
  if (visualizerConfig.vSyncChannel > maxChannels) {
    throw std::runtime_error(std::string("Maximum value for --vsync is ") + std::to_string(maxChannels));
  }
  if (visualizerConfig.hSyncChannel > maxChannels) {
    throw std::runtime_error(std::string("Maximum value for --hsync is ") + std::to_string(maxChannels));
  }

  dataSourceConfig.sampleRate = result["sample-rate"].as<uint64_t>();
  dataSourceConfig.driverName = result.count("driver") ? std::optional<std::string>(result["driver"].as<std::string>()) : std::optional<std::string>();
  dataSourceConfig.inputFile = result.count("input-file") ? std::optional<std::string>(result["input-file"].as<std::string>()) : std::optional<std::string>();
  dataSourceConfig.keepGoing = result["keep-going"].as<bool>();
  dataSourceConfig.enabledChannels = std::set<uint8_t>({
    visualizerConfig.dataRedChannel,
    visualizerConfig.dataGreenChannel,
    visualizerConfig.dataBlueChannel,
    visualizerConfig.vSyncChannel,
    visualizerConfig.hSyncChannel,
  });

  return true;
}
