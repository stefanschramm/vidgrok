#include "DataVisualizer.h"
#include <chrono>
#include <thread>

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

auto DataVisualizer::run() -> void {
  while (true) {
    auto optionalData = mDataDispatcher.get(std::chrono::milliseconds(250));
    if (optionalData) {
      process(optionalData.value());
      mDataDispatcher.clear();
    }

    // When packets are coming in slowly, we render to refresh the window content, even if there is no new data
    if (!mConfig.renderSynced && std::chrono::steady_clock::now() >= lastRenderedAt + MINIMAL_RENDER_PAUSE) {
      render();
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

auto DataVisualizer::process(Samples samples) -> void {
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
        render();
        sdlWrapper.lockTexture(&pixels);
      }
    }

    pixels[position] = getPixelValue(vSyncActive, hSyncActive, sample);

    previousSampleHSyncActive = hSyncActive;
    previousSampleVSyncActive = vSyncActive;

    position++;
    samplesSinceLastRendering++;
    if (position > mConfig.width * mConfig.height) {
      position = 0;
    }
  }

  sdlWrapper.unlockTexture();
}

auto DataVisualizer::getPixelValue(bool vSyncActive, bool hSyncActive, Sample data) -> Pixel {
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

// Render data (the texture is expected to be locked!)
auto DataVisualizer::render() -> void {
  sdlWrapper.render();

  // Add pause to slow down visualization to match real time (important for recorded sessions)
  auto recordingDuration = std::chrono::nanoseconds(1000000000 * samplesSinceLastRendering / mConfig.sampleRate);
  auto now = std::chrono::steady_clock::now();
  auto renderingDuration = now - lastRenderedAt;
  auto additionalWaitTime = recordingDuration - renderingDuration;
  if (additionalWaitTime.count() > 0) {
    std::this_thread::sleep_for(additionalWaitTime);
  }
  lastRenderedAt = now;
  samplesSinceLastRendering = 0;
}
