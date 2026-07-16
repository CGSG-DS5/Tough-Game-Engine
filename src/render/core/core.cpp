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
    , command_manager(ctx.physical_device(), device, surface, frames_in_flight)
    , descriptor_manager(device, frames_in_flight)
    , render_finished_semaphores(device.create_semaphores(swapchain.num_of_images()))
    , image_available_semaphores(device.create_semaphores(frames_in_flight))
    , pipeline_manager(device, descriptor_manager.layouts())
    //, update_command_buffer(std::move(create_command_buffers(1)[0]))
    , /// :TODO: Delete
    tmp_pipeline(pipeline_manager.create_graphics_pipeline(
        device,
        vertex_type::NoVertices{},
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
        .dstSet = descriptor_manager.descriptor_sets(DescriptorLayoutType::RENDER)[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &buffer_info
    };

    device->updateDescriptorSets(descriptor_write, {});
  }

  vk::DescriptorImageInfo image_info{.imageView = tmp_img.get_image_view(), .imageLayout = tmp_img.get_image_layout()};
  vk::WriteDescriptorSet descriptor_write{
      .dstSet = descriptor_manager.descriptor_sets(DescriptorLayoutType::MATERIAL)[0],
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = vk::DescriptorType::eSampledImage,
      .pImageInfo = &image_info
  };

  // device.updateDescriptorSets(descriptor_write, {});

  std::vector<float> tmp_data{1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1};

  update_image({reinterpret_cast<const char*>(tmp_data.data()), tmp_data.size() * sizeof(float)}, tmp_img);
}

tge::Core::~Core() {
  command_manager.wait_idle();
}

void tge::Core::resize() {
  swapchain.resize();

  /// NEW_CODE
  tmp_render_pass.resize(swapchain.screen_size());
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

  /*
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
  */
}

uint32_t tge::Core::frame_index() const {
  return command_manager.frame_index();
}

#include <ctime>

void tge::Core::frame_start() {
  submit_update_buffer();

  command_manager.wait_finishing();
  swapchain.acquire_next_image(image_available_semaphores[frame_index()]);

  command_manager->reset();
  command_manager->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

  tmp_render_pass.begin(*command_manager, frame_index());
  tmp_render_pass.end(*command_manager, frame_index());

  swapchain.swapchain_image().switch_layout(*command_manager, vk::ImageLayout::eColorAttachmentOptimal);

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

  command_manager->beginRendering(render_info);

  command_manager->setViewport(
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
  command_manager->setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.screen_size()));

  command_manager->bindPipeline(vk::PipelineBindPoint::eGraphics, tmp_pipeline);

  command_manager->bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      pipeline_manager.graphics_layout(),
      0,
      descriptor_manager.descriptor_sets(DescriptorLayoutType::RENDER)[frame_index()],
      nullptr
  );

  std::vector<float> s{clock() / static_cast<float>(CLOCKS_PER_SEC)};
  command_manager->pushConstants(
      pipeline_manager.graphics_layout(),
      vk::ShaderStageFlagBits::eAllGraphics,
      0,
      static_cast<const vk::ArrayProxy<const float>&>(s)
  );

  *static_cast<float*>(tmp_buffers[frame_index()].get_mapped_data()) = swapchain.screen_size().width / 1920.f;

  command_manager->draw(1, 1, 0, 0);
}

void tge::Core::frame_end() {
  command_manager->endRendering();

  swapchain.swapchain_image().switch_layout(*command_manager, vk::ImageLayout::ePresentSrcKHR);

  command_manager->end();

  command_manager.submit(
      image_available_semaphores[frame_index()],
      render_finished_semaphores[swapchain.image_index()]
  );

  command_manager.present(render_finished_semaphores[swapchain.image_index()], swapchain);
}
