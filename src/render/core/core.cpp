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
  queue_family_index(get_queue_family_index()),
  queue(create_queue()),
  allocator(create_allocator()),
  swapchain_present_mode(get_swapchain_present_mode(vsync, triple_buffer)),
  swapchain(create_swapchain()),
  swapchain_images(create_swapchain_images()),
  command_pool(create_command_pool()),
  descriptor_pool(create_descriptor_pool()) {
}

vk::raii::Instance tge::core::create_instance() {
  return context.createInstance(
    vk::InstanceCreateInfo(
      vk::InstanceCreateFlags(),
      &application_info().get(),
      layers(context).get(),
      instance_extensions(context).get()
    )
    .setPNext(
      &debug_messenger_info().get()
      .setPNext(&validation_features().get())
    )
  );
}

vk::raii::DebugUtilsMessengerEXT tge::core::create_debugger() {
  return instance.createDebugUtilsMessengerEXT(
    debug_messenger_info().get()
  );
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
  return physical_device.createDevice(
    vk::DeviceCreateInfo(
      vk::DeviceCreateFlags(),
      queue_info(physical_device, surface).get(),
      {},
      device_extensions(physical_device).get()
    )
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
  const vk::Extent2D extent = physical_device.getSurfaceCapabilitiesKHR(surface).currentExtent;

  // trick to supress useless vulkan warnings
  auto tmp = physical_device.getSurfaceFormatsKHR(surface);

  return device.createSwapchainKHR(
    swapchain_info(physical_device, surface, extent, swapchain_present_mode).get()
  );
}

std::vector<tge::image> tge::core::create_swapchain_images() {
  const std::vector<vk::Image> imgs = swapchain.getImages();
  std::vector<tge::image> res;
  res.reserve(3);

  for (const vk::Image &img : imgs) {
    res.emplace_back(allocator, device, img, vk::Format::eB8G8R8A8Unorm);
  }

  return res;
}

void tge::core::resize() {
  swapchain_images.clear();

  const vk::Extent2D extent = physical_device.getSurfaceCapabilitiesKHR(surface).currentExtent;
  swapchain = device.createSwapchainKHR(
    swapchain_info(physical_device, surface, extent, swapchain_present_mode, swapchain).get()
  );

  swapchain_images = create_swapchain_images();
}

vk::raii::CommandPool tge::core::create_command_pool() {
  return device.createCommandPool(
    vk::CommandPoolCreateInfo(
      vk::CommandPoolCreateFlags(),
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
