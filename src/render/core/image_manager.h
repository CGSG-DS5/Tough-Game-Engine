/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_image_manager_h_
#define __tge_image_manager_h_

#include "vma_wrapper/image.h"

namespace tge {
  enum struct ImageSamplerType : uint32_t {
    REPEAT = 0,
    REPEAT_MIPMAP = 1,
    CLAMP = 2,
    CLAMP_MIPMAP = 3,
  };

  class ImageManager {
  public:
    ImageManager(const MemoryAllocator& allocator, const vk::raii::Device& device, vk::PhysicalDevice physical_device);

    template<typename... Args>
    Image create_image(Args&&... args) const {
      return Image(allocator, device, std::forward<Args>(args)...);
    }

    Image create_texture() const;
    vk::Sampler sampler(ImageSamplerType type) const;

  private:
    const MemoryAllocator& allocator;
    const vk::raii::Device& device;

    std::map<ImageSamplerType, vk::raii::Sampler> samplers;

    std::map<ImageSamplerType, vk::raii::Sampler> create_samplers(vk::PhysicalDevice physical_device) const;
  };
} // namespace tge

#endif // __tge_image_manager_h_
