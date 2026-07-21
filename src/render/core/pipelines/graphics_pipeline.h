/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __graphics_pipeline_h_
#define __graphics_pipeline_h_

#include "../core_exception.h"

namespace tge {
  struct AttachmentsFormat {
    std::vector<vk::Format> color;
    std::optional<vk::Format> depth;

    bool has_stencil() const {
      return depth == vk::Format::eD32SfloatS8Uint;
    }
  };

  namespace vertex_type {
    struct NoVertices {
      static constexpr std::vector<uint32_t> get_sizes() {
        return {};
      }
    };

    struct Test {
      float pos[3];
      float color[4];

      static constexpr std::vector<uint32_t> get_sizes() {
        return {12, 16};
      }
    };
  } // namespace vertex_type

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
        const AttachmentsFormat& attachments_format,
        vk::CullModeFlags cull_mode = vk::CullModeFlagBits::eBack,
        bool dynamic_polygon_mode = false,
        bool depth_test = true,
        bool depth_write = true,
        bool stencil_test = false
    )
        : device(device)
        , name(name) {
      modules = create_shader_modules();
      auto shader_stages = create_shader_stages();
      auto dynamic_states = create_dynamic_states(dynamic_polygon_mode);
      auto dynamic_state = create_dynamic_state(dynamic_states);
      auto binding_description = create_binding_description<Vert>();
      auto attribute_descriptions = create_attribute_descriptions<Vert>();
      auto vertex_input = create_vertex_input<Vert>(binding_description, attribute_descriptions);
      auto input_assembly = create_input_assembly(topology);
      auto viewport_state = create_viewport_state();
      auto rasterizer = create_rasterizer(cull_mode);
      auto multisample_state = create_multisample_state();
      auto depth_stencil_state = create_depth_stencil_state(
          depth_test,
          depth_write,
          stencil_test,
          attachments_format.depth.has_value(),
          attachments_format.has_stencil()
      );
      auto attachment_states =
          create_color_blend_attachment_states(static_cast<uint32_t>(attachments_format.color.size()));
      auto color_blend_states = create_color_blend_state(attachment_states);
      auto rendering = create_rendering(attachments_format);

      vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipeline_create_info = {
          {.stageCount = static_cast<uint32_t>(shader_stages.size()),
           .pStages = shader_stages.data(),
           .pVertexInputState = &vertex_input,
           .pInputAssemblyState = &input_assembly,
           .pViewportState = &viewport_state,
           .pRasterizationState = &rasterizer,
           .pMultisampleState = &multisample_state,
           .pDepthStencilState =
               (attachments_format.depth || attachments_format.has_stencil()) ? &depth_stencil_state : nullptr,
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
    GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept;

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
    vk::PipelineDepthStencilStateCreateInfo
    create_depth_stencil_state(bool depth_test, bool depth_write, bool stencil_test, bool is_depth, bool is_stencil)
        const;
    std::vector<vk::PipelineColorBlendAttachmentState> create_color_blend_attachment_states(uint32_t attachment_num);
    vk::PipelineColorBlendStateCreateInfo
    create_color_blend_state(const std::vector<vk::PipelineColorBlendAttachmentState>& attachment_states);
    vk::PipelineRenderingCreateInfo create_rendering(const AttachmentsFormat& attachments_format);

    template<typename Vert>
    vk::VertexInputBindingDescription  create_binding_description() const {
      vk::VertexInputBindingDescription binding_description{
          .binding = 0,
          .stride = sizeof(Vert),
          .inputRate = vk::VertexInputRate::eVertex
      };

      return binding_description;
    }

    template<typename Vert>
    std::vector<vk::VertexInputAttributeDescription> create_attribute_descriptions() const {
      std::vector<uint32_t> vert_sizes = Vert::get_sizes();
      std::vector<vk::VertexInputAttributeDescription> attribute_descriptions(vert_sizes.size());

      static auto get_format = [](uint32_t size) -> vk::Format {
        switch (size) {
        case 16:
          return vk::Format::eR32G32B32A32Sfloat;
        case 12:
          return vk::Format::eR32G32B32Sfloat;
        case 8:
          return vk::Format::eR32G32Sfloat;
        case 4:
          return vk::Format::eR32Sfloat;
        default:
          throw CoreException(std::format("Invalid vertex size: {}", size));
        }
      };

      uint32_t offset = 0;
      for (uint32_t i = 0; i < vert_sizes.size(); i++) {
        attribute_descriptions[i] =
            {.location = i, .binding = 0, .format = get_format(vert_sizes[i]), .offset = offset};
        offset += vert_sizes[i];
      }

      return attribute_descriptions;
    }

    template<typename Vert>
    vk::PipelineVertexInputStateCreateInfo create_vertex_input(
        const vk::VertexInputBindingDescription& binding_description,
        std::span<const vk::VertexInputAttributeDescription> attribute_descriptions
    ) const {
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
  inline vk::PipelineVertexInputStateCreateInfo GraphicsPipeline::create_vertex_input<vertex_type::NoVertices>(
      const vk::VertexInputBindingDescription& binding_description,
      std::span<const vk::VertexInputAttributeDescription> attribute_descriptions
  ) const {
    return {};
  }
} // namespace tge

#endif // __graphics_pipeline_h_
