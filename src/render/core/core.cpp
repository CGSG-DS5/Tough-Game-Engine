/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

tge::core::core(SDL_Window *window, bool vsync, bool triple_buffer) :
  instance(create_instance()),
  debug_messenger(create_debugger()),
  physical_device(create_physical_device()),
  surface(*instance, window),
  device(create_device(window)),
  device_present_mask(device.getGroupPresentCapabilitiesKHR().presentMask[0]),
  queue_family_index(get_queue_family_index()),
  queue(create_queue()),
  allocator(create_allocator()),
  swapchain_present_mode(get_swapchain_present_mode(vsync, triple_buffer)),
  frames_in_fligt_num(swapchain_present_mode == vk::PresentModeKHR::eMailbox ? 3 : 2),  /// :TODO: Check with 2 always
  swapchain(create_swapchain()),
  swapchain_images(create_swapchain_images()),
  command_pool(create_command_pool()),
  descriptor_pool(create_descriptor_pool()),
  fences(create_fences()),
  image_available_semaphores(create_semaphores()),
  render_finished_semaphores(create_semaphores()),
  render_command_buffers(create_command_buffers()) {
}

tge::core::~core() {
  queue.waitIdle();
}

vk::raii::Instance tge::core::create_instance() {
  return context.createInstance(
    vk::InstanceCreateInfo(
      vk::InstanceCreateFlags(),
      &application_info().get(),
#ifdef VALIDATION
      layers(context).get(),
#else  // VALIDATION
      {},
#endif  // VALIDATION
      instance_extensions(context).get()
    )
#ifdef VALIDATION
    .setPNext(
      &debug_messenger_info().get()
      .setPNext(&validation_features().get())
    )
#endif  // VALIDATION
  );
}

vk::raii::DebugUtilsMessengerEXT tge::core::create_debugger() {
#ifdef VALIDATION
  return instance.createDebugUtilsMessengerEXT(
    debug_messenger_info().get()
  );
#else  // VALIDATION
  return nullptr;
#endif  // VALIDATION
}

static int64_t get_physical_device_score(const vk::raii::PhysicalDevice &device) {
  int64_t score = 0;

  vk::PhysicalDeviceProperties props = device.getProperties();

  if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
    score += 100000;
  }

  score += props.limits.maxImageDimension2D;

  return score;
}

vk::raii::PhysicalDevice tge::core::create_physical_device() {
  return std::ranges::max(instance.enumeratePhysicalDevices(), std::less(), get_physical_device_score);
}

vk::raii::Device tge::core::create_device(SDL_Window *window) {
  vk::PhysicalDeviceDynamicRenderingFeatures render_features(true);
  vk::PhysicalDeviceSynchronization2Features sync_features(true, &render_features);

  return physical_device.createDevice(
    vk::DeviceCreateInfo(
      vk::DeviceCreateFlags(),
      queue_info(physical_device, surface).get(),
      {},
      device_extensions(physical_device).get()
    )
    .setPNext(&sync_features)
  );
}

uint32_t tge::core::get_queue_family_index() {
  return queue_info(physical_device, surface).get().queueFamilyIndex;
}

vk::raii::Queue tge::core::create_queue() {
  return device.getQueue(queue_family_index, 0);
}

tge::vma_allocator tge::core::create_allocator() {
  return vma_allocator(instance, physical_device, device);
}

/***
 * Swapchain
 ***/

vk::PresentModeKHR tge::core::get_swapchain_present_mode(const bool vsync, const bool triple_buffer) {
  const std::vector<vk::PresentModeKHR> modes = physical_device.getSurfacePresentModesKHR(surface);

  bool immediate = false, mailbox = false;

  for (const vk::PresentModeKHR mode : modes) {
    switch (mode) {
    case vk::PresentModeKHR::eImmediate:
      immediate = true;
      break;

    case vk::PresentModeKHR::eMailbox:
      mailbox = true;
      break;
    }
  }

  if (!vsync) {
    return immediate ? vk::PresentModeKHR::eImmediate : vk::PresentModeKHR::eFifo;
  }

  if (triple_buffer) {
    return mailbox ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
  }

  return vk::PresentModeKHR::eFifo;
}


vk::raii::SwapchainKHR tge::core::create_swapchain() {
  screen_size = physical_device.getSurfaceCapabilitiesKHR(surface).currentExtent;

  // trick to supress useless vulkan warnings
  auto tmp = physical_device.getSurfaceFormatsKHR(surface);

  return device.createSwapchainKHR(
    swapchain_info(physical_device, surface, screen_size, swapchain_present_mode).get()
  );
}

std::vector<tge::image> tge::core::create_swapchain_images() {
  const std::vector<vk::Image> imgs = swapchain.getImages();
  std::vector<tge::image> res;
  res.reserve(frames_in_fligt_num);

  for (const vk::Image &img : imgs) {
    res.emplace_back(allocator, device, img, vk::Format::eB8G8R8A8Unorm);
  }

  return res;
}

void tge::core::resize() {
  swapchain_images.clear();

  screen_size = physical_device.getSurfaceCapabilitiesKHR(surface).currentExtent;
  swapchain = device.createSwapchainKHR(
    swapchain_info(physical_device, surface, screen_size, swapchain_present_mode, swapchain).get()
  );

  swapchain_images = create_swapchain_images();
}

vk::raii::CommandPool tge::core::create_command_pool() {
  return device.createCommandPool(
    vk::CommandPoolCreateInfo(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      queue_family_index
    )
  );
}

vk::raii::DescriptorPool tge::core::create_descriptor_pool() {
  return device.createDescriptorPool(
    vk::DescriptorPoolCreateInfo(
      vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
      1024
    )
  );
}

std::vector<vk::raii::Fence> tge::core::create_fences() {
  std::vector<vk::raii::Fence> result;
  result.reserve(frames_in_fligt_num);

  for (int32_t i = 0; i < frames_in_fligt_num; i++) {
    result.emplace_back(device.createFence(
      vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)
    ));
  }

  return result;
}

std::vector<vk::raii::Semaphore> tge::core::create_semaphores() {
  std::vector<vk::raii::Semaphore> result;
  result.reserve(frames_in_fligt_num);

  for (int32_t i = 0; i < frames_in_fligt_num; i++) {
    result.emplace_back(device.createSemaphore(
      vk::SemaphoreCreateInfo()
    ));
  }

  return result;
}

std::vector<vk::raii::CommandBuffer> tge::core::create_command_buffers() {
  return device.allocateCommandBuffers(
    vk::CommandBufferAllocateInfo(
      command_pool,
      vk::CommandBufferLevel::ePrimary,
      frames_in_fligt_num
    )
  );
}

void tge::core::frame_start() {
  if (vk::Result res = device.waitForFences(*fences[frame_index], 1, UINT64_MAX); res != vk::Result::eSuccess) {
    throw core_exception("Wait for fence error", static_cast<int32_t>(res));
  }
  device.resetFences(*fences[frame_index]);

  auto [res, new_image_index] = device.acquireNextImage2KHR(
    vk::AcquireNextImageInfoKHR(
      swapchain,
      UINT64_MAX,
      image_available_semaphores[frame_index],
      VK_NULL_HANDLE,
      device_present_mask
    )
  );

  image_index = new_image_index;
  if (res != vk::Result::eSuccess) {
    throw core_exception("Acquire next image error", static_cast<int32_t>(res));
  }

  render_command_buffers[frame_index].reset();
  render_command_buffers[frame_index].begin(
    vk::CommandBufferBeginInfo(
      vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    )
  );

  swapchain_images[image_index].switch_layout(
    render_command_buffers[frame_index], vk::ImageLayout::eColorAttachmentOptimal
  );

  vk::RenderingAttachmentInfo color_attachment(
    swapchain_images[image_index].get_image_view(),
    vk::ImageLayout::eColorAttachmentOptimal,
    vk::ResolveModeFlagBits::eNone,
    VK_NULL_HANDLE,
    vk::ImageLayout::eUndefined,
    vk::AttachmentLoadOp::eClear,
    vk::AttachmentStoreOp::eStore,
    vk::ClearColorValue(0.30f, 0.47f, 0.8f, 1.f)
  );

  vk::RenderingInfo render_info(
    vk::RenderingFlags(),
    VkRect2D {{0, 0}, screen_size},
    1, 0,
    color_attachment
  );

  render_command_buffers[frame_index].beginRendering(render_info);
}

void tge::core::frame_end() {
  render_command_buffers[frame_index].endRendering();

  swapchain_images[image_index].switch_layout(
    render_command_buffers[frame_index], vk::ImageLayout::ePresentSrcKHR
  );

  render_command_buffers[frame_index].end();

  vk::SemaphoreSubmitInfo wait_semaphore_info(
    image_available_semaphores[frame_index],
    0, vk::PipelineStageFlagBits2::eAllCommands
  );

  vk::CommandBufferSubmitInfo cmd_buf_submit_info(
    render_command_buffers[frame_index]
  );

  vk::SemaphoreSubmitInfo signal_semaphore_info(
    render_finished_semaphores[frame_index],
    0, vk::PipelineStageFlagBits2::eAllCommands
  );

  vk::SubmitInfo2 submit_info(
    vk::SubmitFlags(),
    wait_semaphore_info,
    cmd_buf_submit_info,
    signal_semaphore_info
  );
  queue.submit2(submit_info, fences[frame_index]);

  vk::PresentInfoKHR present_info(
    *render_finished_semaphores[frame_index],
    *swapchain,
    image_index
  );

  if (vk::Result res = queue.presentKHR(present_info); res != vk::Result::eSuccess) {
    throw core_exception("Error in present", static_cast<int32_t>(res));
  }

  frame_index = (frame_index + 1) % render_finished_semaphores.size();
}
