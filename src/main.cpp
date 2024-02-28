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
#include "DataRetriever.h"
#include "DataVisualizer.h"
#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char** argv) {
  using std::to_string;
  using cxxopts::value;

  VisualizerConfiguration visualizerConfig;
  RetrieverConfiguration retrieverConfig;

  cxxopts::Options options("vidgrok", "Visualize video data captured by a logic analyzer");
  options.add_options()
    ("width", "Window width", value<int>()->default_value(to_string(visualizerConfig.width)))
    ("height", "Window height", value<int>()->default_value(to_string(visualizerConfig.height)))
    ("data", "Data channel number", value<int>()->default_value(to_string(visualizerConfig.dataChannel)))
    ("vsync", "Vertical sync channel number", value<int>()->default_value(to_string(visualizerConfig.verticalSyncChannel)))
    ("hsync", "Horizontal sync channel number", value<int>()->default_value(to_string(visualizerConfig.horizontalSyncChannel)))
    ("invert-data", "Invert data channel input", value<bool>())
    ("invert-vsync", "Invert vertical sync channel input", value<bool>())
    ("invert-hsync", "Invert horizontal sync channel input", value<bool>())
    ("no-vsync", "Disable vertical synchronisation", value<bool>())
    ("no-hsync", "Disable horizontal synchronisation", value<bool>())
    ("hide-vsync", "Don't draw vertical synchronisation", value<bool>())
    ("hide-hsync", "Don't draw horizontal synchronisation", value<bool>())
    ("synced-rendering", "Render image only on vertical syncs", value<bool>())
    ("s,samplerate", "Sample rate in Hz", value<long unsigned int>()->default_value(to_string(retrieverConfig.sampleRate)))
    ("d,driver", "libsigrok capturing driver to use. First encountered non-demo device is used by default.", value<std::string>()) // example: fx2lafw
    ("baudrates", "list available baudrates", value<bool>())
    ("h,help", "Print usage")
  ;
  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return 0;
  }
 
  visualizerConfig.width = result["width"].as<int>();
  visualizerConfig.height = result["height"].as<int>();
  visualizerConfig.dataChannel = result["data"].as<int>();
  visualizerConfig.verticalSyncChannel = result["vsync"].as<int>();
  visualizerConfig.horizontalSyncChannel = result["hsync"].as<int>();
  visualizerConfig.invertData = result["invert-data"].as<bool>();
  visualizerConfig.invertVerticalSync = result["invert-vsync"].as<bool>();
  visualizerConfig.invertHorizontalSync = result["invert-hsync"].as<bool>();
  visualizerConfig.disableVerticalSync = result["no-vsync"].as<bool>();
  visualizerConfig.disableHorizontalSync = result["no-hsync"].as<bool>();
  visualizerConfig.hideVerticalSync = result["hide-vsync"].as<bool>();
  visualizerConfig.hideHorizontalSync = result["hide-hsync"].as<bool>();
  visualizerConfig.syncedRendering = result["synced-rendering"].as<bool>();

  retrieverConfig.sampleRate = result["samplerate"].as<long unsigned int>();
  retrieverConfig.driverName = result.count("driver") ? std::optional<std::string>(result["driver"].as<std::string>()) : std::optional<std::string>();
  retrieverConfig.enabledChannels = std::set<int>({
    visualizerConfig.dataChannel,
    visualizerConfig.verticalSyncChannel,
    visualizerConfig.horizontalSyncChannel
  });

  DataDispatcher dataDispatcher;
  DataRetriever retriever(&dataDispatcher, retrieverConfig);
  DataVisualizer visualizer(&dataDispatcher, visualizerConfig);

  std::thread retrieverThread(retriever); // main loop of retriever in 2nd thread

  visualizer(); // main loop
  dataDispatcher.close();

  retrieverThread.join();

  return 0;
}
