/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::RenderPass::RenderPass(
    vk::Device device,
    Attachments&& _attachments,
    uint32_t width,
    uint32_t height,
    uint32_t set,
    uint32_t binding,
    vk::PipelineLayout layout,
    vk::DescriptorSet descriptor,
    vk::Sampler sampler
)
    : attachments(std::move(_attachments))
    , layout(layout)
    , descriptor(descriptor)
    , set(set)
    , binding(binding) {
  auto& cri = color_rendering_info;
  for (auto& ca : attachments.color) {
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

  vk::ClearValue clear_value;
  clear_value.depthStencil.depth = 1.f;
  clear_value.depthStencil.stencil = 0.f;

  if (attachments.depth) {
    depth_rendering_info = {
        .imageView = attachments.depth->get_image_view(),
        .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
        .resolveMode = vk::ResolveModeFlagBits::eNone,
        .resolveImageLayout = vk::ImageLayout::eUndefined,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clear_value
    };
  }

  rendering_info = {
      .renderArea = {.offset = {0, 0}, .extent = {width, height}},
      .layerCount = 1,
      .colorAttachmentCount = static_cast<uint32_t>(color_rendering_info.size()),
      .pColorAttachments = color_rendering_info.data(),
      .pDepthAttachment = depth_rendering_info ? &*depth_rendering_info : nullptr
  };

  std::vector<vk::DescriptorImageInfo> images_info;
  for (uint32_t i = 0; i < attachments.color.size(); i++) {
    images_info.push_back(
        {.sampler = sampler,
         .imageView = attachments.color[i].get_image_view(),
         .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal}
    );
  }

  vk::WriteDescriptorSet write_ds{
      .dstSet = descriptor,
      .dstBinding = binding,
      .dstArrayElement = 0,
      .descriptorCount = static_cast<uint32_t>(images_info.size()),
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .pImageInfo = images_info.data()
  };
  device.updateDescriptorSets(write_ds, nullptr);
}

tge::RenderPass::RenderPass(RenderPass&& other) noexcept
    : attachments(std::move(other.attachments))
    , color_rendering_info(std::move(other.color_rendering_info))
    , depth_rendering_info(std::move(other.depth_rendering_info))
    , rendering_info(other.rendering_info)
    , layout(other.layout)
    , descriptor(other.descriptor)
    , set(other.set)
    , binding(other.binding) {
  rendering_info.setColorAttachments(color_rendering_info);
  rendering_info.pDepthAttachment = depth_rendering_info ? &(*depth_rendering_info) : nullptr;
}

tge::RenderPass& tge::RenderPass::operator=(RenderPass&& other) noexcept {
  if (this != &other) {
    attachments = std::move(other.attachments);
    color_rendering_info = std::move(other.color_rendering_info);
    depth_rendering_info = std::move(other.depth_rendering_info);
    rendering_info = other.rendering_info;
    layout = other.layout;
    descriptor = other.descriptor;
    set = other.set;
    binding = other.binding;

    rendering_info.setColorAttachments(color_rendering_info);
    rendering_info.pDepthAttachment = depth_rendering_info ? &(*depth_rendering_info) : nullptr;
  }

  return *this;
}

void tge::RenderPass::begin(vk::CommandBuffer cmd_buf) {
  for (auto& attachment : attachments.color) {
    attachment.switch_layout(cmd_buf, vk::ImageLayout::eColorAttachmentOptimal);
  }
  if (attachments.depth) {
    attachments.depth->switch_layout(cmd_buf, vk::ImageLayout::eDepthStencilAttachmentOptimal);
  }

  cmd_buf.beginRendering(rendering_info);

  cmd_buf.setViewport(
      0,
      vk::Viewport(
          0.f,
          0.f,
          static_cast<float>(rendering_info.renderArea.extent.width),
          static_cast<float>(rendering_info.renderArea.extent.height),
          0.f,
          1.f
      )
  );
  cmd_buf.setScissor(0, rendering_info.renderArea);
}

void tge::RenderPass::end(vk::CommandBuffer cmd_buf) {
  cmd_buf.endRendering();

  for (auto& attachment : attachments.color) {
    attachment.switch_layout(cmd_buf, vk::ImageLayout::eShaderReadOnlyOptimal);
  }
  if (attachments.depth) {
    attachments.depth->switch_layout(cmd_buf, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
  }

  cmd_buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, set, descriptor, nullptr);
}

tge::RenderPassManager::RenderPassManager(
    vk::Device device,
    const ImageManager& image_manager,
    const DescriptorManager& descriptor_manager,
    const PipelineManager& pipeline_manager,
    uint32_t frames_in_flight,
    vk::Extent2D screen_size
)
    : device(device)
    , image_manager(image_manager)
    , descriptor_manager(descriptor_manager)
    , pipeline_manager(pipeline_manager)
    , frames_in_flight(frames_in_flight)
    , screen_size(screen_size)
    , attachments_formats(create_formats())
    , render_passes(create_render_passes()) {}

std::vector<tge::RenderPass> tge::RenderPassManager::create_gbuffer_pass(
    std::span<const vk::Format> formats,
    vk::PipelineLayout layout,
    std::span<const vk::DescriptorSet> descriptors,
    uint32_t set,
    uint32_t binding
) const {
  std::vector<RenderPass> passes;
  passes.reserve(frames_in_flight);
  for (uint32_t i = 0; i < frames_in_flight; i++) {
    Attachments attachments;
    attachments.color.reserve(formats.size());
    for (auto fmt : formats) {
      attachments.color.emplace_back(image_manager.create_color_attachment(screen_size, fmt));
    }
    attachments.depth = image_manager.create_depth_attachment(screen_size);

    RenderPass pass(
        device,
        std::move(attachments),
        screen_size.width,
        screen_size.height,
        set,
        binding,
        layout,
        descriptors[i],
        image_manager.sampler(ImageSamplerType::REPEAT)
    );
    passes.emplace_back(std::move(pass));
  }

  return passes;
}

void tge::RenderPassManager::resize(vk::Extent2D new_screen_size) {
  screen_size = new_screen_size;
  render_passes = create_render_passes();
}

void tge::RenderPassManager::begin(vk::CommandBuffer cmd_buf, RenderPassType type, uint32_t frame_index) {
  current_cmd_buf = cmd_buf;
  current_type = type;
  current_frame_index = frame_index;

  render_passes[current_type][current_frame_index].begin(current_cmd_buf);
}

void tge::RenderPassManager::end() {
  render_passes[current_type][current_frame_index].end(current_cmd_buf);
}

const tge::AttachmentsFormat& tge::RenderPassManager::attachments_format(RenderPassType type) const {
  return attachments_formats.at(type);
}

std::map<tge::RenderPassType, tge::AttachmentsFormat> tge::RenderPassManager::create_formats() const {
  AttachmentsFormat final{.color = {vk::Format::eB8G8R8A8Unorm}};
  AttachmentsFormat opaque{.color = {vk::Format::eB8G8R8A8Unorm}, .depth = vk::Format::eD32Sfloat};
  std::map<RenderPassType, AttachmentsFormat> result{{RenderPassType::OPAQUE, opaque}, {RenderPassType::FINAL, final}};
  return result;
}

std::map<tge::RenderPassType, std::vector<tge::RenderPass>> tge::RenderPassManager::create_render_passes() const {
  std::vector<vk::Format> opaque_formats = attachments_format(RenderPassType::OPAQUE).color;
  std::vector<tge::RenderPass> opaque = create_gbuffer_pass(
      opaque_formats,
      pipeline_manager.graphics_layout(),
      descriptor_manager.descriptor_sets(DescriptorLayoutType::FINAL),
      static_cast<uint32_t>(DescriptorLayoutType::FINAL),
      0
  );

  std::map<RenderPassType, std::vector<RenderPass>> result;
  result.emplace(RenderPassType::OPAQUE, std::move(opaque));
  return result;
}
