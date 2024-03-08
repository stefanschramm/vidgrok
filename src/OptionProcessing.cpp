#include "OptionProcessing.h"
#include <cxxopts.hpp>

std::optional<ProgramConfiguration> OptionProcessing::process(int argc, char** argv) {
  using cxxopts::value;
  using std::to_string;

  ProgramConfiguration programConfig;
  auto& visualizerConfig = programConfig.visualizerConfig;
  auto& dataSourceConfig = programConfig.dataSourceConfig;

  cxxopts::Options options("vidgrok", "Visualize video data captured by a logic analyzer");

  auto addOption = options.add_options(); // not chaining it because of clang-format :/
  addOption("width", "Window width", value<int>()->default_value(to_string(visualizerConfig.width)));
  addOption("height", "Window height", value<int>()->default_value(to_string(visualizerConfig.height)));
  addOption("data", "Data channel number", value<uint8_t>()->default_value(to_string(visualizerConfig.dataChannel)));
  addOption("vsync", "Vertical sync channel number", value<uint8_t>()->default_value(to_string(visualizerConfig.vSyncChannel)));
  addOption("hsync", "Horizontal sync channel number", value<uint8_t>()->default_value(to_string(visualizerConfig.hSyncChannel)));
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

    return std::optional<ProgramConfiguration>();
  }

  visualizerConfig.width = result["width"].as<int>();
  visualizerConfig.height = result["height"].as<int>();
  visualizerConfig.dataChannel = result["data"].as<uint8_t>();
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
  if (visualizerConfig.dataChannel > maxChannels) {
    throw std::runtime_error(std::string("Maximum value for --data is ") + std::to_string(maxChannels));
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
    visualizerConfig.dataChannel,
    visualizerConfig.vSyncChannel,
    visualizerConfig.hSyncChannel,
  });

  return programConfig;
}
