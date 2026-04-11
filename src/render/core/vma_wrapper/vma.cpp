/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#define VMA_IMPLEMENTATION
#include "tge.h"

#include <vma/vk_mem_alloc.h>

tge::MemoryAllocator::MemoryAllocator(
    const vk::Instance instance,
    const vk::PhysicalDevice physical_device,
    const vk::raii::Device &device
)
    : device(device) {
  const VmaAllocatorCreateInfo create_info = {
      .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = physical_device,
      .device = *device,
      .instance = instance
  };

  if (VkResult code = vmaCreateAllocator(&create_info, &allocator); code != VK_SUCCESS) {
    throw CoreException("Could not create memory allocator", code);
  }
}

tge::MemoryAllocator::~MemoryAllocator() {
  if (allocator) {
    vmaDestroyAllocator(allocator);
  }
}

tge::MemoryAllocator::MemoryAllocator(MemoryAllocator &&other) noexcept
    : allocator(other.allocator)
    , device(other.device) {
  other.allocator = VK_NULL_HANDLE;
}

tge::MemoryAllocator::operator VmaAllocator() const {
  return allocator;
}

tge::Image tge::MemoryAllocator::create_image(const vk::Image img, const vk::Format fmt) {
  return Image(*this, device, img, fmt);
}
