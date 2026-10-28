/*
 * This file is part of ligros,
 * the fork desktop application for the Telegram messaging service.
 *
 * For license and copyright information please follow this link:
 * https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
 * Copyright Gleb Obitotsky 2026.
 */

#pragma once

#include <qobject.h>
#include <qtmetamacros.h>

#include <QJsonArray>
#include <QObject>
#include <QString>
#include <QTimer>
#include <optional>
#include <tl/expected.hpp>
#include <unordered_map>

namespace ligros {
namespace config {
namespace badges {

struct CustomBadgesData {
  QString badge_emojiId;
  QString click_message;
};

enum class CustomBadgesError : uint8 {
  NetworkError,
  HttpError,
  JsonParseError,
  InvalidStructure,
  EmptyData
};

class CustomBadgesManager : public QObject {
  Q_OBJECT
 public:
  static CustomBadgesManager& instance();

  std::optional<CustomBadgesData> getBadge(uint64 userId) const;
  tl::expected<CustomBadgesError, void> startFetching();

 private:
  CustomBadgesManager() = default;
  tl::expected<CustomBadgesError, void> fetchFromNetwork();

  std::unordered_map<uint64, CustomBadgesData> _badges;
  QTimer _updateTimer;
};

}  // namespace badges
}  // namespace config
}  // namespace ligros