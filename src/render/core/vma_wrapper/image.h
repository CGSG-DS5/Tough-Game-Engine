/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_image_h_
#define __tge_image_h_

#include "def.h"

namespace tge {
  /* Forward declaration */
  class MemoryAllocator;

  class Image {
  public:
    Image(const MemoryAllocator &alloc, const vk::raii::Device &device, const vk::Image img, const vk::Format fmt);

    Image(const Image &) = delete;
    Image &operator=(const Image &) = delete;

    Image(Image &&other) noexcept;

    ~Image();

    vk::Image get_image() const;
    vk::ImageView get_image_view() const;

    void switch_layout(const vk::CommandBuffer cmd_buf, const vk::ImageLayout new_layout);

  private:
    const MemoryAllocator &allocator;
    vk::Image image_handle;
    vk::raii::ImageView image_view;
    VmaAllocation mem{VK_NULL_HANDLE};
    vk::ImageLayout image_layout{vk::ImageLayout::eUndefined};

    vk::raii::ImageView create_image_view(const vk::raii::Device &device, const vk::Format fmt);

    static std::pair<vk::PipelineStageFlags2, vk::AccessFlags2> get_stage_acess(const vk::ImageLayout layout);
  };
} // namespace tge

#endif // __tge_image_h_
