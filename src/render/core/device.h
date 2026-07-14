/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_device_
#define __tge_device_

#include "surface.h"

namespace tge {
  class Device {
  public:
    Device(vk::raii::PhysicalDevice physical_device, std::span<RaiiSurface> surfaces);

    operator const vk::raii::Device&() const;
    const vk::raii::Device& operator*() const;
    const vk::raii::Device* operator->() const;

  private:
    vk::raii::Device device;

    vk::raii::Device create_device(vk::raii::PhysicalDevice physical_device, std::span<RaiiSurface> surfaces) const;
  };
} // namespace tge

#endif // __tge_device_
