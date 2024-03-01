#pragma once

#include "DataDispatcher.h"
#include <SDL.h>
#include <cstdint>

// Default values should be OK for PAL video
struct VisualizerConfiguration {
  int width = 800;
  int height = 330;
  uint8_t dataChannel = 2;
  uint8_t vSyncChannel = 0;
  uint8_t hSyncChannel = 1;
  bool invertData = false;
  bool invertVSync = false;
  bool invertHSync = false;
  bool disableVSync = false;
  bool disableHSync = false;
  bool highlightVSync = false;
  bool highlightHSync = false;
  bool renderHiddenData = false;
  bool syncedRendering = false;
};

class DataVisualizer {
public:
  DataVisualizer(
    DataDispatcher& dataDispatcher,
    VisualizerConfiguration config
  );

  // Main loop
  void operator()();

private:
  void processData(Samples samples);
  inline void render();
  [[nodiscard]] inline uint32_t getPixelValue(bool vSyncActive, bool hSyncActive, uint8_t data);

  DataDispatcher& mDataDispatcher;
  const VisualizerConfiguration mConfig;

  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
  const int dataChannelMask = 0;
  const int vSyncChannelMask = 0;
  const int hSyncChannelMask = 0;
  long int position = 0;
  uint8_t previousSample = 0;
  bool previousSampleVSyncActive = false;
  bool previousSampleHSyncActive = false;
};
