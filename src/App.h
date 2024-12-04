#pragma once

#include "DataSource.h"
#include "DataVisualizer.h"
class App final {
public:
  auto run(int argc, char** argv) -> int;

private:
  auto processOptions(int argc, char** argv) -> bool;

  VisualizerConfiguration visualizerConfig;
  DataSourceConfiguration dataSourceConfig;
};
