#include "SdlWrapper.h"
#include "SDL_video.h"
#include <stdexcept>

SdlWrapper::SdlWrapper(int width, int height, const std::string& windowTitle) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    throw std::runtime_error("Unable to initialize SDL" + std::string(SDL_GetError()));
  }

  window = SDL_CreateWindow(
    windowTitle.c_str(),
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    width,
    height,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
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
    width,
    height
  );
  if (texture == NULL) {
    throw std::runtime_error("Unable to create SDL texture");
  }
}

SdlWrapper::~SdlWrapper() {
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

bool SdlWrapper::quitEventOccured() {
  SDL_Event event;
  bool quit = false;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      quit = true;
    }
  }

  return quit;
}

void SdlWrapper::lockTexture(Pixel** pixels) {
  int pitch;
  SDL_LockTexture(texture, NULL, (void**)pixels, &pitch);
}

void SdlWrapper::unlockTexture() {
  SDL_UnlockTexture(texture);
}

void SdlWrapper::render() {
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}
