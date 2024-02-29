#include "DataVisualizer.h"
#include "SDL_pixels.h"
#include "SDL_video.h"
#include <SDL.h>
#include <cstdint>
#include <stdexcept>

DataVisualizer::DataVisualizer(
  DataDispatcher* dataDispatcher,
  VisualizerConfiguration config
) : mDataDispatcher(dataDispatcher),
    mConfig(config),
    dataChannelMask(1 << config.dataChannel),
    verticalSyncChannelMask(1 << config.verticalSyncChannel),
    horizontalSyncChannelMask(1 << config.horizontalSyncChannel) {

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    throw std::runtime_error("Unable to initialize SDL" + std::string(SDL_GetError()));
  }

  window = SDL_CreateWindow(
    "Signal Visualization",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    mConfig.width,
    mConfig.height,
    SDL_WINDOW_OPENGL
  );
  if (window == NULL) {
    throw std::runtime_error("Unable to create SDL window");
  }

  renderer = SDL_CreateRenderer(
    window,
    -1,
    SDL_RENDERER_ACCELERATED
  );
  if (renderer == NULL) {
    throw std::runtime_error("Unable to create SDL renderer");
  }

  texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_STREAMING,
    mConfig.width,
    mConfig.height
  );
  if (texture == NULL) {
    throw std::runtime_error("Unable to create SDL texture");
  }
}

void DataVisualizer::operator()() {
  while (true) {
    auto optionalData = mDataDispatcher->get();
    // TODO: mDataDispatcher->get() should wait with a small timeout until data is available.
    //       But timeout should be short enough for smooth input event processing.
    if (optionalData) {
      processData(
        optionalData.value().first,
        optionalData.value().second
      );
      mDataDispatcher->clear();
    }
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        break;
      }
    }
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void DataVisualizer::processData(uint8_t* data, size_t length) {
  void* pixels = nullptr;
  int pitch;
  SDL_LockTexture(texture, NULL, &pixels, &pitch);
  for (size_t i = 0; i < length; i++) {
    auto verticalSyncActive = mConfig.invertVerticalSync == (data[i] & verticalSyncChannelMask);
    auto horizontalSyncActive = mConfig.invertHorizontalSync == (data[i] & horizontalSyncChannelMask);
    auto verticalTriggered = !mConfig.disableVerticalSync && previousSampleVerticalSyncActive && !verticalSyncActive;
    auto horizontalTriggered = !mConfig.disableHorizontalSync && previousSampleHorizontalSyncActive && !horizontalSyncActive;
    if (horizontalTriggered) {
      position = position - (position % mConfig.width) + mConfig.width; // start of next line
    }
    if (verticalTriggered) {
      position = 0; // start of frame
      if (mConfig.syncedRendering) {
        SDL_UnlockTexture(texture);
        render();
        SDL_LockTexture(texture, NULL, &pixels, &pitch);
      }
    }
    if ((!horizontalSyncActive || !mConfig.hideHorizontalSync) && (!verticalSyncActive || !mConfig.hideVerticalSync)) {
      // Red
      auto valueVerticalSync = verticalSyncActive ? 0xff0000ff : 0x00000000;
      // Blue
      auto valueHorizontalSync = horizontalSyncActive ? 0x0000ffff : 0x00000000;
      // White
      auto valueData = ((bool)(data[i] & dataChannelMask) != mConfig.invertData) ? 0xffffffff : 0x00000000;
      ((uint32_t*)pixels)[position] = valueVerticalSync | valueHorizontalSync | valueData;
    } else {
      // Don't draw blanking areas (if requested so)
      ((uint32_t*)pixels)[position] = 0;
    }
    previousSampleHorizontalSyncActive = horizontalSyncActive;
    previousSampleVerticalSyncActive = verticalSyncActive;
    position++;
    if (position > mConfig.width * mConfig.height) {
      position = 0;
    }
  }
  SDL_UnlockTexture(texture);
  if (!mConfig.syncedRendering) {
    render();
  }
}

void DataVisualizer::render() {
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}
