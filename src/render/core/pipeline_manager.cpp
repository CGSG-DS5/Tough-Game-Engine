/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::PipelineManager::PipelineManager(const vk::raii::Device& device, std::span<const vk::DescriptorSetLayout> layouts)
    : device(device)
    , push_constant_range({.stageFlags = vk::ShaderStageFlagBits::eAllGraphics, .size = 4})
    , m_graphics_layout(
          device,
          {.setLayoutCount = static_cast<uint32_t>(layouts.size()),
           .pSetLayouts = layouts.data(),
           .pushConstantRangeCount = 1,
           .pPushConstantRanges = &push_constant_range}
      ) {}

vk::PipelineLayout tge::PipelineManager::graphics_layout() const {
  return m_graphics_layout;
}
