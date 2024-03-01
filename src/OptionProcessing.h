#include "src/DataVisualizer.h"
#include "src/HardwareDataSource.h"
#include <cxxopts.hpp>

struct ProgramConfiguration {
  bool showHelp = false;
  VisualizerConfiguration visualizerConfig;
  DataSourceConfiguration dataSourceConfig;
};

namespace OptionProcessing {
std::optional<ProgramConfiguration> process(int argc, char** argv);
};
