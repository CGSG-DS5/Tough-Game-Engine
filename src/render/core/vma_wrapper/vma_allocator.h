/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_vma_allocator_h_
#define __tge_vma_allocator_h_

#include "def.h"

namespace tge {
  class vma_allocator {
  public:
    vma_allocator(
      const vk::Instance instance,
      const vk::PhysicalDevice physical_device,
      const vk::Device device
    );

    vma_allocator(const vma_allocator &) = delete;
    vma_allocator & operator=(const vma_allocator &) = delete;

    vma_allocator(vma_allocator &&other);

    ~vma_allocator();

  private:
    VmaAllocator allocator;
  };
}

#endif  // __tge_vma_allocator_h_
