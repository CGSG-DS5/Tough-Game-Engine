/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_core_excetpion_
#define __tge_core_excetpion_

#include "def.h"

namespace tge {
  class CoreException : public std::exception {
  public:
    CoreException(const std::string &msg, const int32_t code) noexcept
        : message(msg)
        , error_code(code) {}

    const char *what() const noexcept override {
      return message.c_str();
    }

    int code() const noexcept {
      return error_code;
    }

  private:
    const std::string message;
    const int error_code;
  };
} // namespace tge

#endif // __tge_core_excetpion_
