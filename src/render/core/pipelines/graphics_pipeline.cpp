/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

tge::GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) noexcept
    : device(other.device)
    , name(std::move(other.name))
    , modules(std::move(other.modules))
    , pipeline(std::move(other.pipeline)) {}

tge::GraphicsPipeline& tge::GraphicsPipeline::operator=(GraphicsPipeline&& other) noexcept {
  if (this != &other) {
    name = std::move(other.name);
    modules = std::move(other.modules);
    pipeline = std::move(other.pipeline);
  }

  return *this;
}

tge::GraphicsPipeline::~GraphicsPipeline() {}

tge::GraphicsPipeline::operator vk::Pipeline() const {
  return pipeline;
}

std::vector<vk::PipelineShaderStageCreateInfo> tge::GraphicsPipeline::create_shader_stages() const {
  static std::vector<std::tuple<std::string, vk::ShaderStageFlagBits>> stages{
      {"vert", vk::ShaderStageFlagBits::eVertex},
      {"frag", vk::ShaderStageFlagBits::eFragment},
      {"geom", vk::ShaderStageFlagBits::eGeometry},
      {"ctrl", vk::ShaderStageFlagBits::eTessellationControl},
      {"eval", vk::ShaderStageFlagBits::eTessellationEvaluation},
  };

  std::vector<vk::PipelineShaderStageCreateInfo> stage_create_infos;
  for (const auto& [stage_name, stage_bit] : stages) {
    if (!modules.contains(stage_name)) {
      continue;
    }

    stage_create_infos.push_back({.stage = stage_bit, .module = modules.at(stage_name), .pName = "main"});
  }

  return stage_create_infos;
}

std::optional<vk::raii::ShaderModule> tge::GraphicsPipeline::create_shader_module(const std::string& path) const {
  auto shader_code = file_system::read_resource(path);
  if (!shader_code) {
    return {};
  }
  vk::ShaderModuleCreateInfo shader_module_create_info{
      .codeSize = shader_code->size() * sizeof(char),
      .pCode = reinterpret_cast<const uint32_t*>(shader_code->data())
  };
  return device.createShaderModule(shader_module_create_info);
}

std::map<std::string, vk::raii::ShaderModule> tge::GraphicsPipeline::create_shader_modules() const {
  std::map<std::string, vk::raii::ShaderModule> shader_modules;

  static std::vector<std::string> stage_names{"vert", "frag", "geom", "eval", "ctrl"};
  for (const auto& stage_name : stage_names) {
    auto module = create_shader_module("shaders/" + name + "/" + stage_name + ".spv");
    if (module) {
      shader_modules.emplace(stage_name, std::move(*module));
    }
  }

  return shader_modules;
}

std::vector<vk::DynamicState> tge::GraphicsPipeline::create_dynamic_states(bool polygon_mode) const {
  std::vector<vk::DynamicState> dynamic_states{vk::DynamicState::eViewport, vk::DynamicState::eScissor};

  if (polygon_mode) {
    dynamic_states.push_back(vk::DynamicState::ePolygonModeEXT);
  }
  return dynamic_states;
}

vk::PipelineDynamicStateCreateInfo
tge::GraphicsPipeline::create_dynamic_state(const std::vector<vk::DynamicState>& dynamic_states) const {
  vk::PipelineDynamicStateCreateInfo dynamic_state{
      .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data()
  };

  return dynamic_state;
}

vk::PipelineInputAssemblyStateCreateInfo tge::GraphicsPipeline::create_input_assembly(vk::PrimitiveTopology topology
) const {
  bool restart_enabled = false;

  switch (topology) {
  case vk::PrimitiveTopology::eLineStrip:
  case vk::PrimitiveTopology::eTriangleStrip:
  case vk::PrimitiveTopology::eTriangleFan:
  case vk::PrimitiveTopology::eLineListWithAdjacency:
  case vk::PrimitiveTopology::eLineStripWithAdjacency:
  case vk::PrimitiveTopology::eTriangleListWithAdjacency:
  case vk::PrimitiveTopology::eTriangleStripWithAdjacency:
  case vk::PrimitiveTopology::ePatchList:
    restart_enabled = true;
  }

  vk::PipelineInputAssemblyStateCreateInfo input_assembly{
      .topology = topology,
      .primitiveRestartEnable = restart_enabled
  };

  return input_assembly;
}

vk::PipelineViewportStateCreateInfo tge::GraphicsPipeline::create_viewport_state() const {
  vk::PipelineViewportStateCreateInfo viewport_state{.viewportCount = 1, .scissorCount = 1};
  return viewport_state;
}

vk::PipelineRasterizationStateCreateInfo tge::GraphicsPipeline::create_rasterizer(vk::CullModeFlags cull_mode) const {
  vk::PipelineRasterizationStateCreateInfo rasterization{
      .depthClampEnable = false,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = cull_mode,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .lineWidth = 1.f,
  };

  return rasterization;
}

vk::PipelineMultisampleStateCreateInfo tge::GraphicsPipeline::create_multisample_state() const {
  vk::PipelineMultisampleStateCreateInfo multisample_state{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = false
  };
  return multisample_state;
}

vk::PipelineDepthStencilStateCreateInfo tge::GraphicsPipeline::create_depth_stencil_state(
    bool depth_test,
    bool depth_write,
    bool stencil_test,
    bool is_depth,
    bool is_stencil
) const {
  vk::StencilOpState stencil_op_state{
      .failOp = vk::StencilOp::eKeep,
      .passOp = stencil_test ? vk::StencilOp::eKeep : vk::StencilOp::eReplace,
      .depthFailOp = vk::StencilOp::eKeep,
      .compareOp = stencil_test ? vk::CompareOp::eEqual : vk::CompareOp::eAlways,
      .compareMask = 0xFF,
      .writeMask = stencil_test ? 0x00u : 0xFFu,
      .reference = 1
  };

  vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{
      .depthTestEnable = is_depth && depth_test,
      .depthWriteEnable = is_depth && depth_write,
      .depthCompareOp = vk::CompareOp::eLess,
      .depthBoundsTestEnable = vk::False,
      .stencilTestEnable = is_stencil,
      .front = stencil_op_state,
      .back = stencil_op_state,
      .minDepthBounds = 0.f,
      .maxDepthBounds = 1.f,
  };

  return depth_stencil_state;
}

std::vector<vk::PipelineColorBlendAttachmentState>
tge::GraphicsPipeline::create_color_blend_attachment_states(uint32_t attachment_num) {
  std::vector<vk::PipelineColorBlendAttachmentState> states(attachment_num);
  for (uint32_t i = 0; i < attachment_num; i++) {
    states[i] = {
        .blendEnable = false,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };
  }

  return states;
}

vk::PipelineColorBlendStateCreateInfo tge::GraphicsPipeline::create_color_blend_state(
    const std::vector<vk::PipelineColorBlendAttachmentState>& attachment_states
) {
  vk::PipelineColorBlendStateCreateInfo color_blend_state{
      .logicOpEnable = false,
      .logicOp = vk::LogicOp::eCopy,
      .attachmentCount = static_cast<uint32_t>(attachment_states.size()),
      .pAttachments = attachment_states.data()
  };

  return color_blend_state;
}

vk::PipelineRenderingCreateInfo tge::GraphicsPipeline::create_rendering(const AttachmentsFormat& attachments_format) {
  vk::PipelineRenderingCreateInfoKHR pipeline_rendering_create_info{
      .colorAttachmentCount = static_cast<uint32_t>(attachments_format.color.size()),
      .pColorAttachmentFormats = attachments_format.color.data(),
      .depthAttachmentFormat = attachments_format.depth ? *attachments_format.depth : vk::Format::eUndefined,
      .stencilAttachmentFormat = attachments_format.has_stencil() ? *attachments_format.depth : vk::Format::eUndefined,
  };

  return pipeline_rendering_create_info;
}
