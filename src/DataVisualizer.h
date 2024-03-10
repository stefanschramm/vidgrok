#pragma once

#include "DataDispatcher.h"
#include "SdlWrapper.h"
#include <cstdint>

// Default values should be OK for PAL video
struct VisualizerConfiguration {
  int width = 800;
  int height = 330;
  uint8_t vSyncChannel = 0;
  uint8_t hSyncChannel = 1;
  uint8_t dataRedChannel = 2;
  uint8_t dataGreenChannel = 2;
  uint8_t dataBlueChannel = 2;
  bool invertData = false;
  bool invertVSync = false;
  bool invertHSync = false;
  bool disableVSync = false;
  bool disableHSync = false;
  bool highlightVSync = false;
  bool highlightHSync = false;
  bool renderHiddenData = false;
  bool renderSynced = false;
};

class DataVisualizer {
public:
  DataVisualizer(
    SampleDataDispatcher& dataDispatcher,
    const VisualizerConfiguration& config
  );

  // Main loop: Fetches new samples (if available), processes/renders them and handles events.
  void run();

private:
  inline void process(Samples samples);
  [[nodiscard]] inline Pixel getPixelValue(bool vSyncActive, bool hSyncActive, Sample data);

  SampleDataDispatcher& mDataDispatcher;
  const VisualizerConfiguration& mConfig;

  SdlWrapper sdlWrapper;

  const Sample vSyncChannelMask = 0;
  const Sample hSyncChannelMask = 0;
  const Sample dataRedChannelMask = 0;
  const Sample dataGreenChannelMask = 0;
  const Sample dataBlueChannelMask = 0;
  long int position = 0;
  bool previousSampleVSyncActive = false;
  bool previousSampleHSyncActive = false;
};
