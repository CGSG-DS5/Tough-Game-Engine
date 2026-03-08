/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::raii_surface::raii_surface(vk::Instance instance, SDL_Window *window) :
  instance(instance) {
  SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface);
}

tge::raii_surface::raii_surface(raii_surface &&other) noexcept :
  instance(other.instance), surface(other.surface) {
  other.surface = VK_NULL_HANDLE;
}

tge::raii_surface::~raii_surface() {
  if (surface) {
    SDL_Vulkan_DestroySurface(instance, surface, nullptr);
  }
}

tge::raii_surface::operator vk::SurfaceKHR() const {
  return surface;
}
