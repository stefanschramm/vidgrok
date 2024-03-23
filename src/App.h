#pragma once

#include "DataSource.h"
#include "DataVisualizer.h"
class App final {
public:
  int run(int argc, char** argv);

private:
  bool processOptions(int argc, char** argv);

  VisualizerConfiguration visualizerConfig;
  DataSourceConfiguration dataSourceConfig;
};
