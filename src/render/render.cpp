/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::Render::Render(bool vsync, bool triple_buffer)
    : WindowHandler("Tough Game Engine", 800, 600)
    , Core(get_window_handle(), vsync, triple_buffer) {
  run();
}

void tge::Render::on_render() {
  Core::frame_start();

  Core::frame_end();
}

void tge::Render::on_resize(int32_t wigth, int32_t height) {
  Core::resize();
}

void tge::Render::on_mouse_motion(int32_t x, int32_t y) {}

void tge::Render::on_mouse_up(int32_t button, int32_t x, int32_t y) {}

void tge::Render::on_mouse_down(int32_t button, int32_t x, int32_t y) {}
