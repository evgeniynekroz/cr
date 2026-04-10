#include <Geode/Geode.hpp>

#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/InfoLayer.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/MenuLayer.hpp>

#include <Geode/modify/InfoLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include <network/HttpClient.h>
#include <network/HttpRequest.h>
#include <network/HttpResponse.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

using namespace geode::prelude;

namespace cr {
    static constexpr char const* kTursoUrlRaw = "libsql://custom-rates-evgen.aws-eu-west-1.turso.io";

    static constexpr std::array<uint8_t, 10> kXorKey = {
        0xb1, 0x9a, 0x94, 0x8d, 0x90,
        0x85, 0xcd, 0xcf, 0xcd, 0xc9
    };

    static constexpr uint8_t kEncryptedToken[] = {
        0xd4, 0xe3, 0xde, 0xe5, 0xf2, 0xc2, 0xae, 0xa6, 0x82, 0xa0, 0xfb, 0xdc, 0xce, 0xc8, 0xc2, 0xd1,
        0x9c, 0x9c, 0x84, 0xba, 0xf8, 0xf4, 0xc6, 0xb8, 0xf3, 0xc6, 0x84, 0xf9, 0x84, 0xa2, 0xc1, 0xc2,
        0xc2, 0xce, 0xda, 0xbc, 0xe3, 0xaa, 0xb4, 0x83, 0xd9, 0xd3, 0xfe, 0xe2, 0xf9, 0xe6, 0xa3, 0xac,
        0xa4, 0x85, 0xf2, 0xd0, 0xe4, 0xd4, 0xc8, 0xd4, 0xa4, 0x80, 0xa7, 0x8c, 0x82, 0xd4, 0xee, 0xd8,
        0xa4, 0xc8, 0x99, 0xa4, 0xfd, 0x87, 0xe5, 0xfd, 0xe7, 0xc4, 0xfd, 0xe9, 0xa6, 0x86, 0xa7, 0xa6,
        0xd8, 0xd7, 0xd0, 0xc8, 0xa5, 0xdf, 0x89, 0x96, 0xf8, 0x93, 0xf6, 0xd3, 0xe0, 0xc0, 0xd4, 0xe6,
        0xba, 0x82, 0x9e, 0xf9, 0x82, 0xd4, 0xfe, 0xdb, 0xfb, 0xc9, 0x99, 0xa4, 0xb7, 0x90, 0xe5, 0xc3,
        0xe0, 0xc3, 0xc4, 0xd0, 0xb7, 0x95, 0x9a, 0x93, 0xdc, 0xc3, 0xc0, 0xc3, 0xfb, 0xdc, 0xa0, 0x99,
        0xa1, 0x80, 0xd8, 0xed, 0xfd, 0xee, 0xfd, 0xe9, 0xa6, 0x86, 0xa7, 0xa6, 0xd8, 0xd5, 0xc3, 0xdb,
        0xfd, 0xc8, 0xff, 0x85, 0xa4, 0x87, 0xdc, 0xcf, 0xe0, 0xd4, 0xfa, 0xcc, 0xff, 0x82, 0x9e, 0xf9,
        0x81, 0xd4, 0xc0, 0xcc, 0xa5, 0xc9, 0x9a, 0x89, 0xa7, 0x93, 0xf5, 0xf1, 0xe0, 0xc2, 0xc4, 0xed,
        0xa7, 0x82, 0x9a, 0x90, 0xc6, 0xc0, 0xc0, 0xdc, 0xa4, 0xcb, 0xa7, 0x85, 0xa5, 0x80, 0xdf, 0xaa,
        0xba, 0xfd, 0xc2, 0xcc, 0xbc, 0xbc, 0x81, 0xbd, 0xd3, 0xe0, 0xa3, 0xf4, 0xfb, 0xf3, 0x81, 0x85,
        0xac, 0xf1, 0xf0, 0xe2, 0xcb, 0xbf, 0xa1, 0xc9, 0xa3, 0xe2, 0xff, 0x8f, 0xfb, 0xf3, 0xde, 0xa0,
        0xdf, 0xc4, 0xfd, 0xbe, 0xfe, 0xe4, 0x89, 0xe0, 0xac, 0xcf, 0xe8, 0xfd, 0x80, 0x99, 0xf8, 0xa5,
        0xc2, 0xd8, 0xd2, 0xf8, 0xd9, 0xf5, 0x8e, 0xa0, 0xf9, 0xa4, 0xc1, 0xc8, 0xa1, 0xd7, 0xd8, 0xbc,
        0x9b, 0xf7, 0xa0, 0x84, 0xe7, 0xa2, 0xe0, 0xd7, 0xa5, 0xee, 0xb4, 0x96, 0x98, 0xe4, 0xfe, 0xe0,
        0xfb, 0xd4, 0xc7, 0xb2, 0xfe, 0x8d, 0xaa
    };

    static constexpr int kRowsPerPage = 8;

    enum class Tab {
        Sent = 0,
        Recent = 1,
        Top = 2,
    };

    struct LevelEntry {
        int64_t levelID = 0;
        int blueStars = 0;
        std::string moderator;
        std::string sentBy;
        int64_t sentAt = 0;
        int64_t ratedAt = 0;
    };

    static bool g_bootstrapped = false;
    static std::string g_tursoUrl;
    static std::string g_tursoToken;

    static std::unordered_set<std::string> g_admins = {
        "mapperok232",
        "kiyarikus",
        "nenekroz",
        "ujneft",
    };

    static std::string lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return s;
    }

    static std::string escapeSql(std::string const& in) {
        std::string out;
        out.reserve(in.size() + 16);
        for (char c : in) {
            if (c == '\'') out += "''";
            else out += c;
        }
        return out;
    }

    static std::string escapeJson(std::string const& in) {
        std::string out;
        out.reserve(in.size() + 16);
        for (char c : in) {
            switch (c) {
                case '\\': out += "\\\\"; break;
                case '"':  out += "\\\""; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:   out += c; break;
            }
        }
        return out;
    }

    static std::string normalizeUrl(std::string url) {
        while (!url.empty() && std::isspace((unsigned char)url.back())) url.pop_back();
        while (!url.empty() && std::isspace((unsigned char)url.front())) url.erase(url.begin());

        if (url.rfind("libsql://", 0) == 0) {
            url = "https://" + url.substr(std::string("libsql://").size());
        } else if (url.rfind("http://", 0) != 0 && url.rfind("https://", 0) != 0) {
            url = "https://" + url;
        }

        while (!url.empty() && url.back() == '/') url.pop_back();
        return url;
    }

    static std::string decryptToken() {
        std::string out;
        out.reserve(std::size(kEncryptedToken));
        for (size_t i = 0; i < std::size(kEncryptedToken); ++i) {
            out.push_back(static_cast<char>(kEncryptedToken[i] ^ kXorKey[i % kXorKey.size()]));
        }
        return out;
    }

    static std::string currentPlayerName() {
        auto gm = GameManager::sharedState();
        if (!gm) return "";
        return gm->m_playerName;
    }

    static bool isAdmin() {
        auto name = lower(currentPlayerName());
        return g_admins.find(name) != g_admins.end();
    }

    static void toast(std::string const& message, NotificationIcon icon = NotificationIcon::Success) {
        auto text = std::string("Custom Rates: ") + message;
        Notification::create(text.c_str(), icon)->show();
    }

    static void bootstrap();

    static std::string pipelineUrl() {
        return g_tursoUrl + "/v2/pipeline";
    }

    static std::string makePipelineBody(std::string const& sql) {
        return std::string("{\"requests\":[{\"type\":\"execute\",\"stmt\":{\"sql\":\"")
            + escapeJson(sql) +
            "\"}}]}";
    }

    static std::optional<std::string> extractDataString(matjson::Value const& root) {
        auto resultsRes = root["results"].asArray();
        if (!resultsRes) return std::nullopt;

        auto const& results = *resultsRes;
        if (results.empty()) return std::nullopt;

        auto const& first = results.front();
        if (first["error"].isObject()) return std::nullopt;

        auto rowsRes = first["response"]["result"]["rows"].asArray();
        if (!rowsRes) return std::nullopt;

        auto const& rows = *rowsRes;
        if (rows.empty()) return std::nullopt;

        auto const& row = rows.front();
        auto rowRes = row.asArray();
        if (!rowRes) return std::nullopt;

        auto const& cols = *rowRes;
        if (cols.empty()) return std::nullopt;

        if (cols.front().isString()) {
            return cols.front().asString().unwrapOr("");
        }

        return std::nullopt;
    }

    static std::vector<LevelEntry> parseLevelsJson(std::string const& jsonStr) {
        std::vector<LevelEntry> out;
        auto parsed = matjson::parse(jsonStr).unwrapOr(matjson::Value());

        if (!parsed.isArray()) return out;

        auto arrRes = parsed.asArray();
        if (!arrRes) return out;

        auto const& arr = *arrRes;
        for (auto const& item : arr) {
            LevelEntry e;
            e.levelID = item["level_id"].asInt().unwrapOr(0);
            e.blueStars = item["blue_stars"].asInt().unwrapOr(0);
            e.moderator = item["moderator"].asString().unwrapOr("");
            e.sentBy = item["sent_by"].asString().unwrapOr("");
            e.sentAt = item["sent_at"].asInt().unwrapOr(0);
            e.ratedAt = item["rated_at"].asInt().unwrapOr(0);

            if (e.levelID > 0) {
                out.push_back(std::move(e));
            }
        }

        return out;
    }

    static std::optional<LevelEntry> parseOneLevelJson(std::string const& jsonStr) {
        auto parsed = matjson::parse(jsonStr).unwrapOr(matjson::Value());
        if (!parsed.isObject()) return std::nullopt;

        LevelEntry e;
        e.levelID = parsed["level_id"].asInt().unwrapOr(0);
        e.blueStars = parsed["blue_stars"].asInt().unwrapOr(0);
        e.moderator = parsed["moderator"].asString().unwrapOr("");
        e.sentBy = parsed["sent_by"].asString().unwrapOr("");
        e.sentAt = parsed["sent_at"].asInt().unwrapOr(0);
        e.ratedAt = parsed["rated_at"].asInt().unwrapOr(0);

        if (e.levelID <= 0) return std::nullopt;
        return e;
    }

    static void runSql(
        std::string const& sql,
        std::function<void(matjson::Value const&)> onOk,
        std::function<void(std::string const&)> onErr = nullptr
    ) {
        if (!g_bootstrapped) bootstrap();

        auto request = new network::HttpRequest();
        request->setUrl(pipelineUrl().c_str());
        request->setRequestType(network::HttpRequest::Type::POST);

        std::vector<std::string> headers;
        headers.emplace_back(std::string("Authorization: Bearer ") + g_tursoToken);
        headers.emplace_back("Content-Type: application/json");
        request->setHeaders(headers);

        auto body = makePipelineBody(sql);
        request->setRequestData(body.c_str(), body.size());

        request->setResponseCallback([onOk, onErr](network::HttpClient*, network::HttpResponse* response) {
            if (!response) {
                if (onErr) onErr("Request failed");
                return;
            }

            if (!response->isSucceed()) {
                if (onErr) {
                    std::string msg = "Request failed";
                    auto err = response->getErrorBuffer();
                    if (err && *err) {
                        msg = err;
                    }
                    onErr(msg);
                }
                return;
            }

            std::string text;
            auto data = response->getResponseData();
            if (data && !data->empty()) {
                text.assign(data->begin(), data->end());
            }

            auto parsed = matjson::parse(text).unwrapOr(matjson::Value());
            onOk(parsed);
        });

        network::HttpClient::getInstance()->send(request);
        request->release();
    }

    static std::string sqlSentList() {
        return R"SQL(
            SELECT COALESCE(
                json_group_array(
                    json_object(
                        'level_id', level_id,
                        'sent_by', sent_by,
                        'sent_at', sent_at
                    )
                ),
                '[]'
            ) AS data
            FROM (
                SELECT level_id, sent_by, sent_at
                FROM sent_levels
                ORDER BY sent_at DESC
                LIMIT 100
            );
        )SQL";
    }

    static std::string sqlRecentList() {
        return R"SQL(
            SELECT COALESCE(
                json_group_array(
                    json_object(
                        'level_id', level_id,
                        'blue_stars', blue_stars,
                        'moderator', moderator,
                        'rated_at', rated_at
                    )
                ),
                '[]'
            ) AS data
            FROM (
                SELECT level_id, blue_stars, moderator, rated_at
                FROM rated_levels
                ORDER BY rated_at DESC
                LIMIT 100
            );
        )SQL";
    }

    static std::string sqlTopList() {
        return R"SQL(
            SELECT COALESCE(
                json_group_array(
                    json_object(
                        'level_id', level_id,
                        'blue_stars', blue_stars,
                        'moderator', moderator,
                        'rated_at', rated_at
                    )
                ),
                '[]'
            ) AS data
            FROM (
                SELECT level_id, blue_stars, moderator, rated_at
                FROM rated_levels
                ORDER BY blue_stars DESC, rated_at DESC
                LIMIT 100
            );
        )SQL";
    }

    static std::string sqlRatedMeta(int64_t levelID) {
        return "SELECT json_object("
               "'level_id', level_id, "
               "'blue_stars', blue_stars, "
               "'moderator', moderator, "
               "'sent_by', '', "
               "'sent_at', 0, "
               "'rated_at', rated_at"
               ") AS data "
               "FROM rated_levels "
               "WHERE level_id = " + std::to_string(levelID) + " "
               "LIMIT 1;";
    }

    static std::string sqlSendLevel(int64_t levelID, std::string const& sentBy) {
        return "INSERT OR IGNORE INTO sent_levels (level_id, sent_by, sent_at) VALUES ("
            + std::to_string(levelID) + ", '"
            + escapeSql(sentBy) + "', unixepoch());";
    }

    static std::string sqlDeleteSent(int64_t levelID) {
        return "DELETE FROM sent_levels WHERE level_id = " + std::to_string(levelID) + ";";
    }

    static void loadAdminsFromDb() {
        runSql(
            "SELECT COALESCE(json_group_array(username), '[]') AS data "
            "FROM (SELECT username FROM admins ORDER BY username ASC);",
            [](matjson::Value const& root) {
                auto data = extractDataString(root);
                if (!data) return;

                auto parsed = matjson::parse(*data).unwrapOr(matjson::Value());
                if (!parsed.isArray()) return;

                auto arrRes = parsed.asArray();
                if (!arrRes) return;

                auto const& arr = *arrRes;
                for (auto const& item : arr) {
                    if (item.isString()) {
                        g_admins.insert(lower(item.asString().unwrapOr("")));
                    }
                }
            },
            [](std::string const& err) {
                log::warn("Admin cache load failed: {}", err);
            }
        );
    }

    static void bootstrap() {
        if (g_bootstrapped) return;
        g_bootstrapped = true;

        g_tursoUrl = normalizeUrl(kTursoUrlRaw);
        g_tursoToken = decryptToken();

        loadAdminsFromDb();
    }

    static int64_t getLevelID(GJGameLevel* level) {
        if (!level) return -1;
        return level->m_levelID;
    }

    static CCSprite* makeBlueStarSprite(float scale = 1.f) {
        auto spr = CCSprite::create("blue_star.png");
        if (!spr) {
            spr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        }
        if (spr) {
            spr->setScale(scale);
        }
        return spr;
    }

    static void fetchLevels(
        Tab tab,
        std::function<void(std::vector<LevelEntry>)> onOk,
        std::function<void(std::string const&)> onErr = nullptr
    ) {
        std::string sql;
        switch (tab) {
            case Tab::Sent:   sql = sqlSentList(); break;
            case Tab::Recent: sql = sqlRecentList(); break;
            case Tab::Top:    sql = sqlTopList(); break;
        }

        runSql(
            sql,
            [onOk, onErr](matjson::Value const& root) {
                auto data = extractDataString(root);
                if (!data) {
                    if (onErr) onErr("Invalid DB response");
                    else onOk({});
                    return;
                }

                onOk(parseLevelsJson(*data));
            },
            [onErr, onOk](std::string const& err) {
                if (onErr) onErr(err);
                else onOk({});
            }
        );
    }

    static void fetchRatedMeta(
        int64_t levelID,
        std::function<void(std::optional<LevelEntry>)> onOk,
        std::function<void(std::string const&)> onErr = nullptr
    ) {
        runSql(
            sqlRatedMeta(levelID),
            [onOk](matjson::Value const& root) {
                auto data = extractDataString(root);
                if (!data) {
                    onOk(std::nullopt);
                    return;
                }

                onOk(parseOneLevelJson(*data));
            },
            [onErr, onOk](std::string const& err) {
                if (onErr) onErr(err);
                else onOk(std::nullopt);
            }
        );
    }

    static void sendLevelToDb(
        int64_t levelID,
        std::string const& sender,
        std::function<void(bool, std::string const&)> onDone
    ) {
        runSql(
            sqlSendLevel(levelID, sender),
            [onDone](matjson::Value const&) {
                onDone(true, "Level sent to admins!");
            },
            [onDone](std::string const& err) {
                onDone(false, err);
            }
        );
    }

    static void deleteSentFromDb(
        int64_t levelID,
        std::function<void(bool, std::string const&)> onDone
    ) {
        runSql(
            sqlDeleteSent(levelID),
            [onDone](matjson::Value const&) {
                onDone(true, "Deleted from Sent.");
            },
            [onDone](std::string const& err) {
                onDone(false, err);
            }
        );
    }

    static void rateLevelInDb(
        int64_t levelID,
        int blueStars,
        std::string const& moderator,
        std::function<void(bool, std::string const&)> onDone
    ) {
        runSql(
            sqlDeleteSent(levelID),
            [levelID, blueStars, moderator, onDone](matjson::Value const&) {
                std::string sql =
                    "INSERT OR REPLACE INTO rated_levels (level_id, blue_stars, moderator, rated_at) VALUES ("
                    + std::to_string(levelID) + ", "
                    + std::to_string(blueStars) + ", '"
                    + escapeSql(moderator) + "', unixepoch());";

                runSql(
                    sql,
                    [onDone](matjson::Value const&) {
                        onDone(true, "Level rated!");
                    },
                    [onDone](std::string const& err) {
                        onDone(false, err);
                    }
                );
            },
            [onDone](std::string const& err) {
                onDone(false, err);
            }
        );
    }

    class CustomRatesLayer : public CCLayerColor {
    private:
        Tab m_tab = Tab::Sent;
        int m_page = 0;
        int m_requestSeq = 0;
        std::vector<LevelEntry> m_entries;

        CCLayerColor* m_panel = nullptr;
        CCNode* m_rowsRoot = nullptr;
        CCLabelBMFont* m_statusLabel = nullptr;
        CCLabelBMFont* m_pageLabel = nullptr;
        std::array<CCMenuItemSpriteExtra*, 3> m_tabButtons { nullptr, nullptr, nullptr };

    protected:
        bool init() override {
            if (!CCLayerColor::initWithColor(ccc4(0, 0, 0, 120))) {
                return false;
            }

            auto win = CCDirector::get()->getWinSize();
            this->setContentSize(win);
            this->setAnchorPoint({ 0.f, 0.f });
            this->setPosition({ 0.f, 0.f });
            this->setID("cr-overlay");

            auto panelSize = CCSize { 470.f, 330.f };
            m_panel = CCLayerColor::create(ccc4(22, 28, 36, 245), panelSize.width, panelSize.height);
            m_panel->setAnchorPoint({ 0.5f, 0.5f });
            m_panel->setPosition({ win.width / 2.f, win.height / 2.f });
            this->addChild(m_panel, 2);

            auto topGlow = CCLayerColor::create(ccc4(10, 14, 20, 255), panelSize.width, 2.f);
            topGlow->setPosition({ 0.f, panelSize.height - 2.f });
            m_panel->addChild(topGlow, 1);

            auto centerX = panelSize.width / 2.f;

            auto closeMenu = CCMenu::create();
            closeMenu->setPosition({ 0.f, 0.f });
            m_panel->addChild(closeMenu, 10);

            auto closeSpr = ButtonSprite::create("X");
            closeSpr->setScale(0.55f);
            closeSpr->setColor(ccc3(255, 100, 100));

            auto closeBtn = CCMenuItemSpriteExtra::create(
                closeSpr,
                this,
                menu_selector(CustomRatesLayer::onClosePressed)
            );
            closeBtn->setPosition({ panelSize.width - 18.f, panelSize.height - 16.f });
            closeMenu->addChild(closeBtn);

            auto tabMenu = CCMenu::create();
            tabMenu->setPosition({ 0.f, 0.f });
            m_panel->addChild(tabMenu, 5);

            auto makeTabBtn = [&](char const* text, Tab tab, float x) {
                auto spr = ButtonSprite::create(text);
                spr->setScale(0.68f);

                auto btn = CCMenuItemSpriteExtra::create(
                    spr,
                    this,
                    menu_selector(CustomRatesLayer::onTabPressed)
                );
                btn->setTag(static_cast<int>(tab));
                btn->setPosition({ x, panelSize.height - 24.f });
                tabMenu->addChild(btn);
                return btn;
            };

            m_tabButtons[0] = makeTabBtn("Sent", Tab::Sent, centerX - 120.f);
            m_tabButtons[1] = makeTabBtn("Recent", Tab::Recent, centerX);
            m_tabButtons[2] = makeTabBtn("Top", Tab::Top, centerX + 120.f);

            auto header = CCLabelBMFont::create("Leaderboard-style custom rates", "bigFont.fnt");
            header->setScale(0.30f);
            header->setColor(ccc3(90, 220, 255));
            header->setPosition({ centerX, panelSize.height - 46.f });
            m_panel->addChild(header, 5);

            auto column = CCLabelBMFont::create("RANK      LEVEL ID                      META", "bigFont.fnt");
            column->setScale(0.22f);
            column->setColor(ccc3(110, 255, 120));
            column->setPosition({ centerX, panelSize.height - 66.f });
            m_panel->addChild(column, 5);

            m_rowsRoot = CCNode::create();
            m_rowsRoot->setPosition({ 0.f, 0.f });
            m_panel->addChild(m_rowsRoot, 3);

            m_statusLabel = CCLabelBMFont::create("Loading...", "bigFont.fnt");
            m_statusLabel->setScale(0.20f);
            m_statusLabel->setAnchorPoint({ 0.f, 0.5f });
            m_statusLabel->setPosition({ 14.f, 16.f });
            m_statusLabel->setColor(ccc3(120, 255, 120));
            m_panel->addChild(m_statusLabel, 5);

            m_pageLabel = CCLabelBMFont::create("Page 1/1", "bigFont.fnt");
            m_pageLabel->setScale(0.22f);
            m_pageLabel->setPosition({ centerX, 16.f });
            m_pageLabel->setColor(ccc3(90, 220, 255));
            m_panel->addChild(m_pageLabel, 5);

            auto navMenu = CCMenu::create();
            navMenu->setPosition({ 0.f, 0.f });
            m_panel->addChild(navMenu, 5);

            auto prevSpr = ButtonSprite::create("<");
            prevSpr->setScale(0.55f);
            auto prevBtn = CCMenuItemSpriteExtra::create(
                prevSpr,
                this,
                menu_selector(CustomRatesLayer::onPrevPage)
            );
            prevBtn->setPosition({ centerX - 42.f, 18.f });
            navMenu->addChild(prevBtn);

            auto nextSpr = ButtonSprite::create(">");
            nextSpr->setScale(0.55f);
            auto nextBtn = CCMenuItemSpriteExtra::create(
                nextSpr,
                this,
                menu_selector(CustomRatesLayer::onNextPage)
            );
            nextBtn->setPosition({ centerX + 42.f, 18.f });
            navMenu->addChild(nextBtn);

            auto refreshSpr = ButtonSprite::create("Refresh");
            refreshSpr->setScale(0.52f);
            auto refreshBtn = CCMenuItemSpriteExtra::create(
                refreshSpr,
                this,
                menu_selector(CustomRatesLayer::onRefresh)
            );
            refreshBtn->setPosition({ panelSize.width - 48.f, 18.f });
            navMenu->addChild(refreshBtn);

            if (isAdmin()) {
                auto adminLabel = CCLabelBMFont::create("ADMIN", "bigFont.fnt");
                adminLabel->setScale(0.17f);
                adminLabel->setColor(ccc3(255, 210, 90));
                adminLabel->setPosition({ panelSize.width - 42.f, panelSize.height - 14.f });
                m_panel->addChild(adminLabel, 5);
            }

            loadTab(Tab::Sent);
            return true;
        }

        void updateTabVisuals() {
            for (int i = 0; i < 3; ++i) {
                if (!m_tabButtons[i]) continue;

                bool active = static_cast<int>(m_tab) == i;
                m_tabButtons[i]->setColor(active ? ccc3(90, 255, 120) : ccc3(255, 255, 255));
            }
        }

        void loadTab(Tab tab) {
            m_tab = tab;
            m_page = 0;
            updateTabVisuals();

            if (m_statusLabel) {
                m_statusLabel->setString("Loading...");
            }

            int const token = ++m_requestSeq;
            this->retain();

            fetchLevels(
                tab,
                [this, token](std::vector<LevelEntry> entries) {
                    if (token != m_requestSeq) {
                        this->release();
                        return;
                    }

                    m_entries = std::move(entries);

                    if (m_statusLabel) {
                        m_statusLabel->setString(
                            (std::string("Loaded: ") + std::to_string(m_entries.size())).c_str()
                        );
                    }

                    this->release();
                    renderPage();
                },
                [this, token](std::string const& err) {
                    if (token != m_requestSeq) {
                        this->release();
                        return;
                    }

                    m_entries.clear();
                    if (m_statusLabel) {
                        m_statusLabel->setString(err.c_str());
                    }

                    this->release();
                    renderPage();
                }
            );
        }

        void renderPage() {
            if (!m_rowsRoot || !m_panel) return;

            m_rowsRoot->removeAllChildrenWithCleanup(true);

            auto const panelSize = m_panel->getContentSize();
            float const listLeft = 14.f;
            float const listTop = panelSize.height - 82.f;
            float const listWidth = panelSize.width - 28.f;
            float const rowH = 18.f;
            float const rowStep = 22.f;

            int const totalPages = std::max(1, (int)((m_entries.size() + kRowsPerPage - 1) / kRowsPerPage));
            m_page = std::clamp(m_page, 0, totalPages - 1);

            if (m_pageLabel) {
                m_pageLabel->setString(
                    (std::string("Page ") + std::to_string(m_page + 1) + "/" + std::to_string(totalPages)).c_str()
                );
            }

            int const start = m_page * kRowsPerPage;
            int const end = std::min(start + kRowsPerPage, (int)m_entries.size());

            if (start >= end) {
                auto empty = CCLabelBMFont::create("No entries yet", "bigFont.fnt");
                empty->setScale(0.28f);
                empty->setColor(ccc3(160, 160, 160));
                empty->setPosition({ panelSize.width / 2.f, panelSize.height / 2.f });
                m_rowsRoot->addChild(empty, 5);
                return;
            }

            for (int i = start; i < end; ++i) {
                auto const& e = m_entries[i];
                int const rowIndex = i - start;
                float const y = listTop - rowIndex * rowStep;

                auto rowColor = (rowIndex % 2 == 0)
                    ? ccc4(16, 22, 30, 180)
                    : ccc4(10, 14, 20, 180);

                auto row = CCLayerColor::create(rowColor, listWidth, rowH);
                row->setAnchorPoint({ 0.f, 0.5f });
                row->setPosition({ listLeft, y });
                m_rowsRoot->addChild(row, 1);

                auto rank = CCLabelBMFont::create((std::string("#") + std::to_string(i + 1)).c_str(), "bigFont.fnt");
                rank->setScale(0.22f);
                rank->setColor(ccc3(110, 255, 120));
                rank->setPosition({ 18.f, rowH / 2.f });
                row->addChild(rank, 2);

                auto level = CCLabelBMFont::create((std::string("ID ") + std::to_string(e.levelID)).c_str(), "bigFont.fnt");
                level->setScale(0.22f);
                level->setAnchorPoint({ 0.f, 0.5f });
                level->setPosition({ 48.f, rowH / 2.f });
                level->setColor(ccc3(255, 255, 255));
                row->addChild(level, 2);

                if (m_tab == Tab::Sent) {
                    std::string sentText = e.sentBy.empty()
                        ? "sent"
                        : (std::string("sent by ") + e.sentBy);

                    auto sent = CCLabelBMFont::create(sentText.c_str(), "bigFont.fnt");
                    sent->setScale(0.18f);
                    sent->setAnchorPoint({ 1.f, 0.5f });
                    sent->setPosition({ listWidth - 8.f, rowH / 2.f });
                    sent->setColor(ccc3(170, 170, 170));
                    row->addChild(sent, 2);
                } else {
                    if (auto star = CCSprite::create("blue_star.png")) {
                        star->setScale(0.28f);
                        star->setPosition({ listWidth - 120.f, rowH / 2.f });
                        row->addChild(star, 2);
                    }

                    auto stars = CCLabelBMFont::create(std::to_string(e.blueStars).c_str(), "bigFont.fnt");
                    stars->setScale(0.20f);
                    stars->setAnchorPoint({ 0.f, 0.5f });
                    stars->setPosition({ listWidth - 108.f, rowH / 2.f });
                    stars->setColor(ccc3(90, 220, 255));
                    row->addChild(stars, 2);

                    std::string modText = e.moderator.empty()
                        ? "rate by ?"
                        : (std::string("rate by ") + e.moderator);

                    auto mod = CCLabelBMFont::create(modText.c_str(), "bigFont.fnt");
                    mod->setScale(0.18f);
                    mod->setAnchorPoint({ 1.f, 0.5f });
                    mod->setPosition({ listWidth - 8.f, rowH / 2.f });
                    mod->setColor(ccc3(90, 220, 255));
                    row->addChild(mod, 2);
                }

                if (isAdmin() && m_tab == Tab::Sent) {
                    auto menu = CCMenu::create();
                    menu->setPosition({ 0.f, 0.f });
                    row->addChild(menu, 3);

                    auto delSpr = ButtonSprite::create("X");
                    delSpr->setScale(0.38f);
                    delSpr->setColor(ccc3(255, 90, 90));

                    auto delBtn = CCMenuItemSpriteExtra::create(
                        delSpr,
                        this,
                        menu_selector(CustomRatesLayer::onDeleteRow)
                    );
                    delBtn->setTag((int)e.levelID);
                    delBtn->setPosition({ listWidth - 18.f, rowH / 2.f });
                    menu->addChild(delBtn);
                }
            }
        }

        void onTabPressed(CCObject* sender) {
            auto node = typeinfo_cast<CCNode*>(sender);
            if (!node) return;

            Tab tab = (Tab)node->getTag();
            if (tab == m_tab) return;

            loadTab(tab);
        }

        void onPrevPage(CCObject*) {
            if (m_page > 0) {
                --m_page;
                renderPage();
            }
        }

        void onNextPage(CCObject*) {
            int const totalPages = std::max(1, (int)((m_entries.size() + kRowsPerPage - 1) / kRowsPerPage));
            if (m_page + 1 < totalPages) {
                ++m_page;
                renderPage();
            }
        }

        void onRefresh(CCObject*) {
            loadTab(m_tab);
        }

        void onDeleteRow(CCObject* sender) {
            if (!isAdmin()) return;

            auto node = typeinfo_cast<CCNode*>(sender);
            if (!node) return;

            int64_t const levelID = node->getTag();
            if (levelID <= 0) return;

            this->retain();
            deleteSentFromDb(levelID, [this](bool ok, std::string const& msg) {
                toast(msg, ok ? NotificationIcon::Success : NotificationIcon::Error);
                if (ok && m_tab == Tab::Sent) {
                    loadTab(Tab::Sent);
                }
                this->release();
            });
        }

        void onClosePressed(CCObject*) {
            this->removeFromParentAndCleanup(true);
        }

    public:
        static CustomRatesLayer* create() {
            auto ret = new CustomRatesLayer();
            if (ret && ret->init()) {
                ret->autorelease();
                return ret;
            }

            CC_SAFE_DELETE(ret);
            return nullptr;
        }

        static void show() {
            auto scene = CCDirector::get()->getRunningScene();
            if (!scene) return;

            if (auto old = scene->getChildByID("cr-overlay")) {
                old->removeFromParentAndCleanup(true);
            }

            auto layer = CustomRatesLayer::create();
            if (!layer) return;

            scene->addChild(layer, 99999);
        }
    };
}

class $modify(CustomRatesMenuLayer, MenuLayer) {
    bool init() override {
        if (!MenuLayer::init()) return false;

        cr::bootstrap();

        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return true;

        if (menu->getChildByID("cr-main-btn")) return true;

        auto spr = cr::makeBlueStarSprite(0.72f);
        if (!spr) {
            spr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        }

        auto btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(CustomRatesMenuLayer::onOpenCustomRates)
        );
        btn->setID("cr-main-btn");
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onOpenCustomRates(CCObject*) {
        cr::CustomRatesLayer::show();
    }
};

class $modify(CustomRatesBrowserLayer, LevelBrowserLayer) {
    void onEnter() override {
        LevelBrowserLayer::onEnter();

        if (this->getChildByID("cr-browser-menu")) return;

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setID("cr-browser-menu");
        this->addChild(menu, 9999);

        auto spr = cr::makeBlueStarSprite(0.55f);
        auto btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(CustomRatesBrowserLayer::onOpenCustomRates)
        );
        btn->setID("cr-browser-btn");

        auto win = CCDirector::get()->getWinSize();
        btn->setPosition({ win.width - 24.f, win.height - 24.f });
        menu->addChild(btn);
    }

    void onOpenCustomRates(CCObject*) {
        cr::CustomRatesLayer::show();
    }
};

class $modify(CustomRatesLevelInfoLayer, LevelInfoLayer) {
    void onEnter() override {
        LevelInfoLayer::onEnter();

        if (this->getChildByID("cr-send-menu")) return;
        if (!m_level) return;

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setID("cr-send-menu");
        this->addChild(menu, 1000);

        auto share = CCSprite::createWithSpriteFrameName("GJ_shareBtn_001.png");
        if (share) {
            share->setScale(0.85f);
        } else {
            share = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        }

        auto btn = CCMenuItemSpriteExtra::create(
            share,
            this,
            menu_selector(CustomRatesLevelInfoLayer::onSendLevel)
        );
        btn->setID("cr-send-btn");

        auto win = CCDirector::get()->getWinSize();
        btn->setPosition({ 32.f, win.height * 0.5f });
        menu->addChild(btn);
    }

    void onSendLevel(CCObject*) {
        auto levelID = cr::getLevelID(m_level);
        if (levelID <= 0) return;

        cr::fetchRatedMeta(
            levelID,
            [levelID](std::optional<cr::LevelEntry> rated) {
                if (rated) {
                    cr::toast("This level is already rated.", NotificationIcon::Error);
                    return;
                }

                auto sender = cr::currentPlayerName();
                cr::sendLevelToDb(levelID, sender, [levelID](bool ok, std::string const& msg) {
                    cr::toast(msg, ok ? NotificationIcon::Success : NotificationIcon::Error);
                });
            },
            [](std::string const& err) {
                cr::toast(err, NotificationIcon::Error);
            }
        );
    }
};

class $modify(CustomRatesInfoLayer, InfoLayer) {
    void onEnter() override {
        InfoLayer::onEnter();

        if (this->getChildByID("cr-rate-label")) return;
        if (!m_level) return;

        auto win = CCDirector::get()->getWinSize();

        auto label = CCLabelBMFont::create("checking rate...", "bigFont.fnt");
        label->setScale(0.24f);
        label->setColor(ccc3(90, 220, 255));
        label->setID("cr-rate-label");
        label->setPosition({ win.width * 0.5f, win.height - 12.f });
        this->addChild(label, 1000);

        auto levelID = cr::getLevelID(m_level);
        if (levelID <= 0) {
            label->setVisible(false);
            return;
        }

        label->retain();

        cr::fetchRatedMeta(
            levelID,
            [label](std::optional<cr::LevelEntry> info) {
                if (!label) return;

                if (!info || info->moderator.empty()) {
                    label->setVisible(false);
                    label->release();
                    return;
                }

                label->setString((std::string("rate by ") + info->moderator).c_str());
                label->setVisible(true);
                label->release();
            },
            [label](std::string const&) {
                if (!label) return;
                label->setVisible(false);
                label->release();
            }
        );
    }
};
