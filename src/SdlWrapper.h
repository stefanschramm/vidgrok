#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <string>

using Pixel = uint32_t;

class SdlWrapper {
public:
  SdlWrapper(int width, int height, const std::string& windowTitle);
  ~SdlWrapper();

  bool quitEventOccured();
  void lockTexture(Pixel** pixels);
  void unlockTexture();
  void render();

private:
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
};
