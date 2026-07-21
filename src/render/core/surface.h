/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_surface_h_
#define __tge_surface_h_

#include "core_exception.h"

namespace tge {
  class Surface {
  public:
    Surface(vk::Instance instance, SDL_Window* Window);

    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;

    Surface(Surface&& other) noexcept;

    ~Surface();

    operator vk::SurfaceKHR() const;

  private:
    vk::Instance instance;
    VkSurfaceKHR surface;
  };
} // namespace tge

#endif // __tge_surface_h_
