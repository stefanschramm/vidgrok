#pragma once

#include "DataSource.h"
#include "DataVisualizer.h"
#include <cxxopts.hpp>

struct ProgramConfiguration {
  bool showHelp = false;
  VisualizerConfiguration visualizerConfig;
  DataSourceConfiguration dataSourceConfig;
};

namespace OptionProcessing {
std::optional<ProgramConfiguration> process(int argc, char** argv);
};
