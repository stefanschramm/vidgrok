// vidgrok - Visualize video data captured by a logic analyzer
//
// Copyright (C) 2024 Stefan Schramm <mail@stefanschramm.net>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "DataDispatcher.h"
#include "DataVisualizer.h"
#include "HardwareDataSource.h"
#include <cstdint>
#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char** argv) {
  using cxxopts::value;
  using std::to_string;

  VisualizerConfiguration visualizerConfig;
  DataSourceConfiguration dataSourceConfig;

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
  addOption("synced-rendering", "Render image only on vertical syncs", value<bool>());
  addOption("s,samplerate", "Sample rate in Hz", value<long unsigned int>()->default_value(to_string(dataSourceConfig.sampleRate)));
  addOption("d,driver", "libsigrok capturing driver to use. First encountered non-demo device is used by default.", value<std::string>()); // example: fx2lafw
  addOption("baudrates", "list available baudrates", value<bool>());
  addOption("h,help", "Print usage");

  try {
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      return 0;
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
    visualizerConfig.syncedRendering = result["synced-rendering"].as<bool>();

    dataSourceConfig.sampleRate = result["samplerate"].as<long unsigned int>();
    dataSourceConfig.driverName = result.count("driver") ? std::optional<std::string>(result["driver"].as<std::string>()) : std::optional<std::string>();
    dataSourceConfig.enabledChannels = std::set<uint8_t>({
      visualizerConfig.dataChannel,
      visualizerConfig.vSyncChannel,
      visualizerConfig.hSyncChannel,
    });
  } catch (std::exception& e) {
    std::cerr << "Error while processing arguments: " << e.what() << std::endl;
    return 1;
  }

  try {
    DataDispatcher dataDispatcher;
    HardwareDataSource dataSource(dataDispatcher, dataSourceConfig);
    DataVisualizer visualizer(dataDispatcher, visualizerConfig);

    // main loop of data source
    std::thread dataSourceThread(dataSource);

    // main loop
    visualizer();

    dataDispatcher.close();
    dataSourceThread.join();
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
