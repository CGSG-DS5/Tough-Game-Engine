/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __file_system_
#define __file_system_

#include "def.h"

namespace tge {
  namespace file_system {
    std::optional<std::vector<char>> read_file(const std::string& filename);
    std::optional<std::vector<char>> read_resource(const std::string& filename);
  }
} // namespace tge

#endif // __file_system_
