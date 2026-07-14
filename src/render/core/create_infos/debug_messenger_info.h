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
              {.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                  vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                  vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                  vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
               .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                              vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                              vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
               .pfnUserCallback = debug_callback}
          ) {}

  private:
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );
  };
} // namespace tge

#undef CURRENT_VULKAN_VERSION

#endif // __tge_debug_messenger_info_
