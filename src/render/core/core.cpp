/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

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
    , image_manager(allocator, device, ctx.physical_device())
    , render_pass_manager(
          *device,
          image_manager,
          descriptor_manager,
          pipeline_manager,
          frames_in_flight,
          swapchain.screen_size()
      )
    , tmp_ind_buffer(allocator)
    , tmp_vert_buffer(allocator) {
  create_shader<vertex_type::Test>(
      RenderPassType::OPAQUE,
      "test_passes/opaque_pass",
      vk::PrimitiveTopology::eTriangleList,
      vk::CullModeFlagBits::eNone,
      true,
      true
  );

  create_final_shader("test_passes/final_pass");

  tmp_vert_buffer = Buffer(
      allocator,
      static_cast<uint32_t>(sizeof(vertex_type::Test) * 3) * num_of_triangles,
      true,
      vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
  );
  tmp_ind_buffer = Buffer(
      allocator,
      static_cast<uint32_t>(sizeof(uint32_t) * 3) * num_of_triangles,
      true,
      vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst
  );

  Buffer staging_vert(allocator);
  Buffer staging_ind(allocator);

  auto cmd_buf = command_manager.begin_single_commands();

  staging_vert = Buffer(
      allocator,
      static_cast<uint32_t>(sizeof(vertex_type::Test) * 3) * num_of_triangles,
      false,
      vk::BufferUsageFlagBits::eTransferSrc
  );
  staging_ind = Buffer(
      allocator,
      static_cast<uint32_t>(sizeof(uint32_t) * 3) * num_of_triangles,
      false,
      vk::BufferUsageFlagBits::eTransferSrc
  );

  srand(47);
  static auto get_rand = [](float min, float max) {
    return rand() / static_cast<float>(RAND_MAX) * (max - min) + min;
  };

  for (uint32_t i = 0; i < num_of_triangles; i++) {
    float depth = get_rand(0, 1);
    float color[4] = {depth, depth, depth, 1};
    for (uint32_t j = 0; j < 3; j++) {
      vertex_type::Test* data = static_cast<vertex_type::Test*>(staging_vert.get_mapped_data());
      std::copy(color, color + 4, data[i * 3 + j].color);
      data[i * 3 + j].pos[0] = get_rand(-1, 1);
      data[i * 3 + j].pos[1] = get_rand(-1, 1);
      data[i * 3 + j].pos[2] = depth;
    }

    for (uint32_t j = 0; j < 3; j++) {
      uint32_t* data = static_cast<uint32_t*>(staging_ind.get_mapped_data());
      data[i * 3 + j] = j;
    }
  }

  vk::BufferCopy region_vert{
      .srcOffset = 0,
      .dstOffset = 0,
      .size = static_cast<uint32_t>(sizeof(vertex_type::Test) * 3) * num_of_triangles
  };
  cmd_buf.copyBuffer(staging_vert.get_buffer(), tmp_vert_buffer.get_buffer(), region_vert);

  vk::BufferCopy region_ind{
      .srcOffset = 0,
      .dstOffset = 0,
      .size = static_cast<uint32_t>(sizeof(uint32_t) * 3) * num_of_triangles
  };
  cmd_buf.copyBuffer(staging_ind.get_buffer(), tmp_ind_buffer.get_buffer(), region_ind);

  command_manager.end_single_commands(std::move(cmd_buf));
}

tge::Core::~Core() {
  command_manager.wait_idle();
}

void tge::Core::resize() {
  command_manager.wait_idle();
  swapchain.resize();
  render_pass_manager.resize(swapchain.screen_size());
}

uint32_t tge::Core::frame_index() const {
  return command_manager.frame_index();
}

void tge::Core::frame_start() {
  command_manager.wait_finishing();
  swapchain.acquire_next_image(image_available_semaphores[frame_index()]);

  command_manager->reset();
  command_manager->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

  render_pass_manager.begin(*command_manager, RenderPassType::OPAQUE, frame_index());

  command_manager->bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipelines[RenderPassType::OPAQUE][0]);

  command_manager->bindVertexBuffers(0, {tmp_vert_buffer.get_buffer()}, {0});
  command_manager->bindIndexBuffer(tmp_ind_buffer.get_buffer(), 0, vk::IndexType::eUint32);

  for (uint32_t i = 0; i < num_of_triangles; i++) {
    command_manager->drawIndexed(3, 1, i * 3, i * 3, 0);
  }

  render_pass_manager.end();
}

void tge::Core::frame_end() {
  draw_final_pass();

  command_manager->end();

  command_manager.submit(
      image_available_semaphores[frame_index()],
      render_finished_semaphores[swapchain.image_index()]
  );

  command_manager.present(render_finished_semaphores[swapchain.image_index()], swapchain);
}

void tge::Core::create_final_shader(const std::string& name) {
  graphics_pipelines.try_emplace(RenderPassType::FINAL);
  graphics_pipelines[RenderPassType::FINAL].push_back(pipeline_manager.create_graphics_pipeline(
      vertex_type::NoVertices{},
      name,
      vk::PrimitiveTopology::ePointList,
      render_pass_manager.attachments_format(RenderPassType::FINAL),
      vk::CullModeFlagBits::eNone,
      false,
      false,
      false,
      false
  ));
}

void tge::Core::draw_final_pass() {
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

  for (auto& p : graphics_pipelines[RenderPassType::FINAL]) {
    command_manager->bindPipeline(vk::PipelineBindPoint::eGraphics, p);
    command_manager->draw(1, 1, 0, 0);
  }

  command_manager->endRendering();

  swapchain.swapchain_image().switch_layout(*command_manager, vk::ImageLayout::ePresentSrcKHR);
}
