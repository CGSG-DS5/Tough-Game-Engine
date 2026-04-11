/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::Image::Image(const MemoryAllocator &alloc, const vk::raii::Device &device, const vk::Image img, const vk::Format fmt)
    : allocator(alloc)
    , image_handle(img)
    , image_view(create_image_view(device, fmt)) {}

vk::raii::ImageView tge::Image::create_image_view(const vk::raii::Device &device, const vk::Format format) {
  return device.createImageView(
      vk::ImageViewCreateInfo(
          vk::ImageViewCreateFlags(),
          image_handle,
          vk::ImageViewType::e2D,
          format,
          vk::ComponentMapping(),
          vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
      )
  );
}

tge::Image::Image(Image &&other) noexcept
    : allocator(other.allocator)
    , image_handle(other.image_handle)
    , image_view(std::move(other.image_view))
    , mem(other.mem)
    , image_layout(other.image_layout) {
  other.mem = VK_NULL_HANDLE;
}

tge::Image::~Image() {
  if (mem) {
    vmaDestroyImage(allocator, image_handle, mem);
  }
}

vk::ImageView tge::Image::get_image_view() const {
  return image_view;
}

vk::Image tge::Image::get_image() const {
  return image_handle;
}

std::pair<vk::PipelineStageFlags2, vk::AccessFlags2> tge::Image::get_stage_acess(const vk::ImageLayout layout) {
  switch (layout) {
  case vk::ImageLayout::eUndefined:
  case vk::ImageLayout::eGeneral:
    return {vk::PipelineStageFlagBits2::eTopOfPipe, vk::AccessFlagBits2::eNone};
  case vk::ImageLayout::eColorAttachmentOptimal:
    return {vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite};
  case vk::ImageLayout::ePresentSrcKHR:
    return {vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eNone};
  }

  return {};
}

void tge::Image::switch_layout(const vk::CommandBuffer cmd_buf, const vk::ImageLayout new_layout) {
  if (image_layout == new_layout) {
    return;
  }

  auto [src_stage, src_acess] = get_stage_acess(image_layout);
  auto [dst_stage, dst_acess] = get_stage_acess(new_layout);

  vk::ImageMemoryBarrier2 image_memory_barrier(
      src_stage,
      src_acess,
      dst_stage,
      dst_acess,
      image_layout,
      new_layout,
      vk::QueueFamilyIgnored,
      vk::QueueFamilyIgnored,
      image_handle,
      vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
  );

  cmd_buf.pipelineBarrier2(vk::DependencyInfo(vk::DependencyFlags(), {}, {}, image_memory_barrier));

  image_layout = new_layout;
}
