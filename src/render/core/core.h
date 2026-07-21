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

    template<typename T>
    void create_shader(
        RenderPassType type,
        const std::string& name,
        vk::PrimitiveTopology topology,
        vk::CullModeFlags cull_mode = vk::CullModeFlagBits::eBack,
        bool depth_test = true,
        bool depth_write = true
    ) {
      graphics_pipelines.try_emplace(type);
      switch (type) {
      case RenderPassType::OPAQUE:
        graphics_pipelines[type].push_back(pipeline_manager.create_graphics_pipeline(
            T{},
            name,
            topology,
            render_pass_manager.attachments_format(type),
            cull_mode,
            false,
            depth_test,
            depth_write,
            false
        ));
        break;
      }
    }

    void create_final_shader(const std::string& name);

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

    RenderPassManager render_pass_manager;

    std::map<RenderPassType, std::vector<GraphicsPipeline>> graphics_pipelines;
    ////// TMP CODE

    Buffer tmp_vert_buffer;
    Buffer tmp_ind_buffer;
    uint32_t num_of_triangles{32};

    ////// TMP CODE

    uint32_t frame_index() const;
    void draw_final_pass();
  };
} // namespace tge

#endif // __tge_core_h_
