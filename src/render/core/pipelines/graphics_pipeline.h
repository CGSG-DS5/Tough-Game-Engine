/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __graphics_pipeline_h_
#define __graphics_pipeline_h_

#include "../core_exception.h"
#include "../render_pass.h"

namespace tge {
  namespace topology {
    struct NoVertices {};
  } // namespace topology

  class GraphicsPipeline {
  public:
    /***
     * Vert::get_sizes method must exist
     * if Vert is topology::NoVertices, pipeline will be created with no attributes
     ***/
    template<typename Vert>
    GraphicsPipeline(
        const vk::raii::PipelineLayout& layout,
        const vk::raii::Device& device,
        const Vert& __tmp,
        const std::string& name,
        vk::PrimitiveTopology topology,
        const AttachmentsInfo& attachments_info,
        vk::CullModeFlags cull_mode = vk::CullModeFlagBits::eBack,
        bool dynamic_polygon_mode = false
    )
        : device(device)
        , name(name) {
      modules = create_shader_modules();
      auto shader_stages = create_shader_stages();
      auto dynamic_states = create_dynamic_states(dynamic_polygon_mode);
      auto dynamic_state = create_dynamic_state(dynamic_states);
      auto vertex_input = create_vertex_input<Vert>();
      auto input_assembly = create_input_assembly(topology);
      auto viewport_state = create_viewport_state();
      auto rasterizer = create_rasterizer(cull_mode);
      auto multisample_state = create_multisample_state();
      auto depth_stencil_state = create_depth_stencil_state(attachments_info.depth_attachment_format.has_value());
      auto attachment_states =
          create_color_blend_attachment_states(static_cast<uint32_t>(attachments_info.color_attachments_formats.size())
          );
      auto color_blend_states = create_color_blend_state(attachment_states);
      auto rendering = create_rendering(attachments_info);

      vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipeline_create_info = {
          {.stageCount = static_cast<uint32_t>(shader_stages.size()),
           .pStages = shader_stages.data(),
           .pVertexInputState = &vertex_input,
           .pInputAssemblyState = &input_assembly,
           .pViewportState = &viewport_state,
           .pRasterizationState = &rasterizer,
           .pMultisampleState = &multisample_state,
           .pDepthStencilState = attachments_info.depth_attachment_format ? &depth_stencil_state : nullptr,
           .pColorBlendState = &color_blend_states,
           .pDynamicState = &dynamic_state,
           .layout = layout,
           .renderPass = nullptr},
          rendering
      };

      pipeline = device.createGraphicsPipeline(nullptr, pipeline_create_info.get<vk::GraphicsPipelineCreateInfo>());
    }

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    GraphicsPipeline(GraphicsPipeline&& other) noexcept;

    ~GraphicsPipeline();

    operator vk::Pipeline() const;

  private:
    const vk::raii::Device& device;
    std::string name;
    std::map<std::string, vk::raii::ShaderModule> modules;
    vk::raii::Pipeline pipeline{nullptr};

    std::optional<vk::raii::ShaderModule> create_shader_module(const std::string& path) const;
    std::map<std::string, vk::raii::ShaderModule> create_shader_modules() const;
    std::vector<vk::PipelineShaderStageCreateInfo> create_shader_stages() const;
    std::vector<vk::DynamicState> create_dynamic_states(bool polygon_mode) const;
    vk::PipelineDynamicStateCreateInfo create_dynamic_state(const std::vector<vk::DynamicState>& dynamic_states) const;
    vk::PipelineInputAssemblyStateCreateInfo create_input_assembly(vk::PrimitiveTopology topology) const;
    vk::PipelineViewportStateCreateInfo create_viewport_state() const;
    vk::PipelineRasterizationStateCreateInfo create_rasterizer(vk::CullModeFlags cull_mode) const;
    vk::PipelineMultisampleStateCreateInfo create_multisample_state() const;
    vk::PipelineDepthStencilStateCreateInfo create_depth_stencil_state(bool is_depth) const;
    std::vector<vk::PipelineColorBlendAttachmentState> create_color_blend_attachment_states(uint32_t attachment_num);
    vk::PipelineColorBlendStateCreateInfo
    create_color_blend_state(const std::vector<vk::PipelineColorBlendAttachmentState>& attachment_states);
    vk::PipelineRenderingCreateInfo create_rendering(const AttachmentsInfo& attachments_info);

    template<typename Vert>
    vk::PipelineVertexInputStateCreateInfo create_vertex_input() const {
      vk::VertexInputBindingDescription binding_description{
          .binding = 0,
          .stride = sizeof(Vert),
          .inputRate = vk::VertexInputRate::eVertex
      };

      std::vector<uint32_t> vert_sizes = Vert::get_sizes();
      std::vector<vk::VertexInputAttributeDescription> attribute_descriptions(vert_sizes.size());

      static auto get_format = [](uint32_t size) -> vk::Format {
        switch (size) {
        case 8:
          return vk::Format::eR32G32B32A32Sfloat;
        case 4:
          return vk::Format::eR32G32B32Sfloat;
        case 2:
          return vk::Format::eR32G32Sfloat;
        case 1:
          return vk::Format::eR32Sfloat;
        default:
          throw CoreException("Invalid vertex size: " + size);
        }
      };

      uint32_t offset = 0;
      for (uint32_t i = 0; i < vert_sizes.size(); i++) {
        attribute_descriptions[i] =
            {.location = i, .binding = 0, .format = get_format(vert_sizes[i]), .offset = offset};
        offset += vert_sizes[i];
      }

      vk::PipelineVertexInputStateCreateInfo vertex_input{
          .vertexBindingDescriptionCount = 1,
          .pVertexBindingDescriptions = &binding_description,
          .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()),
          .pVertexAttributeDescriptions = attribute_descriptions.data(),
      };

      return vertex_input;
    }
  };

  template<>
  inline vk::PipelineVertexInputStateCreateInfo GraphicsPipeline::create_vertex_input<topology::NoVertices>() const {
    return {};
  }
} // namespace tge

#endif // __graphics_pipeline_h_
