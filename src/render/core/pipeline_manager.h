/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_pipeline_manager_h_
#define __tge_pipeline_manager_h_

#include "pipelines/graphics_pipeline.h"

namespace tge {
  class PipelineManager {
  public:
    PipelineManager(const vk::raii::Device& device, std::span<const vk::DescriptorSetLayout> layouts);

    /***
     * Vert::get_sizes method must exist
     * if Vert is topology::NoVertices, pipeline will be created with no attributes
     ***/
    template<typename... Args>
    GraphicsPipeline create_graphics_pipeline(Args&&... args) const {
      return GraphicsPipeline(m_graphics_layout, device, std::forward<Args>(args)...);
    }

    vk::PipelineLayout graphics_layout() const;

  private:
    const vk::raii::Device& device;
    vk::PushConstantRange push_constant_range;
    vk::raii::PipelineLayout m_graphics_layout;
  };
} // namespace tge

#endif // __tge_pipeline_manager_h_
