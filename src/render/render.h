/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_render_h_
#define __tge_render_h_

#include "core/core.h"
#include "win/win.h"

namespace tge {
  class Render
      : WindowHandler
      , Core {
  public:
    Render(bool vsync, bool triple_buffer);

  private:
    void on_resize(int32_t wigth, int32_t height) override;
    void on_mouse_motion(int32_t x, int32_t y) override;
    void on_mouse_up(int32_t button, int32_t x, int32_t y) override;
    void on_mouse_down(int32_t button, int32_t x, int32_t y) override;
    void on_render() override;
  };
} // namespace tge

#endif // __tge_render_h_
