/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::Buffer::Buffer(
    MemoryAllocator &alloc,
    const uint32_t size,
    const VmaMemoryUsage memory_usage,
    const VkBufferUsageFlags buffer_usage
)
    : allocator(alloc) {
  const VmaAllocationCreateInfo vma_allocation_info{.usage = memory_usage};

  const VkBufferCreateInfo buffer_create_info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = buffer_usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  if (VkResult res =
          vmaCreateBuffer(allocator, &buffer_create_info, &vma_allocation_info, &buf, &buf_mem, VK_NULL_HANDLE);
      res != VK_SUCCESS) {
    throw CoreException(std::format("Could not create buffer with size: {}", size), res);
  }
}

tge::Buffer::Buffer(Buffer &&other) noexcept
    : allocator(other.allocator)
    , buf(other.buf)
    , buf_mem(other.buf_mem) {
  other.buf = VK_NULL_HANDLE;
}

tge::Buffer::~Buffer() {
  if (buf) {
    vmaDestroyBuffer(allocator, buf, buf_mem);
  }
}
