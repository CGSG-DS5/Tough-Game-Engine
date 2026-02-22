#include <string>

#include <SDL3/SDL.h>

namespace tge {
  class window_handler {
  private:
    SDL_Window *window {};
    int32_t error;

    void on_resize(int32_t wigth, int32_t height);
    void on_mouse_motion(int32_t x, int32_t y);
    void on_mouse_up(int32_t button, int32_t x, int32_t y);
    void on_mouse_down(int32_t button, int32_t x, int32_t y);

  public:
    window_handler(const std::string &title, int32_t width, int32_t height);

    void run();
  };
}
