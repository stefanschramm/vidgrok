#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <string>

using Pixel = uint32_t;

class SdlWrapper final {
public:
  SdlWrapper(int width, int height, const std::string& windowTitle);
  ~SdlWrapper();

  auto quitEventOccured() -> bool;
  auto lockTexture(Pixel** pixels) -> void;
  auto unlockTexture() -> void;
  auto render() -> void;

private:
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
};
