/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __render_pass_h_
#define __render_pass_h_

#include "descriptor_manager.h"
#include "image_manager.h"
#include "pipeline_manager.h"

namespace tge {
  class RenderPass {
  public:
    struct Attachments {
      std::vector<Image> color;
      std::optional<Image> depth;

      Attachments() = default;

      Attachments(Attachments&& other) noexcept = default;
      Attachments& operator=(Attachments&& other) noexcept = default;
    };

    RenderPass(
        vk::Device device,
        Attachments&& attachments,
        uint32_t width,
        uint32_t height,
        uint32_t set,
        uint32_t binding,
        vk::PipelineLayout layout,
        vk::DescriptorSet descriptor,
        vk::Sampler sampler
    );

    RenderPass(RenderPass&& other) noexcept;
    RenderPass& operator=(RenderPass&& other) noexcept;

    void begin(vk::CommandBuffer cmd_buf);
    void end(vk::CommandBuffer cmd_buf);

  private:
    Attachments attachments;

    std::vector<vk::RenderingAttachmentInfo> color_rendering_info;
    std::optional<vk::RenderingAttachmentInfo> depth_rendering_info;

    vk::RenderingInfo rendering_info;

    vk::PipelineLayout layout;
    vk::DescriptorSet descriptor;
    uint32_t set;
    uint32_t binding;
  };

  enum struct RenderPassType : uint32_t {
    OPAQUE,
    FINAL
  };

  class RenderPassManager {
  public:
    RenderPassManager(
        vk::Device device,
        const ImageManager& image_manager,
        const DescriptorManager& descriptor_manager,
        const PipelineManager& pipeline_manager,
        uint32_t frames_in_flight,
        vk::Extent2D screen_size
    );

    void resize(vk::Extent2D new_screen_size);

    void begin(vk::CommandBuffer cmd_buf, RenderPassType type, uint32_t frame_index);
    void end();

    const AttachmentsFormat& attachments_format(RenderPassType type) const;

  private:
    using Attachments = RenderPass::Attachments;

    vk::Device device;
    const ImageManager& image_manager;
    const DescriptorManager& descriptor_manager;
    const PipelineManager& pipeline_manager;
    uint32_t frames_in_flight;
    vk::Extent2D screen_size;

    std::map<RenderPassType, AttachmentsFormat> attachments_formats;
    std::map<RenderPassType, std::vector<RenderPass>> render_passes;

    vk::CommandBuffer current_cmd_buf;
    RenderPassType current_type;
    uint32_t current_frame_index;

    std::map<RenderPassType, AttachmentsFormat> create_formats() const;
    std::map<RenderPassType, std::vector<RenderPass>> create_render_passes() const;

    std::vector<RenderPass> create_gbuffer_pass(
        std::span<const vk::Format> formats,
        vk::PipelineLayout layout,
        std::span<const vk::DescriptorSet> descriptors,
        uint32_t set,
        uint32_t binding
    ) const;
  };
} // namespace tge

#endif // __render_pass_h_
