/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_buffer_h_
#define __tge_buffer_h_

#include "../core_exception.h"

namespace tge {
  /* Forward declaration */
  class vma_allocator;

  class buffer {
  public:
    buffer(
      vma_allocator &alloc,
      const uint32_t size,
      const VmaMemoryUsage memory_usage,
      const VkBufferUsageFlags buffer_usage
    );

    buffer(const buffer &) = delete;
    buffer & operator=(const buffer &) = delete;

    buffer(buffer &&other) noexcept;

    ~buffer();

  private:
    vma_allocator & const allocator;
    VkBuffer buf;
    VmaAllocation buf_mem;
  };
}

#endif  // __tge_buffer_h_
