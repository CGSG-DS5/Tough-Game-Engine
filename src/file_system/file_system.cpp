/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

std::optional<std::vector<char>> tge::file_system::read_file(const std::string& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    return {};
  }

  std::vector<char> buffer(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

  return buffer;
}

std::optional<std::vector<char>> tge::file_system::read_resource(const std::string& filename) {
  if (std::filesystem::exists("bin")) {
    return read_file("bin/" + filename);
  } else if (std::filesystem::exists("../../../bin")) {
    return read_file("../../../bin/" + filename);
  } else {
    throw std::runtime_error("Bin directory not found");
  }
}
