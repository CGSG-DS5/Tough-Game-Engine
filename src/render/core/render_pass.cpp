/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::RenderPass::RenderPass(
    const MemoryAllocator& alloc,
    const vk::raii::Device& device,
    const AttachmentsInfo& info,
    uint32_t width,
    uint32_t height,
    uint32_t frames_in_flight
)
    : alloc(alloc)
    , device(device)
    , attachments_info(info)
    , frames_in_flight(frames_in_flight) {
  color_rendering_infos.resize(frames_in_flight);
  depth_rendering_info.resize(frames_in_flight);
  color_attachments.resize(frames_in_flight);
  if (info.depth_attachment_format) {
    depth_attachment.emplace();
  }
  rendering_info.resize(frames_in_flight);

  vk::Extent3D extent{.width = width, .height = height, .depth = 1};
  for (const auto& caf : info.color_attachments_formats) {
    for (auto& cas : color_attachments) {
      cas.emplace_back(
          alloc,
          device,
          caf,
          extent,
          1,
          vk::SampleCountFlagBits::e1,
          vk::ImageUsageFlagBits::eColorAttachment |
              vk::ImageUsageFlagBits::eSampled, // vk::ImageUsageFlagBits::eTransferSrc
          false
      );
    }
  }

  for (uint32_t i = 0; i < frames_in_flight; i++) {
    auto& cri = color_rendering_infos[i];
    auto& cas = color_attachments[i];

    for (auto& ca : cas) {
      cri.push_back(
          {.imageView = ca.get_image_view(),
           .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
           .resolveMode = vk::ResolveModeFlagBits::eNone,
           .resolveImageLayout = vk::ImageLayout::eUndefined,
           .loadOp = vk::AttachmentLoadOp::eClear,
           .storeOp = vk::AttachmentStoreOp::eStore,
           .clearValue = {{.0f, .0f, .0f, .0f}}}
      );
    }
  }

  if (!depth_attachment) {
    return;
  } else {
    for (uint32_t i = 0; i < frames_in_flight; i++) {
      auto& dri = depth_rendering_info[i];

      depth_attachment->emplace_back(
          alloc,
          device,
          *info.depth_attachment_format,
          extent,
          1,
          vk::SampleCountFlagBits::e1,
          vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
          false
      );

      auto& da = depth_attachment.value()[i];

      dri = {
          .imageView = da.get_image_view(),
          .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
          .resolveMode = vk::ResolveModeFlagBits::eNone,
          .resolveImageLayout = vk::ImageLayout::eUndefined,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .clearValue = {{.0f, .0f, .0f, .0f}}
      };
    }
  }

  for (uint32_t i = 0; i < frames_in_flight; i++) {
    auto& ri = rendering_info[i];
    auto& cri = color_rendering_infos[i];
    auto& dri = depth_rendering_info[i];
    ri = {
        .renderArea = {.offset = {0, 0}, .extent = {width, height}},
        .layerCount = 1,
        .colorAttachmentCount = static_cast<uint32_t>(cri.size()),
        .pColorAttachments = cri.data(),
        .pDepthAttachment = dri ? &*dri : nullptr
    };
  }
}

void tge::RenderPass::swap(RenderPass& other) {
  color_rendering_infos.swap(other.color_rendering_infos);
  depth_rendering_info.swap(other.depth_rendering_info);
  color_attachments.swap(other.color_attachments);
  depth_attachment.swap(other.depth_attachment);
  rendering_info.swap(other.rendering_info);
  std::swap(attachments_info, other.attachments_info);
}

void tge::RenderPass::begin(vk::CommandBuffer cmd_buf, uint32_t frame) {
  for (auto& attachment : color_attachments[frame]) {
    attachment.switch_layout(cmd_buf, vk::ImageLayout::eColorAttachmentOptimal);
  }
  if (depth_attachment) {
    (*depth_attachment)[frame].switch_layout(cmd_buf, vk::ImageLayout::eDepthStencilAttachmentOptimal);
  }

  cmd_buf.beginRendering(rendering_info[frame]);

  cmd_buf.setViewport(
      0,
      vk::Viewport(
          0.f,
          0.f,
          static_cast<float>(rendering_info[frame].renderArea.extent.width),
          static_cast<float>(rendering_info[frame].renderArea.extent.height),
          0.f,
          1.f
      )
  );
  cmd_buf.setScissor(0, rendering_info[frame].renderArea);
}

void tge::RenderPass::end(vk::CommandBuffer cmd_buf, uint32_t frame) const {
  cmd_buf.endRendering();
}

void tge::RenderPass::resize(uint32_t width, uint32_t height) {
  RenderPass new_pass(alloc, device, attachments_info, width, height, frames_in_flight);
  swap(new_pass);
}

void tge::RenderPass::resize(vk::Extent2D screen_size) {
  resize(screen_size.width, screen_size.height);
}

tge::RenderPassFactory::RenderPassFactory(const MemoryAllocator& alloc, const vk::raii::Device& device)
    : alloc(alloc)
    , device(device) {}

tge::RenderPass tge::RenderPassFactory::create_gbuffer_pass(
    uint32_t width,
    uint32_t height,
    uint32_t num_of_attachments,
    uint32_t frames_in_flight
) const {
  AttachmentsInfo info{.color_attachments_formats = {}, .depth_attachment_format = vk::Format::eD32Sfloat};

  for (uint32_t i = 0; i < num_of_attachments; i++) {
    info.color_attachments_formats.push_back(vk::Format::eR32G32B32A32Sfloat);
  }

  return RenderPass(alloc, device, info, width, height, frames_in_flight);
}

tge::RenderPass tge::RenderPassFactory::create_gbuffer_pass(
    vk::Extent2D screen_size,
    uint32_t num_of_attachments,
    uint32_t frames_in_flight
) const {
  return create_gbuffer_pass(screen_size.width, screen_size.height, num_of_attachments, frames_in_flight);
}
