/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

void tge::window_handler::on_resize(int32_t wigth, int32_t height) {
}

void tge::window_handler::on_mouse_motion(int32_t x, int32_t y) {
}

void tge::window_handler::on_mouse_up(int32_t button, int32_t x, int32_t y) {
}

void tge::window_handler::on_mouse_down(int32_t button, int32_t x, int32_t y) {
}

std::span<const char * const> tge::window_handler::get_sdl_extensions() {
  uint32_t sdl_extensions_count = 0;
  const char * const *sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);

  if (sdl_extensions == nullptr) {
    throw new core_exception(SDL_GetError(), 30);
  }

  return std::span<const char * const>(sdl_extensions, sdl_extensions_count);
}

tge::window_handler::window_handler(const std::string &title, int32_t width, int32_t height) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    error = 1;
    return;
  }

  window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

  if (!window) {
    printf("SDL_CreateWindow error: %s\n", SDL_GetError());
    SDL_Quit();
    error = 2;
    return;
  }
}

void tge::window_handler::run() {
  SDL_Event event;
  int8_t running = 1;

  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        running = 0;
        break;

      case SDL_EVENT_WINDOW_RESIZED:
      case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
      case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
        on_resize(event.window.data1, event.window.data2);
        break;

      case SDL_EVENT_MOUSE_MOTION:
        on_mouse_motion(event.motion.x, event.motion.y);
        break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        on_mouse_down(event.button.button, event.button.x, event.button.y);
        break;

      case SDL_EVENT_MOUSE_BUTTON_UP:
        on_mouse_up(event.button.button, event.button.x, event.button.y);
        break;

      case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_F11) {
          if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) {
            SDL_SetWindowFullscreen(window, false);
          } else {
            SDL_SetWindowFullscreen(window, true);
          }
        }
        break;
      }
    }

    // render()
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
}
