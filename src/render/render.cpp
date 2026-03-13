/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::render::render(bool vsync, bool triple_buffer) :
  window_handler("Tough Game Engine", 800, 600),
  core(get_window_handle(), vsync, triple_buffer) {
  run();
}

void tge::render::on_render() {
  core::frame_start();

  core::frame_end();
}

void tge::render::on_resize(int32_t wigth, int32_t height) {
  core::resize();
}

void tge::render::on_mouse_motion(int32_t x, int32_t y) {
}

void tge::render::on_mouse_up(int32_t button, int32_t x, int32_t y) {
}

void tge::render::on_mouse_down(int32_t button, int32_t x, int32_t y) {
}
