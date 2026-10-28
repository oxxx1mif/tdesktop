/*
 * This file is part of ligros,
 * the fork desktop application for the Telegram messaging service.
 *
 * For license and copyright information please follow this link:
 * https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
 * Copyright Gleb Obitotsky 2026.
 */

#include "lig/badges/lbadges_manager.h"

#include <optional>

namespace ligros {
namespace config {
namespace badges {

CustomBadgesManager& CustomBadgesManager::instance() {
  static CustomBadgesManager manager;
  return manager;
}

std::optional<CustomBadgesData> CustomBadgesManager::getBadge(
    uint64 userId) const {
  auto it = _badges.find(userId);
  if (it != _badges.end()) {
    return it->second;
  }
  return std::nullopt;
}

}  // namespace badges
}  // namespace config
}  // namespace ligros