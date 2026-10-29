/*
 * This file is part of ligros,
 * the fork desktop application for the Telegram messaging service.
 *
 * For license and copyright information please follow this link:
 * https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
 * Copyright Gleb Obitotsky 2026.
 */

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include <optional>
#include <unordered_map>

#include "rpl/event_stream.h"

namespace ligros {
namespace config {
namespace badges {

struct CustomBadgesData {
    QString badge_emojiId;
    QString click_message;
};

enum class CustomBadgesError : uint8 {
    None = 0,
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
    
    void startFetching();

    [[nodiscard]] rpl::producer<CustomBadgesError> fetchFinished() const {
        return _fetchFinished.events();
    }

private:
    CustomBadgesManager();
    void fetchFromNetwork();

    std::unordered_map<uint64, CustomBadgesData> _badges;
    QNetworkAccessManager _networkManager;
    rpl::event_stream<CustomBadgesError> _fetchFinished;
    bool _isFetching = false;
};

} // namespace badges
} // namespace config
} // namespace ligros