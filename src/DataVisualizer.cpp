#include "DataVisualizer.h"
#include <chrono>

DataVisualizer::DataVisualizer(
  SampleDataDispatcher& dataDispatcher,
  const VisualizerConfiguration& config
) : mDataDispatcher(dataDispatcher),
    mConfig(config),
    sdlWrapper(mConfig.width, mConfig.height, "vidgrok"),
    vSyncChannelMask(1 << mConfig.vSyncChannel),
    hSyncChannelMask(1 << mConfig.hSyncChannel),
    dataRedChannelMask(1 << mConfig.dataRedChannel),
    dataGreenChannelMask(1 << mConfig.dataGreenChannel),
    dataBlueChannelMask(1 << mConfig.dataBlueChannel) {
}

void DataVisualizer::run() {
  auto minimalRenderPause = std::chrono::milliseconds(20); // = 50 fps
  auto lastRendering = std::chrono::system_clock::now() - minimalRenderPause;
  // TODO: When a recorded session is played back, the packets are much bigger than during normal capturing. Insert some wait somewhere to slow down playback to real (recorded) time.
  while (true) {
    auto optionalData = mDataDispatcher.get(std::chrono::milliseconds(250));
    if (optionalData) {
      process(optionalData.value());
      mDataDispatcher.clear();
    }

    if (!mConfig.renderSynced) {
      auto currentTime = std::chrono::system_clock::now();
      if (currentTime - lastRendering >= minimalRenderPause) {
        sdlWrapper.render();
        lastRendering = currentTime;
      }
    }

    if (mDataDispatcher.isClosed()) {
      break;
    }

    if (sdlWrapper.quitEventOccured()) {
      mDataDispatcher.close();
      break;
    }
  }
}

void DataVisualizer::process(Samples samples) {
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
      if (mConfig.renderSynced) {
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
    value |= ((bool)(data & dataRedChannelMask) != mConfig.invertData) ? 0xff0000ff : 0x00000000;
    value |= ((bool)(data & dataGreenChannelMask) != mConfig.invertData) ? 0x00ff00ff : 0x00000000;
    value |= ((bool)(data & dataBlueChannelMask) != mConfig.invertData) ? 0x0000ffff : 0x00000000;
  }

  return value;
}
