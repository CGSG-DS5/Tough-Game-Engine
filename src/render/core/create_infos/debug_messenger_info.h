/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_debug_messenger_info_
#define __tge_debug_messenger_info_

#include "info_template.h"

namespace tge {
  class DebugMessengerInfo : public InfoTemplate<vk::DebugUtilsMessengerCreateInfoEXT> {
  public:
    DebugMessengerInfo()
        : InfoTemplate(
              vk::DebugUtilsMessengerCreateFlagsEXT(),
              vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                  vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
              vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                  vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
              debug_callback
          ) {}

  private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    );
  };
} // namespace tge

#undef CURRENT_VULKAN_VERSION

#endif // __tge_debug_messenger_info_
