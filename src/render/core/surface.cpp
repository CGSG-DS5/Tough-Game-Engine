/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::Surface::Surface(vk::Instance instance, SDL_Window* window)
    : instance(instance) {
  SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface);
}

tge::Surface::Surface(Surface&& other) noexcept
    : instance(other.instance)
    , surface(other.surface) {
  other.surface = VK_NULL_HANDLE;
}

tge::Surface::~Surface() {
  if (surface) {
    SDL_Vulkan_DestroySurface(instance, surface, nullptr);
  }
}

tge::Surface::operator vk::SurfaceKHR() const {
  return surface;
}
