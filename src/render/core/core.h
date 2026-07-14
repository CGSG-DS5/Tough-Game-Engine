/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef __tge_core_h_
#define __tge_core_h_

#include "create_infos/infos.h"
#include "vulkan_context.h"
#include "pipelines/graphics_pipeline.h"
#include "render_pass.h"
#include "surface.h"
#include "vma_wrapper/vma_allocator.h"

namespace tge {
  class Core {
  public:
    enum struct DescriptorSetLayoutType : uint32_t {
      RENDER = 0,
      MATERIAL = 1,
      FINAL = 2,
    };

    Core(SDL_Window* window, bool vsync, bool triple_buffer);
    ~Core();

    void resize();

    void frame_start();
    void frame_end();

    const vk::raii::CommandBuffer& get_render_cmd_buf() const;

    void update_image(std::span<const char> data, Image& img);

  private:
    VulkanContext ctx;

    const RaiiSurface surface;
    const vk::raii::Device device;
    const MemoryAllocator allocator;
    const uint32_t device_present_mask;
    const uint32_t queue_family_index;
    const vk::raii::Queue queue;

    vk::Extent2D screen_size;
    const vk::PresentModeKHR swapchain_present_mode;
    const uint32_t frames_in_flight;
    vk::raii::SwapchainKHR swapchain;
    std::vector<Image> swapchain_images;

    const std::vector<vk::raii::DescriptorSetLayout> descriptor_set_layouts_raii;
    const std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;
    const vk::PushConstantRange push_constant_range{.stageFlags = vk::ShaderStageFlagBits::eAllGraphics, .size = 4};
    const vk::raii::PipelineLayout graphics_layout;

    const vk::raii::CommandPool command_pool;
    const vk::raii::DescriptorPool descriptor_pool;

    const std::map<DescriptorSetLayoutType, std::vector<vk::raii::DescriptorSet>> descriptor_sets_raii;
    const std::map<DescriptorSetLayoutType, std::vector<vk::DescriptorSet>> descriptor_sets;

    const std::vector<vk::raii::Fence> fences;
    const std::vector<vk::raii::Semaphore> image_available_semaphores;
    const std::vector<vk::raii::Semaphore> render_finished_semaphores;
    const std::vector<vk::raii::CommandBuffer> render_command_buffers;

    const vk::raii::CommandBuffer update_command_buffer;
    std::vector<char> update_data{};
    std::vector<std::pair<Image&, vk::BufferImageCopy>> update_command_buffer_data{};

    uint32_t frame_index{};
    uint32_t image_index{};

    //////// NEW CODE

    vk::PushConstantRange tmp_range{.stageFlags = vk::ShaderStageFlagBits::eAllGraphics, .size = 4};
    AttachmentsInfo attachments_info{.color_attachments_formats = {vk::Format::eB8G8R8A8Unorm}};
    std::vector<Buffer> tmp_buffers;
    GraphicsPipeline tmp_pipeline;

    RenderPassFactory render_pass_factory;
    RenderPass tmp_render_pass;

    Image tmp_img;

    //////// NEW CODE

    vk::raii::Device create_device(SDL_Window* window);
    uint32_t get_queue_family_index();
    vk::raii::Queue create_queue();
    MemoryAllocator create_allocator();

    vk::PresentModeKHR get_swapchain_present_mode(bool vsync, bool triple_buffer);
    vk::raii::SwapchainKHR create_swapchain();
    std::vector<Image> create_swapchain_images();

    static const std::map<DescriptorSetLayoutType, std::vector<vk::DescriptorSetLayoutBinding>>& get_layout_bindings();
    std::vector<vk::raii::DescriptorSetLayout> create_descriptor_set_layout() const;

    vk::raii::CommandPool create_command_pool();
    vk::raii::DescriptorPool create_descriptor_pool();

    std::map<DescriptorSetLayoutType, std::vector<vk::raii::DescriptorSet>> create_all_descriptor_sets();
    std::vector<vk::raii::DescriptorSet> create_descriptor_sets(DescriptorSetLayoutType type);

    std::vector<vk::raii::Fence> create_fences();
    std::vector<vk::raii::Semaphore> create_semaphores(uint32_t num);
    std::vector<vk::raii::CommandBuffer> create_command_buffers(uint32_t num);

    void submit_update_buffer();
  };
} // namespace tge

#endif // __tge_core_h_
