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
        MemoryAllocator& alloc,
        uint32_t size,
        bool is_local,
        VkBufferUsageFlags buffer_usage
    );

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& other) noexcept;

    ~Buffer();

    void* get_mapped_data() const;

  private:
    MemoryAllocator& allocator;
    VkBuffer buf;
    VmaAllocation buf_mem;
    void* mapped_data{};
    bool is_local;
  };
} // namespace tge

#endif // __tge_buffer_h_
