#pragma once

#include "DataDispatcher.h"
#include "DataSource.h"

class RecordedSessionDataSource : public DataSource {
public:
  RecordedSessionDataSource(
    SampleDataDispatcher& dataDispatcher,
    const DataSourceConfiguration& config
  );
  virtual void run();
};
