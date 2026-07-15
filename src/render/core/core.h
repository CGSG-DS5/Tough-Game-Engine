/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_core_h_
#define __tge_core_h_

#include "create_infos/infos.h"
#include "device.h"
#include "descriptor_manager.h"
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

    const vk::raii::CommandBuffer& get_render_cmd_buf() const;

    void update_image(std::span<const char> data, Image& img);

  private:
    VulkanContext ctx;
    Surface surface;
    Device device;
    MemoryAllocator allocator;
    Swapchain swapchain;
    uint32_t frames_in_flight;

    DescriptorManager descriptor_manager;

    const vk::PushConstantRange push_constant_range{.stageFlags = vk::ShaderStageFlagBits::eAllGraphics, .size = 4};
    const vk::raii::PipelineLayout graphics_layout;

    const uint32_t queue_family_index;
    const vk::raii::Queue queue;

    const vk::raii::CommandPool command_pool;

    const std::vector<vk::raii::Fence> fences;
    const std::vector<vk::raii::Semaphore> image_available_semaphores;
    const std::vector<vk::raii::CommandBuffer> render_command_buffers;

    const vk::raii::CommandBuffer update_command_buffer;
    std::vector<char> update_data{};
    std::vector<std::pair<Image&, vk::BufferImageCopy>> update_command_buffer_data{};

    uint32_t frame_index{};

    //////// NEW CODE

    vk::PushConstantRange tmp_range{.stageFlags = vk::ShaderStageFlagBits::eAllGraphics, .size = 4};
    AttachmentsInfo attachments_info{.color_attachments_formats = {vk::Format::eB8G8R8A8Unorm}};
    std::vector<Buffer> tmp_buffers;
    GraphicsPipeline tmp_pipeline;

    RenderPassFactory render_pass_factory;
    RenderPass tmp_render_pass;

    Image tmp_img;

    //////// NEW CODE

    uint32_t get_queue_family_index();
    vk::raii::Queue create_queue();

    vk::raii::CommandPool create_command_pool();

    std::vector<vk::raii::Fence> create_fences();
    std::vector<vk::raii::Semaphore> create_semaphores(uint32_t num);
    std::vector<vk::raii::CommandBuffer> create_command_buffers(uint32_t num);

    void submit_update_buffer();
  };
} // namespace tge

#endif // __tge_core_h_
