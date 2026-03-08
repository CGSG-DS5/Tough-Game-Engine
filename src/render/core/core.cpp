/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

tge::core::core(SDL_Window *window, bool vsync, bool triple_buffer) :
    instance(create_instance()),
    debug_messenger(create_debugger()),
    physical_device(create_physical_device()),
    device(create_device(window)),
    queue(create_queue()),
    allocator(create_allocator())
    //swa
    //swapchain(create_swapchain())
{
}

tge::core::~core() {
  SDL_Vulkan_DestroySurface(*instance, static_cast<VkSurfaceKHR>(surface), nullptr);
}

vk::raii::Instance tge::core::create_instance() {
  return context.createInstance(
    vk::InstanceCreateInfo(
      vk::InstanceCreateFlags(),
      &application_info().get(),
      layers(context).get(),
      instance_extensions(context).get()
    )
    .setPNext(&debug_messenger_info().get()

      .setPNext(&validation_features().get()))
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

void tge::core::create_suface(SDL_Window *window) {
  SDL_Vulkan_CreateSurface(window, *instance, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface));
}

vk::raii::Device tge::core::create_device(SDL_Window *window) {
  create_suface(window);

  return physical_device.createDevice(
    vk::DeviceCreateInfo(
      vk::DeviceCreateFlags(),
      queue_info(physical_device, surface).get(),
      {},
      device_extensions(physical_device).get()
    )
  );
}

vk::raii::Queue tge::core::create_queue() {
  return device.getQueue(queue_info(physical_device, surface).get().queueFamilyIndex, 0);
}

tge::vma_allocator tge::core::create_allocator() {
  return vma_allocator(instance, physical_device, device);
}

//vk::raii::SwapchainKHR tge::core::create_swapchain() {
//  device.createSwapchainKHR(
//    swapchain_info(physical_device, surface, 3, 0, 0).get()
//  );
//}
