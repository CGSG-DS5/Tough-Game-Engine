/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_render_h_
#define __tge_render_h_

#include "core/core.h"
#include "win/win.h"

namespace tge {
  class render : window_handler, core {
  public:
    render(bool vsync, bool triple_buffer);

  private:
    void on_resize(int32_t wigth, int32_t height);
    void on_mouse_motion(int32_t x, int32_t y);
    void on_mouse_up(int32_t button, int32_t x, int32_t y);
    void on_mouse_down(int32_t button, int32_t x, int32_t y);
  };
}

#endif  // __tge_render_h_
