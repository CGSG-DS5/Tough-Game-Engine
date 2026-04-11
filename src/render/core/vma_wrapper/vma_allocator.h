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
        const vk::raii::Device &device
    );

    MemoryAllocator(const MemoryAllocator &) = delete;
    MemoryAllocator &operator=(const MemoryAllocator &) = delete;

    MemoryAllocator(MemoryAllocator &&other) noexcept;

    ~MemoryAllocator();

    operator VmaAllocator() const;

    Image create_image(const vk::Image img, const vk::Format fmt);

  private:
    const vk::raii::Device &device;
    VmaAllocator allocator;
  };
} // namespace tge

#endif // __tge_vma_allocator_h_
