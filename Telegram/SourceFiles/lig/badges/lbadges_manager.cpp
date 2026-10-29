/*
 * This file is part of ligros,
 * the fork desktop application for the Telegram messaging service.
 *
 * For license and copyright information please follow this link:
 * https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
 * Copyright Gleb Obitotsky 2026.
 */

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>

#include "lig/badges/lbadges_manager.h"
#include "crl/crl.h"

namespace ligros {
namespace config {
namespace badges {

CustomBadgesManager& CustomBadgesManager::instance() {
    static CustomBadgesManager manager;
    return manager;
}

CustomBadgesManager::CustomBadgesManager() {
}

std::optional<CustomBadgesData> CustomBadgesManager::getBadge(uint64 userId) const {
    auto it = _badges.find(userId);
    if (it != _badges.end()) {
        return it->second;
    }
    return std::nullopt;
}

void CustomBadgesManager::startFetching() {
    fetchFromNetwork();
}

void CustomBadgesManager::fetchFromNetwork() {
    QNetworkRequest request(QUrl("https://raw.githubusercontent.com/oxxx1mif/tbadges/refs/heads/main/lbadges_users.json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "ligros/1.0");

    QNetworkReply *reply = _networkManager.get(request);

    connect(reply, &QNetworkReply::finished, reply, [=, this]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "LigBadges: Network error:" << reply->errorString();
            _fetchFinished.fire(CustomBadgesError::NetworkError);
            return;
        }

        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode != 200) {
            qWarning() << "LigBadges: HTTP error, status:" << statusCode;
            _fetchFinished.fire(CustomBadgesError::HttpError);
            return;
        }

        QByteArray data = reply->readAll();

        crl::async([=, this, data = std::move(data)]() mutable {
            
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

            if (parseError.error != QJsonParseError::NoError) {
                crl::on_main(this, [=, this] {
                    _fetchFinished.fire(CustomBadgesError::JsonParseError);
                });
                return;
            }

            if (!doc.isObject() || !doc.object().value("custom_badges_users").isArray()) {
                crl::on_main(this, [=, this] {
                    _fetchFinished.fire(CustomBadgesError::InvalidStructure);
                });
                return;
            }

            QJsonArray users = doc.object().value("custom_badges_users").toArray();
            std::unordered_map<uint64, CustomBadgesData> parsedBadges;
            parsedBadges.reserve(users.size());

            for (const QJsonValue &val : users) {
                if (!val.isObject()) continue;
                QJsonObject obj = val.toObject();
                
                if (!obj.contains("userId")) continue;
                if (!obj.contains("badge_emojiId") || !obj["badge_emojiId"].isString()) continue;
                
                quint64 userId = 0;
                if (obj["userId"].isString()) {
                    bool ok = false;
                    userId = obj["userId"].toString().toULongLong(&ok);
                    if (!ok) continue;
                } else if (obj["userId"].isDouble()) {
                    userId = obj["userId"].toVariant().toULongLong();
                } else {
                    continue;
                }
                
                QString emojiId = obj["badge_emojiId"].toString();
                QString message = obj["click_message"].toString();
                
                if (userId != 0 && !emojiId.isEmpty()) {
                    parsedBadges[userId] = { emojiId, message };
                }
            }
            
            crl::on_main(this, [=, this, parsed = std::move(parsedBadges)]() mutable {
                if (parsed.empty()) {
                    _fetchFinished.fire(CustomBadgesError::EmptyData);
                } else {
                    _badges = std::move(parsed);
                    qDebug() << "LigBadges: Successfully loaded" << _badges.size() << "badges.";
                    _fetchFinished.fire(CustomBadgesError::None);
                }
            });
        });
    });
}

} // namespace badges
} // namespace config
} // namespace ligros
