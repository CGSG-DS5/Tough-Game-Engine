/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __render_pass_h_
#define __render_pass_h_

#include "vma_wrapper/image.h"

namespace tge {
  struct AttachmentsInfo {
    std::vector<vk::Format> color_attachments_formats{};
    std::optional<vk::Format> depth_attachment_format{};
  };

  class RenderPass {
  public:
    RenderPass(
        const MemoryAllocator& alloc,
        const vk::raii::Device& device,
        const AttachmentsInfo& info,
        uint32_t width,
        uint32_t height,
        uint32_t frames_in_flight
    );

    void begin(vk::CommandBuffer cmd_buf, uint32_t frame);
    void end(vk::CommandBuffer cmd_buf, uint32_t frame) const;
    void resize(uint32_t width, uint32_t height);
    void resize(vk::Extent2D screen_size);

  private:
    const MemoryAllocator& alloc;
    const vk::raii::Device& device;

    uint32_t frames_in_flight;

    AttachmentsInfo attachments_info{};

    std::vector<std::vector<vk::RenderingAttachmentInfo>> color_rendering_infos;
    std::vector<std::optional<vk::RenderingAttachmentInfo>> depth_rendering_info;

    std::vector<std::vector<Image>> color_attachments{};
    std::optional<std::vector<Image>> depth_attachment{};

    std::vector<vk::RenderingInfo> rendering_info{};

    void swap(RenderPass& other);
  };

  class RenderPassFactory {
  public:
    RenderPassFactory(const MemoryAllocator& alloc, const vk::raii::Device& device);

    RenderPass create_gbuffer_pass(uint32_t width, uint32_t height, uint32_t num_of_attachments, uint32_t frames_in_flight) const;
    RenderPass create_gbuffer_pass(vk::Extent2D screen_size, uint32_t num_of_attachments, uint32_t frames_in_flight) const;

  private:
    const MemoryAllocator& alloc;
    const vk::raii::Device& device;
  };
} // namespace tge

#endif // __render_pass_h_
