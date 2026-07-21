/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_def_h_
#define __tge_def_h_

/***
 * STL Libraries
 ***/

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <vector>

/***
 * STB Image Support
 ***/
#include "third_party/stb/stb_image.h"

/***
 * SDL Support
 ***/

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

/***
 * Vulkan Support
 ***/

#include <vma/vk_mem_alloc.h>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <vulkan/vulkan_raii.hpp>

#endif // __tge_def_h_
