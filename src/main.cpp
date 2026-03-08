/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

int main() {
  try {
    tge::render Rnd(true, true);
  } catch (const tge::core_exception &err) {
    std::cerr << "Error occured while core initialization" << err.what() << "\n";

    return err.code();
  } catch (const vk::SystemError &err) {
    std::cerr << "Vulkan error occured while core initialization" << err.what() << "\n";

    return err.code().value();
  }

  return 0;
}
