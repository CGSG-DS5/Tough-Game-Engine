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