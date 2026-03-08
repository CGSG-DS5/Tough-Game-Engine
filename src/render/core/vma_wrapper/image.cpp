/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::image::image(
  const vma_allocator &alloc,
  const vk::raii::Device &device,
  const vk::Image img,
  const vk::Format fmt
) : allocator(alloc), image_handle(img), image_view(create_image_view(device, fmt)) {
}

vk::raii::ImageView tge::image::create_image_view(
  const vk::raii::Device &device,
  const vk::Format format
) {
  return device.createImageView(
    vk::ImageViewCreateInfo(
      vk::ImageViewCreateFlags(),
      image_handle,
      vk::ImageViewType::e2D,
      format,
      vk::ComponentMapping(),
      vk::ImageSubresourceRange(
        vk::ImageAspectFlagBits::eColor,
        0, 1, 0, 1
      )
    )
  );
}

tge::image::image(image &&other) noexcept :
  allocator(other.allocator), image_handle(other.image_handle),
  image_view(std::move(other.image_view)), mem(other.mem) {
  other.image_handle = VK_NULL_HANDLE;
}

tge::image::~image() {
  if (mem) {
    vmaDestroyImage(allocator, image_handle, mem);
  }
}
