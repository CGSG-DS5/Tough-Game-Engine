/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

template<typename T>
auto get_handle_from_raii(const std::vector<T>& data) {
  using HandleType = typename T::CppType;
  std::vector<HandleType> result;
  result.reserve(data.size());
  for (const auto& item : data) {
    result.push_back(*item);
  }
  return result;
}

template<typename S, typename T>
auto get_handle_from_raii(const std::map<S, std::vector<T>>& data) {
  using HandleType = typename T::CppType;
  std::map<S, std::vector<HandleType>> result;
  for (const auto& [k, v] : data) {
    result[k] = get_handle_from_raii(v);
  }
  return result;
}

tge::Core::Core(SDL_Window* window, bool vsync, bool triple_buffer)
    : ctx()
    , surface(ctx.instance(), window)
    , device(ctx.physical_device(), surface)
    , allocator(ctx.instance(), ctx.physical_device(), device)
    , swapchain(ctx.physical_device(), device, surface, allocator, vsync, triple_buffer)
    , frames_in_flight(swapchain.num_of_images() - 1)
    , queue_family_index(get_queue_family_index())
    , queue(create_queue())
    , descriptor_set_layouts_raii(create_descriptor_set_layout())
    , descriptor_set_layouts(get_handle_from_raii(descriptor_set_layouts_raii))
    , graphics_layout(
          device,
          {.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
           .pSetLayouts = descriptor_set_layouts.data(),
           .pushConstantRangeCount = 1,
           .pPushConstantRanges = &push_constant_range}
      )
    , command_pool(create_command_pool())
    , descriptor_pool(create_descriptor_pool())
    , descriptor_sets_raii(create_all_descriptor_sets())
    , descriptor_sets(get_handle_from_raii(descriptor_sets_raii))
    , fences(create_fences())
    , image_available_semaphores(create_semaphores(frames_in_flight))
    , render_command_buffers(create_command_buffers(frames_in_flight))
    , update_command_buffer(std::move(create_command_buffers(1)[0]))
    , /// :TODO: Delete
    tmp_pipeline(GraphicsPipeline(
        graphics_layout,
        device,
        topology::NoVertices{},
        "test_shader",
        vk::PrimitiveTopology::ePointList,
        attachments_info,
        vk::CullModeFlagBits::eNone
    ))
    , tmp_buffers()
    , render_pass_factory(allocator, device)
    , tmp_render_pass(render_pass_factory.create_gbuffer_pass(swapchain.screen_size(), 2, frames_in_flight))
    , tmp_img(
          allocator,
          device,
          vk::Format::eR32G32B32A32Sfloat,
          {2, 2, 1},
          1,
          vk::SampleCountFlagBits::e1,
          vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
          false
      ) {
  tmp_buffers.reserve(frames_in_flight);
  for (uint32_t i = 0; i < frames_in_flight; i++) {
    tmp_buffers.emplace_back(allocator, 4, false, vk::BufferUsageFlagBits::eUniformBuffer);
  }

  for (size_t i = 0; i < frames_in_flight; i++) {
    vk::DescriptorBufferInfo buffer_info{.buffer = tmp_buffers[i].get_buffer(), .offset = 0, .range = 4};
    vk::WriteDescriptorSet descriptor_write{
        .dstSet = descriptor_sets.at(DescriptorSetLayoutType::RENDER)[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &buffer_info
    };

    device->updateDescriptorSets(descriptor_write, {});
  }

  for (size_t i = 0; i < frames_in_flight; i++) {
    vk::DescriptorImageInfo image_info{
        .imageView = tmp_img.get_image_view(),
        .imageLayout = tmp_img.get_image_layout()
    };
    vk::WriteDescriptorSet descriptor_write{
        .dstSet = descriptor_sets.at(DescriptorSetLayoutType::MATERIAL)[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eSampledImage,
        .pImageInfo = &image_info
    };

    // device.updateDescriptorSets(descriptor_write, {});
  }

  std::vector<float> tmp_data{1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1};

  update_image({reinterpret_cast<const char*>(tmp_data.data()), tmp_data.size() * sizeof(float)}, tmp_img);
}

tge::Core::~Core() {
  queue.waitIdle();
}

uint32_t tge::Core::get_queue_family_index() {
  return QueueInfo(ctx.physical_device(), surface).get().queueFamilyIndex;
}

vk::raii::Queue tge::Core::create_queue() {
  return device->getQueue(queue_family_index, 0);
}

/***
 * Swapchain
 ***/

void tge::Core::resize() {
  swapchain.resize();

  /// NEW_CODE
  tmp_render_pass.resize(swapchain.screen_size());
}

const std::map<tge::Core::DescriptorSetLayoutType, std::vector<vk::DescriptorSetLayoutBinding>>&
tge::Core::get_layout_bindings() {
  static const std::map<DescriptorSetLayoutType, std::vector<vk::DescriptorSetLayoutBinding>> bindings{
      {DescriptorSetLayoutType::RENDER,
       {{.binding = 0,
         .descriptorType = vk::DescriptorType::eUniformBuffer,
         .descriptorCount = 1,
         .stageFlags = vk::ShaderStageFlagBits::eAll},
        {.binding = 1,
         .descriptorType = vk::DescriptorType::eStorageBuffer,
         .descriptorCount = 3,
         .stageFlags = vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eVertex}}},
      {DescriptorSetLayoutType::MATERIAL,
       {{.binding = 0,
         .descriptorType = vk::DescriptorType::eCombinedImageSampler,
         .descriptorCount = 1,
         .stageFlags = vk::ShaderStageFlagBits::eFragment}}},
      {DescriptorSetLayoutType::FINAL,
       {{.binding = 0,
         .descriptorType = vk::DescriptorType::eCombinedImageSampler,
         .descriptorCount = 1,
         .stageFlags = vk::ShaderStageFlagBits::eFragment}}}
  };

  return bindings;
}

std::vector<vk::raii::DescriptorSetLayout> tge::Core::create_descriptor_set_layout() const {
  const auto& layout_bindings = get_layout_bindings();

  std::vector<vk::raii::DescriptorSetLayout> res;
  for (const auto& [type, bindings] : layout_bindings) {
    vk::DescriptorSetLayoutCreateInfo info{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    res.emplace_back(device, info);
  }

  return res;
}

vk::raii::CommandPool tge::Core::create_command_pool() {
  return device->createCommandPool(
      {.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = queue_family_index}
  );
}

vk::raii::DescriptorPool tge::Core::create_descriptor_pool() {
  const auto& layout_bindings = get_layout_bindings();

  uint32_t max_sets = 0;
  std::vector<vk::DescriptorPoolSize> sizes;

  for (const auto& [type, bindings] : layout_bindings) {
    uint32_t num_sets = frames_in_flight;
    max_sets += num_sets;
    for (const auto& binding : bindings) {
      sizes.push_back({.type = binding.descriptorType, .descriptorCount = binding.descriptorCount * num_sets});
    }
  }

  return device->createDescriptorPool(
      {.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
       .maxSets = max_sets,
       .poolSizeCount = static_cast<uint32_t>(sizes.size()),
       .pPoolSizes = sizes.data()}
  );
}

std::map<tge::Core::DescriptorSetLayoutType, std::vector<vk::raii::DescriptorSet>>
tge::Core::create_all_descriptor_sets() {
  std::map<DescriptorSetLayoutType, std::vector<vk::raii::DescriptorSet>> res;
  for (uint32_t i = 0; i < descriptor_set_layouts.size(); i++) {
    auto type = static_cast<DescriptorSetLayoutType>(i);
    res.insert({type, create_descriptor_sets(type)});
  }

  return res;
}

std::vector<vk::raii::DescriptorSet> tge::Core::create_descriptor_sets(DescriptorSetLayoutType type) {
  std::vector<vk::DescriptorSetLayout> layouts(frames_in_flight, descriptor_set_layouts[static_cast<uint32_t>(type)]);

  return device->allocateDescriptorSets(
      {.descriptorPool = descriptor_pool,
       .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
       .pSetLayouts = layouts.data()}
  );
}

std::vector<vk::raii::Fence> tge::Core::create_fences() {
  std::vector<vk::raii::Fence> result;
  result.reserve(frames_in_flight);

  for (int32_t i = 0; i < frames_in_flight; i++) {
    result.emplace_back(device->createFence({.flags = vk::FenceCreateFlagBits::eSignaled}));
  }

  return result;
}

std::vector<vk::raii::Semaphore> tge::Core::create_semaphores(uint32_t num) {
  std::vector<vk::raii::Semaphore> result;
  result.reserve(num);

  for (int32_t i = 0; i < num; i++) {
    result.emplace_back(device->createSemaphore(vk::SemaphoreCreateInfo()));
  }

  return result;
}

std::vector<vk::raii::CommandBuffer> tge::Core::create_command_buffers(uint32_t num) {
  return device->allocateCommandBuffers(vk::CommandBufferAllocateInfo{
      .commandPool = command_pool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = num
  });
}

void tge::Core::update_image(std::span<const char> data, Image& img) {
  update_command_buffer_data.push_back(
      {img,
       {.bufferOffset = static_cast<uint32_t>(update_data.size()),
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
        .imageOffset = {0, 0, 0},
        .imageExtent = img.get_image_sizes()}}
  );

  update_data.insert(update_data.end(), data.begin(), data.end());
}

void tge::Core::submit_update_buffer() {
  if (update_data.empty()) {
    return;
  }

  Buffer staging_buffer(
      allocator,
      static_cast<uint32_t>(update_data.size()),
      false,
      vk::BufferUsageFlagBits::eTransferSrc
  );
  std::memcpy(staging_buffer.get_mapped_data(), update_data.data(), update_data.size());

  update_command_buffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  for (auto& [img, region] : update_command_buffer_data) {
    img.switch_layout(update_command_buffer, vk::ImageLayout::eTransferDstOptimal);
    update_command_buffer
        .copyBufferToImage(staging_buffer.get_buffer(), img.get_image(), vk::ImageLayout::eTransferDstOptimal, region);
    img.switch_layout(update_command_buffer, vk::ImageLayout::eShaderReadOnlyOptimal);
  }
  update_command_buffer.end();

  vk::SubmitInfo submitInfo{.commandBufferCount = 1, .pCommandBuffers = &*update_command_buffer};
  queue.submit(submitInfo, nullptr);
  queue.waitIdle();

  update_data.clear();
}

#include <ctime>

void tge::Core::frame_start() {
  submit_update_buffer();

  if (vk::Result res = device->waitForFences(*fences[frame_index], 1, UINT64_MAX); res != vk::Result::eSuccess) {
    throw CoreException("Wait for fence error", static_cast<int32_t>(res));
  }
  device->resetFences(*fences[frame_index]);

  swapchain.acquire_next_image(image_available_semaphores[frame_index]);

  get_render_cmd_buf().reset();
  get_render_cmd_buf().begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

  tmp_render_pass.begin(get_render_cmd_buf(), frame_index);
  tmp_render_pass.end(get_render_cmd_buf(), frame_index);

  swapchain.swapchain_image().switch_layout(get_render_cmd_buf(), vk::ImageLayout::eColorAttachmentOptimal);

  vk::RenderingAttachmentInfo color_attachment{
      .imageView = swapchain.swapchain_image().get_image_view(),
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .resolveMode = vk::ResolveModeFlagBits::eNone,
      .resolveImageLayout = vk::ImageLayout::eUndefined,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = {{0.30f, 0.47f, 0.8f, 1.f}}
  };

  vk::RenderingInfo render_info{
      .renderArea = {.offset = {0, 0}, .extent = swapchain.screen_size()},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment
  };

  get_render_cmd_buf().beginRendering(render_info);

  get_render_cmd_buf().setViewport(
      0,
      vk::Viewport(
          0.f,
          0.f,
          static_cast<float>(swapchain.screen_size().width),
          static_cast<float>(swapchain.screen_size().height),
          0.f,
          1.f
      )
  );
  get_render_cmd_buf().setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.screen_size()));

  get_render_cmd_buf().bindPipeline(vk::PipelineBindPoint::eGraphics, tmp_pipeline);

  get_render_cmd_buf().bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      graphics_layout,
      0,
      descriptor_sets.at(DescriptorSetLayoutType::RENDER)[frame_index],
      nullptr
  );

  std::vector<float> s{clock() / static_cast<float>(CLOCKS_PER_SEC)};
  get_render_cmd_buf().pushConstants(
      graphics_layout,
      vk::ShaderStageFlagBits::eAllGraphics,
      0,
      static_cast<const vk::ArrayProxy<const float>&>(s)
  );

  *static_cast<float*>(tmp_buffers[frame_index].get_mapped_data()) = swapchain.screen_size().width / 1920.f;

  get_render_cmd_buf().draw(1, 1, 0, 0);
}

void tge::Core::frame_end() {
  get_render_cmd_buf().endRendering();

  swapchain.swapchain_image().switch_layout(get_render_cmd_buf(), vk::ImageLayout::ePresentSrcKHR);

  get_render_cmd_buf().end();

  vk::SemaphoreSubmitInfo wait_semaphore_info{
      .semaphore = image_available_semaphores[frame_index],
      .stageMask = vk::PipelineStageFlagBits2::eAllCommands
  };

  vk::CommandBufferSubmitInfo cmd_buf_submit_info{.commandBuffer = get_render_cmd_buf()};

  vk::SemaphoreSubmitInfo signal_semaphore_info{
      .semaphore = swapchain.semaphore(),
      .stageMask = vk::PipelineStageFlagBits2::eAllCommands
  };

  vk::SubmitInfo2 submit_info{
      .waitSemaphoreInfoCount = 1,
      .pWaitSemaphoreInfos = &wait_semaphore_info,
      .commandBufferInfoCount = 1,
      .pCommandBufferInfos = &cmd_buf_submit_info,
      .signalSemaphoreInfoCount = 1,
      .pSignalSemaphoreInfos = &signal_semaphore_info
  };
  queue.submit2(submit_info, fences[frame_index]);

  auto present_sem = swapchain.semaphore();
  auto present_swp = swapchain.swapchain();
  auto present_ind = swapchain.image_index();
  vk::PresentInfoKHR present_info{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &present_sem,
      .swapchainCount = 1,
      .pSwapchains = &present_swp,
      .pImageIndices = &present_ind
  };

  if (vk::Result res = queue.presentKHR(present_info); res != vk::Result::eSuccess) {
    throw CoreException("Error in present", static_cast<int32_t>(res));
  }

  frame_index = (frame_index + 1) % frames_in_flight;
}

const vk::raii::CommandBuffer& tge::Core::get_render_cmd_buf() const {
  return render_command_buffers[frame_index];
}
