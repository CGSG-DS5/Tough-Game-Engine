/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

/***
 * Instance extensions
 ***/

const std::vector<const char *> tge::instance_extensions::additional_exts {
  VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

std::vector<const char *> tge::instance_extensions::getExts(
      std::span<const char * const> required_exts,
      const vk::raii::Context &ctx) {
  std::vector<const char *> all_extentions;
  all_extentions.reserve(required_exts.size() + additional_exts.size());

  all_extentions.insert(all_extentions.end(), additional_exts.begin(), additional_exts.end());
  all_extentions.insert(all_extentions.end(), required_exts.begin(), required_exts.end());

  /***
   * Check extensions
   ***/
  std::vector<vk::ExtensionProperties> extension_properties = ctx.enumerateInstanceExtensionProperties();

  for (const char * const extension_name : all_extentions) {
    if (
        std::ranges::none_of(extension_properties,
          [&extension_name](const vk::ExtensionProperties &props) -> bool {
              return strcmp(props.extensionName, extension_name) == 0;
            }
        )
      ) {
      // extension not supported
      throw new core_exception(std::string("Unsuported extension required: ") + extension_name, 30);
    }
  }

  return all_extentions;
}

/***
 * Debug messenger
 ***/

enum class console_rgb {
  red,
  green,
  yellow,
  blue,
  white
};

static void set_console_color(const console_rgb rgb) {
  switch (rgb)
  {
  case console_rgb::red:
    std::cout << "\x1b[31m";
    break;
  case console_rgb::green:
    std::cout << "\x1b[32m";
    break;
  case console_rgb::yellow:
    std::cout << "\x1b[33m";
    break;
  case console_rgb::blue:
    std::cout << "\x1b[34m";
    break;
  case console_rgb::white:
    std::cout << "\x1b[0m";
    break;
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL tge::debug_messenger_info::debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
  const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
  void*                                            pUserData
) {

  std::string msg;

  switch (messageTypes) {
  case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
    msg += "[GENERAL";
    break;
  case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
    msg += "[PERFORMANCE";
    break;
  case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
    msg += "[VALIDATION";
    break;
  }

  switch (messageSeverity) {
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    set_console_color(console_rgb::blue);
    msg += " | VERBOSE] ";
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    set_console_color(console_rgb::blue);
    msg += " | INFO] ";
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    set_console_color(console_rgb::yellow);
    msg += " | WARNING] ";
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    set_console_color(console_rgb::red);
    msg += " | ERROR] ";
    break;
  }

  std::cerr << msg << pCallbackData->pMessage << "\n";

  set_console_color(console_rgb::white);

  return VK_FALSE;
}

/***
 * Validation features
 ***/

const std::vector<vk::ValidationFeatureEnableEXT> tge::validation_features::enabled_features {
  vk::ValidationFeatureEnableEXT::eGpuAssisted,
  vk::ValidationFeatureEnableEXT::eBestPractices,
  vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
  //vk::ValidationFeatureEnableEXT::eDebugPrintf,  // debugPrintfEXT in shaders
};
