/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

tge::WindowHandler::WindowHandler(const std::string &title, int32_t width, int32_t height) {
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

SDL_Window *tge::WindowHandler::get_window_handle() {
  return window;
}

void tge::WindowHandler::run() {
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
        is_fullscreen = false;
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
          is_fullscreen = true;
        }
        break;
      }
    }

    if (!is_fullscreen) {
      on_render();
    }
  }
}

tge::WindowHandler::~WindowHandler() {
  SDL_DestroyWindow(window);
  SDL_Quit();
}
