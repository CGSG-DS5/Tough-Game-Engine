/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_surface_h_
#define __tge_surface_h_

#include "core_exception.h"

namespace tge {
  class RaiiSurface {
  public:
    RaiiSurface(vk::Instance instance, SDL_Window *Window);

    RaiiSurface(const RaiiSurface &) = delete;
    RaiiSurface &operator=(const RaiiSurface &) = delete;

    RaiiSurface(RaiiSurface &&other) noexcept;

    ~RaiiSurface();

    operator vk::SurfaceKHR() const;

  private:
    vk::Instance instance;
    VkSurfaceKHR surface;
  };
} // namespace tge

#endif // __tge_surface_h_
