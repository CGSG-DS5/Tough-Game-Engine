/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_vma_allocator_h_
#define __tge_vma_allocator_h_

#include "buffer.h"
#include "image.h"

namespace tge {
  class MemoryAllocator {
  public:
    MemoryAllocator(
        const vk::Instance instance,
        const vk::PhysicalDevice physical_device,
        const vk::raii::Device& device
    );

    MemoryAllocator(const MemoryAllocator&) = delete;
    MemoryAllocator& operator=(const MemoryAllocator&) = delete;

    MemoryAllocator(MemoryAllocator&& other) noexcept;

    ~MemoryAllocator();

    operator VmaAllocator() const;

    template<typename... Args>
    Image create_image(Args&&... args) const {
      return Image(*this, device, std::forward<Args>(args)...);
    }

    template<typename... Args>
    Buffer create_buffer(Args&&... args) const {
      return Buffer(*this, std::forward<Args>(args)...);
    }

  private:
    const vk::raii::Device& device;
    VmaAllocator allocator;
  };
} // namespace tge

#endif // __tge_vma_allocator_h_
