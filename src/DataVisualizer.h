#pragma once

#include "DataDispatcher.h"
#include <SDL.h>
#include <cstddef>
#include <cstdint>

// Default values should be OK for PAL video
struct VisualizerConfiguration {
  int width = 800;
  int height = 330;
  int dataChannel = 2;
  int verticalSyncChannel = 0;
  int horizontalSyncChannel = 1;
  bool invertData = false;
  bool invertVerticalSync = false;
  bool invertHorizontalSync = false;
  bool disableVerticalSync = false;
  bool disableHorizontalSync = false;
  bool hideVerticalSync = false;
  bool hideHorizontalSync = false;
  bool syncedRendering = false;
};

class DataVisualizer {
public:
  DataVisualizer(
    DataDispatcher* dataDispatcher,
    VisualizerConfiguration config
  );

  // Main loop
  void operator()();

private:
  void init();
  void processData(uint8_t* data, size_t length);
  inline void render();

  DataDispatcher* mDataDispatcher;
  VisualizerConfiguration mConfig;

  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
  long int position = 0;
  int dataChannelMask = 0;
  int verticalSyncChannelMask = 0;
  int horizontalSyncChannelMask = 0;
  uint8_t previousSample = 0;
};
