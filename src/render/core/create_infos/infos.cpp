/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

/***
 * Common functions
 ***/

std::vector<const char *> tge::get_supported(
  const std::span<const char * const> &required,
  const std::span<const char * const> &supported
) {
  std::vector<const char *> result;

  for (const char * const extension_name : required) {
    if (
        std::ranges::any_of(supported,
          [extension_name](const char * const name) -> bool {
            return strcmp(extension_name, name) == 0;
          })
      ) {
      result.push_back(extension_name);
    }
  }

  return result;
}

std::vector<const char *> tge::get_unsupported(
  const std::span<const char * const> &required,
  const std::span<const char * const> &supported
) {
  std::vector<const char *> result;

  for (const char * const extension_name : required) {
    if (
        std::ranges::none_of(supported,
          [extension_name](const char * const name) -> bool {
            return strcmp(extension_name, name) == 0;
          })
      ) {
      result.push_back(extension_name);
    }
  }

  return result;
}

/***
 * Instance extensions
 ***/

const std::vector<const char *> tge::instance_extensions::additional_exts {
  VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

std::vector<const char *> tge::instance_extensions::get_exts(
      std::span<const char * const> required_exts,
      const vk::raii::Context &ctx) {
  std::vector<const char *> all_extentions;
  all_extentions.reserve(required_exts.size());

  /***
   * Check extensions
   ***/
  std::vector<vk::ExtensionProperties> extension_properties = ctx.enumerateInstanceExtensionProperties();
  std::vector<const char *> extension_names;
  extension_names.reserve(extension_properties.size());

  for (const vk::ExtensionProperties &props : extension_properties) {
    extension_names.push_back(props.extensionName);
  }

  std::vector<const char *> required_unsupported = get_unsupported(required_exts, extension_names);

  // extension not supported
  if (!required_unsupported.empty())
    throw new core_exception(std::string("Unsuported extension required: ") + required_unsupported.front(), 30);

  std::vector<const char *> supported_additional = get_supported(additional_exts, extension_names);
  all_extentions.insert(all_extentions.end(), supported_additional.begin(), supported_additional.end());

  if (!layers(ctx).get().empty()) {
    all_extentions.push_back(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);
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

/***
 * Layers
 ***/

std::vector<const char *> tge::layers::get_layers(const vk::raii::Context &ctx) {
  std::vector<const char *> required {
    "VK_LAYER_KHRONOS_validation"
  };

  std::vector<vk::LayerProperties> layer_properties = ctx.enumerateInstanceLayerProperties();
  std::vector<const char *> layer_names;
  layer_names.reserve(layer_properties.size());

  for (const vk::LayerProperties &props : layer_properties) {
    layer_names.push_back(props.layerName);
  }

  return get_supported(required, layer_names);
}
