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
  };
}

#endif  // __tge_render_h_
