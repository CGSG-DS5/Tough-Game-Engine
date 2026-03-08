/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_surface_h_
#define __tge_surface_h_

#include "core_exception.h"

namespace tge {
  class raii_surface {
  public:
    raii_surface(vk::Instance instance, SDL_Window *Window);

    raii_surface(const raii_surface &) = delete;
    raii_surface & operator=(const raii_surface &) = delete;

    raii_surface(raii_surface &&other) noexcept;

    ~raii_surface();

    operator vk::SurfaceKHR() const;

  private:
    vk::Instance instance;
    VkSurfaceKHR surface;
  };
}

#endif  // __tge_surface_h_
