#pragma once

#include "src/DataSource.h"
#include "src/DataVisualizer.h"
class App final {
  public:
    int run(int argc, char** argv);

  private:
    bool processOptions(int argc, char** argv);

    VisualizerConfiguration visualizerConfig;
    DataSourceConfiguration dataSourceConfig;
};
