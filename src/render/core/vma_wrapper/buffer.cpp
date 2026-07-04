/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::Buffer::Buffer(MemoryAllocator& alloc, uint32_t size, bool is_local, VkBufferUsageFlags buffer_usage)
    : allocator(alloc)
    , is_local(is_local) {
  VmaAllocationCreateInfo vma_create_info{
      .usage = VMA_MEMORY_USAGE_AUTO,
  };

  auto local_flags = vk::MemoryPropertyFlagBits::eDeviceLocal;
  auto host_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
  if (is_local) {
    vma_create_info.requiredFlags = static_cast<uint32_t>(local_flags);
  } else {
    vma_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    vma_create_info.requiredFlags = static_cast<uint32_t>(host_flags);
    vma_create_info.preferredFlags = static_cast<uint32_t>(local_flags);
  }

  VkBufferCreateInfo buffer_create_info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = buffer_usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  VmaAllocationInfo vma_allocation_info;

  if (VkResult res =
          vmaCreateBuffer(allocator, &buffer_create_info, &vma_create_info, &buf, &buf_mem, &vma_allocation_info);
      res != VK_SUCCESS) {
    throw CoreException(std::format("Could not create buffer with size: {}", size), res);
  }

  mapped_data = vma_allocation_info.pMappedData;
}

tge::Buffer::Buffer(Buffer&& other) noexcept
    : allocator(other.allocator)
    , buf(other.buf)
    , buf_mem(other.buf_mem)
    , mapped_data(other.mapped_data)
    , is_local(other.is_local) {
  other.buf = VK_NULL_HANDLE;
}

tge::Buffer::~Buffer() {
  if (buf) {
    vmaDestroyBuffer(allocator, buf, buf_mem);
  }
}

void* tge::Buffer::get_mapped_data() const {
  return mapped_data;
}
