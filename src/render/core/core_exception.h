/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_core_excetpion_
#define __tge_core_excetpion_

#include "def.h"

namespace tge {
  class core_exception : public std::exception {
  public:
    core_exception(const std::string &msg, const int code) noexcept :
      message(msg), error_code(code)  {
    }

    const char * what() const noexcept override {
      return message.c_str();
    }

    int code() const noexcept {
      return error_code;
    }

  private:
    const std::string message;
    const int error_code;
  };
}

#endif  // __tge_core_excetpion_
