/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "tge.h"

tge::vma_allocator::vma_allocator(
      const vk::Instance instance,
      const vk::PhysicalDevice physical_device,
      const vk::raii::Device &device
) : device(device) {
  const VmaAllocatorCreateInfo create_info = {
    .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
    .physicalDevice = physical_device,
    .device = *device,
    .instance = instance
  };

  if (VkResult code = vmaCreateAllocator(&create_info, &allocator); code != VK_SUCCESS) {
    throw core_exception("Could not create memory allocator", code);
  }
}

tge::vma_allocator::~vma_allocator() {
  if (allocator) {
    vmaDestroyAllocator(allocator);
  }
}

tge::vma_allocator::vma_allocator(vma_allocator &&other) noexcept
  : allocator(other.allocator), device(other.device) {
  other.allocator = VK_NULL_HANDLE;
}

tge::vma_allocator::operator VmaAllocator() {
  return allocator;
}

tge::image tge::vma_allocator::create_image(const vk::Image img, const vk::Format fmt) {
  return image(*this, device, img, fmt);
}
