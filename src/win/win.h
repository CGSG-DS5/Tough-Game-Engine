/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_win_h_
#define __tge_win_h_

#include "def.h"

namespace tge {
  class window_handler {
  private:
    SDL_Window *window {};
    int32_t error {};

    virtual void on_resize(int32_t wigth, int32_t height) = 0;
    virtual void on_mouse_motion(int32_t x, int32_t y) = 0;
    virtual void on_mouse_up(int32_t button, int32_t x, int32_t y) = 0;
    virtual void on_mouse_down(int32_t button, int32_t x, int32_t y) = 0;

  public:
    window_handler(const std::string &title, int32_t width, int32_t height);

    ~window_handler();

    SDL_Window * get_window_handle();

    void run();
  };
}

#endif  // __tge_win_h_
