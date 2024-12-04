#pragma once

#include "DataDispatcher.h"
#include "SdlWrapper.h"
#include <chrono>
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
  uint64_t sampleRate = 0; // not configurable via command line arguments
};

class DataVisualizer final {
public:
  DataVisualizer(
    SampleDataDispatcher& dataDispatcher,
    const VisualizerConfiguration& config
  );

  // Main loop: Fetches new samples (if available), processes/renders them and handles events.
  auto run() -> void;

private:
  inline auto process(Samples samples) -> void;
  [[nodiscard]] inline auto getPixelValue(bool vSyncActive, bool hSyncActive, Sample data) -> Pixel;
  inline auto render() -> void;

  SampleDataDispatcher& mDataDispatcher;
  const VisualizerConfiguration& mConfig;

  SdlWrapper sdlWrapper;

  const Sample vSyncChannelMask = 0;
  const Sample hSyncChannelMask = 0;
  const Sample dataRedChannelMask = 0;
  const Sample dataGreenChannelMask = 0;
  const Sample dataBlueChannelMask = 0;
  long int position = 0;
  long int samplesSinceLastRendering = 0;
  bool previousSampleVSyncActive = false;
  bool previousSampleHSyncActive = false;
  std::chrono::time_point<std::chrono::steady_clock> lastRenderedAt = std::chrono::steady_clock::now();

  const std::chrono::milliseconds MINIMAL_RENDER_PAUSE = std::chrono::milliseconds(20); // = 50 fps
};
