/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

#ifndef __tge_validation_features_
#define __tge_validation_features_

#include "def.h"

namespace tge {
  class ValidationFeatures : public InfoTemplate<vk::ValidationFeaturesEXT> {
  public:
    ValidationFeatures()
        : InfoTemplate(enabled_features) {}

  private:
    static const std::vector<vk::ValidationFeatureEnableEXT> enabled_features;
  };
} // namespace tge

#endif // __tge_validation_features_
