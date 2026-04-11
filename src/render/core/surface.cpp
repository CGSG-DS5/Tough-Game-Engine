/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::RaiiSurface::RaiiSurface(vk::Instance instance, SDL_Window *window)
    : instance(instance) {
  SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface);
}

tge::RaiiSurface::RaiiSurface(RaiiSurface &&other) noexcept
    : instance(other.instance)
    , surface(other.surface) {
  other.surface = VK_NULL_HANDLE;
}

tge::RaiiSurface::~RaiiSurface() {
  if (surface) {
    SDL_Vulkan_DestroySurface(instance, surface, nullptr);
  }
}

tge::RaiiSurface::operator vk::SurfaceKHR() const {
  return surface;
}
