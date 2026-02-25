/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#include "tge.h"

int main() {
  tge::window_handler window("DD1 Window", 800, 600);

  try {
    tge::core Rnd(tge::window_handler::get_sdl_extensions());
  } catch (const tge::core_exception &err) {
    std::cerr << "Error occured while core initialization" << err.what() << "\n";

    return err.code();
  } catch (const vk::SystemError &err) {
    std::cerr << "Vulkan error occured while core initialization" << err.what() << "\n";

    return err.code().value();
  }

  window.run();

  return 0;
}
