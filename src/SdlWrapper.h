#include <SDL2/SDL.h>
#include <cstdint>

using Pixel = uint32_t;

enum class Event {
  NONE,
  QUIT,
};

class SdlWrapper {
public:
  SdlWrapper(int width, int height);
  ~SdlWrapper();

  Event pollEvent();
  void lockTexture(Pixel** pixels);
  void unlockTexture();
  void render();

private:
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
};
