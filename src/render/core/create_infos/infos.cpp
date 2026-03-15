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
    static const std::vector<float> priority;

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

std::span<const char * const> get_sdl_extensions() {
  uint32_t sdl_extensions_count = 0;
  const char * const *sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);

  if (sdl_extensions == nullptr) {
    throw tge::core_exception(SDL_GetError(), 30);
  }

  return std::span<const char * const>(sdl_extensions, sdl_extensions_count);
}

std::vector<const char *> tge::instance_extensions::get_exts(const vk::raii::Context &ctx) {
  std::span<const char * const> required_exts = get_sdl_extensions();

  /* Check extensions */
  std::vector<vk::ExtensionProperties> extension_properties = ctx.enumerateInstanceExtensionProperties();
  std::vector<const char *> extension_names;
  extension_names.reserve(extension_properties.size());

  for (const vk::ExtensionProperties &props : extension_properties) {
    extension_names.push_back(props.extensionName);
  }

  std::vector<const char *> required_unsupported = get_unsupported(required_exts, extension_names);

  // extension not supported
  if (!required_unsupported.empty())
    throw core_exception(std::string("Unsuported extension required: ") + required_unsupported.front(), 30);

  std::vector<const char *> all_extentions;
  std::vector<const char *> supported_additional = get_supported(additional_exts, extension_names);
  all_extentions.insert(all_extentions.end(), supported_additional.begin(), supported_additional.end());
  all_extentions.insert(all_extentions.end(), required_exts.begin(), required_exts.end());

#ifdef VALIDATION
    all_extentions.push_back(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);
#endif  // VALIDATION

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

/***
 * Device extensions
 ***/
std::vector<const char *> tge::device_extensions::get_exts(vk::PhysicalDevice phys_device) {
  std::vector<vk::ExtensionProperties> extension_properties = phys_device.enumerateDeviceExtensionProperties();
  std::vector<const char *> extension_names;
  extension_names.reserve(extension_properties.size());

  for (const vk::ExtensionProperties &props : extension_properties) {
    extension_names.push_back(props.extensionName);
  }

  std::vector<const char *> required_extensions {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    // VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME
  };

  std::vector<const char *> required_unsupported = get_unsupported(required_extensions, extension_names);

  // extension not supported
  if (!required_unsupported.empty())
    throw core_exception(std::string("Unsuported extension required: ") + required_unsupported.front(), 30);

  return required_extensions;
}

/***
 * Queue info
 ***/

const std::vector<float> tge::queue_info::priority {1.f};

uint32_t tge::queue_info::get_family_index(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
  std::vector<vk::QueueFamilyProperties> props = device.getQueueFamilyProperties();

  int32_t queue_family_index = -1;
  for (const vk::QueueFamilyProperties &prop : props) {
    queue_family_index++;

    if (!(prop.queueFlags & vk::QueueFlagBits::eGraphics)) continue;
    if (!(prop.queueFlags & vk::QueueFlagBits::eCompute))  continue;
    if (!(prop.queueFlags & vk::QueueFlagBits::eTransfer)) continue;

    if (!(device.getSurfaceSupportKHR(queue_family_index, surface))) continue;

    return queue_family_index;
  }

  throw core_exception("Could not find queue family with required flags", 30);
}

