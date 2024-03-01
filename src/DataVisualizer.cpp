#include "DataVisualizer.h"
#include "SDL_pixels.h"
#include "SDL_video.h"
#include <SDL.h>
#include <cstdint>
#include <stdexcept>

DataVisualizer::DataVisualizer(
  DataDispatcher& dataDispatcher,
  const VisualizerConfiguration& config
) : mDataDispatcher(dataDispatcher),
    mConfig(config),
    dataChannelMask(1 << config.dataChannel),
    vSyncChannelMask(1 << config.vSyncChannel),
    hSyncChannelMask(1 << config.hSyncChannel) {

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
    auto optionalData = mDataDispatcher.get(std::chrono::milliseconds(250));
    if (optionalData) {
      processData(optionalData.value());
      mDataDispatcher.clear();
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

void DataVisualizer::processData(Samples samples) {
  void* pixels = nullptr;
  int pitch;
  SDL_LockTexture(texture, NULL, &pixels, &pitch);
  for (auto& sample : samples) {
    bool vSyncActive = mConfig.invertVSync == (sample & vSyncChannelMask);
    bool hSyncActive = mConfig.invertHSync == (sample & hSyncChannelMask);
    bool verticalTriggered = !mConfig.disableVSync && previousSampleVSyncActive && !vSyncActive;
    bool horizontalTriggered = !mConfig.disableHSync && previousSampleHSyncActive && !hSyncActive;
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
    ((uint32_t*)pixels)[position] = getPixelValue(vSyncActive, hSyncActive, sample);
    previousSampleHSyncActive = hSyncActive;
    previousSampleVSyncActive = vSyncActive;
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

uint32_t DataVisualizer::getPixelValue(bool vSyncActive, bool hSyncActive, uint8_t data) {
  uint32_t value = 0;
  if (mConfig.highlightVSync && vSyncActive) {
    value |= 0x3f0000ff;
  }
  if (mConfig.highlightHSync && hSyncActive) {
    value |= 0x00003fff;
  }
  if ((!vSyncActive && !hSyncActive) || mConfig.renderHiddenData) {
    // TODO: support color by using 3 or 6 channels for data (CGA, EGA)
    value |= ((bool)(data & dataChannelMask) != mConfig.invertData) ? 0xffffffff : 0x00000000;
  }

  return value;
}
