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
    Image(
        const MemoryAllocator& alloc,
        const vk::raii::Device& device,
        vk::Image img,
        vk::Format fmt,
        vk::ImageViewType type = vk::ImageViewType::e2D,
        vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor,
        uint32_t mip_count = 1,
        uint32_t layer_count = 1
    );

    Image(
        const MemoryAllocator& alloc,
        const vk::raii::Device& device,
        vk::Format fmt,
        vk::Extent3D extent,
        uint32_t mip_count,
        vk::SampleCountFlagBits sample_count,
        vk::ImageUsageFlags usage,
        bool is_cube
    );

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    ~Image();

    vk::Image get_image() const;
    vk::ImageView get_image_view() const;

    void switch_layout(vk::CommandBuffer cmd_buf, vk::ImageLayout new_layout);


  private:
    const MemoryAllocator& allocator;
    VmaAllocation mem{VK_NULL_HANDLE};
    vk::Image image_handle;
    vk::raii::ImageView image_view;
    vk::ImageLayout image_layout{vk::ImageLayout::eUndefined};
    vk::Format format;

    vk::raii::ImageView create_image_view(
        const vk::raii::Device& device,
        vk::Format fmt,
        vk::ImageViewType type,
        vk::ImageAspectFlags aspect,
        uint32_t mip_count,
        uint32_t layer_count
    );

    VkImage create_image(
        const MemoryAllocator& alloc,
        const vk::raii::Device& device,
        vk::Format fmt,
        vk::Extent3D extent,
        uint32_t mip_count,
        vk::SampleCountFlagBits sample_count,
        vk::ImageUsageFlags usage,
        bool is_cube
    );

    static std::pair<vk::PipelineStageFlags2, vk::AccessFlags2> get_stage_acess(const vk::ImageLayout layout);
  };
} // namespace tge

#endif // __tge_image_h_
