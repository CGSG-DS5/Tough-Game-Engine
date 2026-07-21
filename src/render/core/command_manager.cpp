/*
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tge.h"

tge::CommandManager::CommandManager(
    vk::PhysicalDevice physical_device,
    const Device& device,
    vk::SurfaceKHR surface,
    uint32_t frames_in_flight
)
    : device(device)
    , frames_in_flight(frames_in_flight)
    , queue_family_index(get_queue_family_index(physical_device, surface))
    , queue(create_queue())
    , command_pool(create_command_pool())
    , fences(device.create_fences(frames_in_flight))
    , command_buffers(create_command_buffers(frames_in_flight)) {}

uint32_t tge::CommandManager::frame_index() const {
  return m_frame_index;
}

void tge::CommandManager::wait_idle() const {
  queue.waitIdle();
}

void tge::CommandManager::wait_finishing() const {
  if (vk::Result res = device->waitForFences(*fences[m_frame_index], 1, UINT64_MAX); res != vk::Result::eSuccess) {
    throw CoreException("Wait for fence error", static_cast<int32_t>(res));
  }

  device->resetFences(*fences[m_frame_index]);
}

const vk::CommandBuffer& tge::CommandManager::operator*() const {
  return *command_buffers[m_frame_index];
}

const vk::CommandBuffer* tge::CommandManager::operator->() const {
  return &*command_buffers[m_frame_index];
}

uint32_t tge::CommandManager::get_queue_family_index(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface) {
  return QueueInfo(physical_device, surface).get().queueFamilyIndex;
}

vk::raii::Queue tge::CommandManager::create_queue() {
  return device->getQueue(queue_family_index, 0);
}

vk::raii::CommandPool tge::CommandManager::create_command_pool() {
  return device->createCommandPool(
      {.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = queue_family_index}
  );
}

std::vector<vk::raii::CommandBuffer> tge::CommandManager::create_command_buffers(uint32_t num) const {
  return device->allocateCommandBuffers(vk::CommandBufferAllocateInfo{
      .commandPool = command_pool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = num
  });
}

void tge::CommandManager::submit(
    vk::Semaphore image_available_semaphore,
    vk::Semaphore render_finished_semaphore
) const {
  vk::SemaphoreSubmitInfo wait_semaphore_info{
      .semaphore = image_available_semaphore,
      .stageMask = vk::PipelineStageFlagBits2::eAllCommands
  };

  vk::CommandBufferSubmitInfo cmd_buf_submit_info{.commandBuffer = *command_buffers[frame_index()]};

  vk::SemaphoreSubmitInfo signal_semaphore_info{
      .semaphore = render_finished_semaphore,
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
  queue.submit2(submit_info, fences[frame_index()]);
}

void tge::CommandManager::present(vk::Semaphore render_finished_semaphore, const Swapchain& swapchain) const {
  auto present_swp = swapchain.swapchain();
  auto present_ind = swapchain.image_index();
  vk::PresentInfoKHR present_info{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &render_finished_semaphore,
      .swapchainCount = 1,
      .pSwapchains = &present_swp,
      .pImageIndices = &present_ind
  };

  if (vk::Result res = queue.presentKHR(present_info); res != vk::Result::eSuccess) {
    throw CoreException("Error in present", static_cast<int32_t>(res));
  }
}

vk::raii::CommandBuffer tge::CommandManager::begin_single_commands() const {
  vk::raii::CommandBuffer commandBuffer = std::move(create_command_buffers(1)[0]);

  vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  commandBuffer.begin(beginInfo);

  return commandBuffer;
}

void tge::CommandManager::end_single_commands(vk::raii::CommandBuffer&& cmd_buf) const {
  cmd_buf.end();

  vk::SubmitInfo submitInfo{.commandBufferCount = 1, .pCommandBuffers = &*cmd_buf};
  queue.submit(submitInfo, nullptr);
  queue.waitIdle();
}
