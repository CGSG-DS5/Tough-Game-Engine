/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_buffer_h_
#define __tge_buffer_h_

#include "../core_exception.h"

namespace tge {
  /* Forward declaration */
  class MemoryAllocator;

  class Buffer {
  public:
    Buffer(
        MemoryAllocator &alloc,
        const uint32_t size,
        const VmaMemoryUsage memory_usage,
        const VkBufferUsageFlags buffer_usage
    );

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    Buffer(Buffer &&other) noexcept;

    ~Buffer();

  private:
    MemoryAllocator &allocator;
    VkBuffer buf;
    VmaAllocation buf_mem;
  };
} // namespace tge

#endif // __tge_buffer_h_
