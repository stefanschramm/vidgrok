#include "DataVisualizer.h"
#include <SDL.h>

DataVisualizer::DataVisualizer(
  DataDispatcher& dataDispatcher,
  const VisualizerConfiguration& config
) : mDataDispatcher(dataDispatcher),
    mConfig(config),
    sdlWrapper(mConfig.width, mConfig.height),
    dataChannelMask(1 << mConfig.dataChannel),
    vSyncChannelMask(1 << mConfig.vSyncChannel),
    hSyncChannelMask(1 << mConfig.hSyncChannel) {
}

void DataVisualizer::operator()() {
  while (true) {
    auto optionalData = mDataDispatcher.get(std::chrono::milliseconds(250));
    if (optionalData) {
      processData(optionalData.value());
      mDataDispatcher.clear();
    }

    if (sdlWrapper.pollEvent() == Event::QUIT) {
      break;
    }
  }
}

void DataVisualizer::processData(Samples samples) {
  Pixel* pixels = nullptr;

  sdlWrapper.lockTexture(&pixels);

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
        sdlWrapper.unlockTexture();
        sdlWrapper.render();
        sdlWrapper.lockTexture(&pixels);
      }
    }

    pixels[position] = getPixelValue(vSyncActive, hSyncActive, sample);

    previousSampleHSyncActive = hSyncActive;
    previousSampleVSyncActive = vSyncActive;

    position++;
    if (position > mConfig.width * mConfig.height) {
      position = 0;
    }
  }

  sdlWrapper.unlockTexture();

  if (!mConfig.syncedRendering) {
    sdlWrapper.render();
  }
}

Pixel DataVisualizer::getPixelValue(bool vSyncActive, bool hSyncActive, Sample data) {
  Pixel value = 0;
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
