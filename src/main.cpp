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
#include "OptionProcessing.h"
#include <cxxopts.hpp>
#include <exception>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
  ProgramConfiguration programConfig;

  try {
    auto optionalProgramConfiguration = OptionProcessing::process(argc, argv);
    if (!optionalProgramConfiguration) {
      return 0; // help has been displayed
    }
    programConfig = optionalProgramConfiguration.value();
  } catch (std::exception& e) {
    std::cerr << "Error while processing arguments: " << e.what() << std::endl;
    return 1;
  }

  try {
    DataDispatcher dataDispatcher;
    HardwareDataSource dataSource(dataDispatcher, programConfig.dataSourceConfig);
    DataVisualizer visualizer(dataDispatcher, programConfig.visualizerConfig);

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
