/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_core_h_
#define __tge_core_h_

#include "command_manager.h"
#include "create_infos/infos.h"
#include "descriptor_manager.h"
#include "device.h"
#include "image_manager.h"
#include "pipeline_manager.h"
#include "pipelines/graphics_pipeline.h"
#include "render_pass.h"
#include "surface.h"
#include "swapchain.h"
#include "vma_wrapper/vma_allocator.h"
#include "vulkan_context.h"

namespace tge {
  class Core {
  public:
    Core(SDL_Window* window, bool vsync, bool triple_buffer);
    ~Core();

    void resize();

    void frame_start();
    void frame_end();

  private:
    VulkanContext ctx;
    Surface surface;
    Device device;
    MemoryAllocator allocator;
    Swapchain swapchain;
    uint32_t frames_in_flight;

    DescriptorManager descriptor_manager;
    CommandManager command_manager;
    PipelineManager pipeline_manager;
    ImageManager image_manager;

    std::vector<vk::raii::Semaphore> render_finished_semaphores;
    std::vector<vk::raii::Semaphore> image_available_semaphores;

    //////// NEW CODE

    vk::PushConstantRange tmp_range{.stageFlags = vk::ShaderStageFlagBits::eAllGraphics, .size = 4};
    AttachmentsInfo attachments_info{.color_attachments_formats = {vk::Format::eB8G8R8A8Unorm}};
    std::vector<Buffer> tmp_buffers;
    GraphicsPipeline tmp_pipeline;

    RenderPassFactory render_pass_factory;
    RenderPass tmp_render_pass;

    Image tmp_img;

    //////// NEW CODE

    uint32_t frame_index() const;
  };
} // namespace tge

#endif // __tge_core_h_
