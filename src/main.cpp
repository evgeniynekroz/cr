#include <Geode/Geode.hpp>

#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/InfoLayer.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/binding/CreatorLayer.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/GJRateStarsLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/TextInputNode.hpp>
#include <Geode/binding/GJDropDownLayer.hpp>

#include <Geode/modify/InfoLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <Geode/loader/Event.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/ui/TextInput.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace geode::prelude;

// ============================================================
//  NAMESPACE cr — всё ядро мода
// ============================================================
namespace cr {

// ────────────────────────────────────────────────────────────
//  Константы / шифрование токена
// ────────────────────────────────────────────────────────────
static constexpr char const* kTursoUrlRaw =
    "libsql://custom-rates-evgen.aws-eu-west-1.turso.io";

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

static constexpr int kRowsPerPage = 10;

// ────────────────────────────────────────────────────────────
//  Типы
// ────────────────────────────────────────────────────────────
enum class Tab { Sent = 0, Recent = 1, Top = 2 };

enum class RateType { Star, Featured, Epic, Legendary, Mythic };

struct LevelEntry {
    int64_t     levelID    = 0;
    int         blueStars  = 0;
    std::string difficulty;
    std::string rateType;
    std::string moderator;
    std::string sentBy;
    std::string levelName;
    int64_t     sentAt     = 0;
    int64_t     ratedAt    = 0;
};

// ────────────────────────────────────────────────────────────
//  Глобальное состояние
// ────────────────────────────────────────────────────────────
static bool        g_bootstrapped = false;
static std::string g_tursoUrl;
static std::string g_tursoToken;

static std::unordered_set<std::string> g_admins = {
    "mapperok232", "kiyarikus", "nenekroz", "ujneft"
};

// Кэш: levelID → название
static std::unordered_map<int64_t, std::string> g_nameCache;

// ────────────────────────────────────────────────────────────
//  Утилиты
// ────────────────────────────────────────────────────────────
static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

static std::string escapeSql(std::string const& in) {
    std::string out;
    out.reserve(in.size() + 8);
    for (char c : in) {
        if (c == '\'') out += "''";
        else           out += c;
    }
    return out;
}

static std::string escapeJson(std::string const& in) {
    std::string out;
    out.reserve(in.size() + 8);
    for (char c : in) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

static std::string normalizeUrl(std::string url) {
    while (!url.empty() && std::isspace((unsigned char)url.back()))  url.pop_back();
    while (!url.empty() && std::isspace((unsigned char)url.front())) url.erase(url.begin());
    if (url.rfind("libsql://", 0) == 0)
        url = "https://" + url.substr(9);
    else if (url.rfind("http://", 0) != 0 && url.rfind("https://", 0) != 0)
        url = "https://" + url;
    while (!url.empty() && url.back() == '/') url.pop_back();
    return url;
}

static std::string decryptToken() {
    std::string out;
    out.reserve(std::size(kEncryptedToken));
    for (size_t i = 0; i < std::size(kEncryptedToken); ++i)
        out.push_back((char)(kEncryptedToken[i] ^ kXorKey[i % kXorKey.size()]));
    return out;
}

static std::string currentPlayerName() {
    auto gm = GameManager::sharedState();
    return gm ? std::string(gm->m_playerName) : "";
}

static bool isAdmin() {
    return g_admins.count(lower(currentPlayerName())) > 0;
}

static void toast(std::string const& msg,
                  NotificationIcon icon = NotificationIcon::Success) {
    Notification::create(msg.c_str(), icon)->show();
}

static std::string rateTypeToString(RateType rt) {
    switch (rt) {
        case RateType::Featured:  return "featured";
        case RateType::Epic:      return "epic";
        case RateType::Legendary: return "legendary";
        case RateType::Mythic:    return "mythic";
        default:                  return "star";
    }
}

// ────────────────────────────────────────────────────────────
//  HTTP / SQL helper
// ────────────────────────────────────────────────────────────
static std::string pipelineUrl() {
    return g_tursoUrl + "/v2/pipeline";
}

static std::string makePipelineBody(std::string const& sql) {
    return std::string(R"({"requests":[{"type":"execute","stmt":{"sql":")") +
           escapeJson(sql) + "\"}}]}";
}

// Возвращает строковое значение первой ячейки первой строки результата
static std::optional<std::string> extractFirstString(matjson::Value const& root) {
    auto resultsRes = root["results"].asArray();
    if (!resultsRes || resultsRes->empty()) return std::nullopt;

    auto const& first = resultsRes->front();
    if (first["error"].isObject()) return std::nullopt;

    auto rowsRes = first["response"]["result"]["rows"].asArray();
    if (!rowsRes || rowsRes->empty()) return std::nullopt;

    auto colsRes = rowsRes->front().asArray();
    if (!colsRes || colsRes->empty()) return std::nullopt;

    auto const& cell = colsRes->front();
    if (cell.isNull()) return std::nullopt;
    if (cell.isString()) return cell.asString().unwrapOr("");

    // Turso возвращает {"type":"integer","value":"123"}
    if (cell.isObject()) {
        auto v = cell["value"];
        if (v.isString()) return v.asString().unwrapOr("");
    }
    return std::nullopt;
}

// ────────────────────────────────────────────────────────────
//  WebTask wrapper — живёт на куче, удаляет себя сам
// ────────────────────────────────────────────────────────────
struct SqlTask {
    geode::EventListener<web::WebTask> listener;
    std::function<void(matjson::Value const&)> onOk;
    std::function<void(std::string const&)>    onErr;
    bool done = false;

    static void run(
        std::string const& sql,
        std::function<void(matjson::Value const&)> onOk,
        std::function<void(std::string const&)>    onErr = nullptr
    ) {
        auto* t = new SqlTask();
        t->onOk  = std::move(onOk);
        t->onErr = std::move(onErr);

        web::WebRequest req;
        req.header("Authorization", "Bearer " + g_tursoToken);
        req.header("Content-Type",  "application/json");

        auto bodyStr = makePipelineBody(sql);
        req.bodyString(bodyStr);

        t->listener.bind([t](web::WebTask::Event* ev) {
            if (t->done) return;
            if (auto* res = ev->getValue()) {
                t->done = true;
                if (res->ok()) {
                    auto text   = res->string().unwrapOr("{}");
                    auto parsed = matjson::parse(text).unwrapOr(matjson::Value());
                    if (t->onOk) t->onOk(parsed);
                } else {
                    auto text = res->string().unwrapOr("Request failed");
                    if (t->onErr) t->onErr(text);
                }
                delete t;
            } else if (ev->isCancelled()) {
                t->done = true;
                if (t->onErr) t->onErr("Cancelled");
                delete t;
            }
        });
        t->listener.setFilter(req.post(pipelineUrl()));
    }
};

static void runSql(
    std::string const& sql,
    std::function<void(matjson::Value const&)> onOk,
    std::function<void(std::string const&)>    onErr = nullptr
) {
    SqlTask::run(sql, std::move(onOk), std::move(onErr));
}

// ────────────────────────────────────────────────────────────
//  Парсинг ответов БД
// ────────────────────────────────────────────────────────────
static std::vector<LevelEntry> parseLevelsJson(std::string const& jsonStr) {
    std::vector<LevelEntry> out;
    auto parsed = matjson::parse(jsonStr).unwrapOr(matjson::Value());
    if (!parsed.isArray()) return out;

    for (auto const& item : *parsed.asArray()) {
        LevelEntry e;
        e.levelID    = item["level_id"].asInt().unwrapOr(0);
        e.blueStars  = item["blue_stars"].asInt().unwrapOr(0);
        e.difficulty = item["difficulty"].asString().unwrapOr("normal");
        e.rateType   = item["rate_type"].asString().unwrapOr("star");
        e.moderator  = item["moderator"].asString().unwrapOr("");
        e.sentBy     = item["sent_by"].asString().unwrapOr("");
        e.sentAt     = item["sent_at"].asInt().unwrapOr(0);
        e.ratedAt    = item["rated_at"].asInt().unwrapOr(0);

        // Подставить название из кэша если есть
        auto it = g_nameCache.find(e.levelID);
        if (it != g_nameCache.end()) e.levelName = it->second;

        if (e.levelID > 0) out.push_back(std::move(e));
    }
    return out;
}

static std::optional<LevelEntry> parseOneLevelJson(std::string const& jsonStr) {
    auto parsed = matjson::parse(jsonStr).unwrapOr(matjson::Value());
    if (!parsed.isObject()) return std::nullopt;

    LevelEntry e;
    e.levelID    = parsed["level_id"].asInt().unwrapOr(0);
    e.blueStars  = parsed["blue_stars"].asInt().unwrapOr(0);
    e.difficulty = parsed["difficulty"].asString().unwrapOr("normal");
    e.rateType   = parsed["rate_type"].asString().unwrapOr("star");
    e.moderator  = parsed["moderator"].asString().unwrapOr("");
    e.sentBy     = parsed["sent_by"].asString().unwrapOr("");
    e.sentAt     = parsed["sent_at"].asInt().unwrapOr(0);
    e.ratedAt    = parsed["rated_at"].asInt().unwrapOr(0);

    if (e.levelID <= 0) return std::nullopt;
    return e;
}

// ────────────────────────────────────────────────────────────
//  SQL-запросы
// ────────────────────────────────────────────────────────────
static void loadAdminsFromDb() {
    runSql(
        "SELECT COALESCE(json_group_array(lower(username)),'[]') AS data "
        "FROM admins ORDER BY username ASC;",
        [](matjson::Value const& root) {
            auto data = extractFirstString(root);
            if (!data) return;
            auto parsed = matjson::parse(*data).unwrapOr(matjson::Value());
            if (!parsed.isArray()) return;
            for (auto const& item : *parsed.asArray()) {
                if (item.isString())
                    g_admins.insert(item.asString().unwrapOr(""));
            }
        }
    );
}

static void fetchLevels(
    Tab tab,
    std::function<void(std::vector<LevelEntry>)> onOk,
    std::function<void(std::string const&)>      onErr = nullptr
) {
    std::string sql;
    switch (tab) {
        case Tab::Sent:
            sql = "SELECT COALESCE(json_group_array(json_object("
                  "'level_id',level_id,'sent_by',sent_by,'sent_at',sent_at"
                  ")),'[]') AS data FROM "
                  "(SELECT level_id,sent_by,sent_at FROM sent_levels "
                  "ORDER BY sent_at DESC LIMIT 100);";
            break;
        case Tab::Recent:
            sql = "SELECT COALESCE(json_group_array(json_object("
                  "'level_id',level_id,'blue_stars',blue_stars,"
                  "'difficulty',difficulty,'rate_type',rate_type,"
                  "'moderator',moderator,'rated_at',rated_at"
                  ")),'[]') AS data FROM "
                  "(SELECT level_id,blue_stars,difficulty,rate_type,moderator,rated_at "
                  "FROM rated_levels ORDER BY rated_at DESC LIMIT 100);";
            break;
        case Tab::Top:
            sql = "SELECT COALESCE(json_group_array(json_object("
                  "'level_id',level_id,'blue_stars',blue_stars,"
                  "'difficulty',difficulty,'rate_type',rate_type,"
                  "'moderator',moderator,'rated_at',rated_at"
                  ")),'[]') AS data FROM "
                  "(SELECT level_id,blue_stars,difficulty,rate_type,moderator,rated_at "
                  "FROM rated_levels ORDER BY blue_stars DESC,rated_at DESC LIMIT 100);";
            break;
    }

    runSql(sql,
        [onOk, onErr](matjson::Value const& root) {
            auto data = extractFirstString(root);
            if (!data) {
                if (onErr) onErr("Invalid DB response");
                else onOk({});
                return;
            }
            onOk(parseLevelsJson(*data));
        },
        [onOk, onErr](std::string const& err) {
            if (onErr) onErr(err);
            else onOk({});
        }
    );
}

static void fetchRatedMeta(
    int64_t levelID,
    std::function<void(std::optional<LevelEntry>)> onOk,
    std::function<void(std::string const&)>        onErr = nullptr
) {
    std::string sql =
        "SELECT json_object("
        "'level_id',level_id,'blue_stars',blue_stars,"
        "'difficulty',difficulty,'rate_type',rate_type,"
        "'moderator',moderator,'rated_at',rated_at"
        ") AS data FROM rated_levels WHERE level_id=" +
        std::to_string(levelID) + " LIMIT 1;";

    runSql(sql,
        [onOk](matjson::Value const& root) {
            auto data = extractFirstString(root);
            if (!data) { onOk(std::nullopt); return; }
            onOk(parseOneLevelJson(*data));
        },
        [onOk, onErr](std::string const& err) {
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
    std::string sql =
        "INSERT OR IGNORE INTO sent_levels (level_id,sent_by,sent_at) VALUES (" +
        std::to_string(levelID) + ",'" + escapeSql(sender) + "',unixepoch());";

    runSql(sql,
        [onDone](matjson::Value const&) { onDone(true,  "Level sent for review!"); },
        [onDone](std::string const& e)  { onDone(false, e); }
    );
}

static void deleteSentFromDb(
    int64_t levelID,
    std::function<void(bool, std::string const&)> onDone
) {
    runSql(
        "DELETE FROM sent_levels WHERE level_id=" + std::to_string(levelID) + ";",
        [onDone](matjson::Value const&) { onDone(true,  "Removed from sent list."); },
        [onDone](std::string const& e)  { onDone(false, e); }
    );
}

static void deleteRatedFromDb(
    int64_t levelID,
    std::function<void(bool, std::string const&)> onDone
) {
    runSql(
        "DELETE FROM rated_levels WHERE level_id=" + std::to_string(levelID) + ";",
        [onDone](matjson::Value const&) { onDone(true,  "Rating removed."); },
        [onDone](std::string const& e)  { onDone(false, e); }
    );
}

static void rateLevelInDb(
    int64_t levelID,
    int blueStars,
    std::string const& difficulty,
    std::string const& rateType,
    std::string const& moderator,
    std::function<void(bool, std::string const&)> onDone
) {
    // Сначала убираем из sent, потом вставляем в rated
    std::string delSql =
        "DELETE FROM sent_levels WHERE level_id=" + std::to_string(levelID) + ";";

    runSql(delSql,
        [=](matjson::Value const&) {
            std::string sql =
                "INSERT OR REPLACE INTO rated_levels "
                "(level_id,blue_stars,difficulty,rate_type,moderator,rated_at) VALUES (" +
                std::to_string(levelID) + "," +
                std::to_string(blueStars) + ",'" +
                escapeSql(difficulty) + "','" +
                escapeSql(rateType)   + "','" +
                escapeSql(moderator)  + "',unixepoch());";

            runSql(sql,
                [onDone](matjson::Value const&) { onDone(true,  "Level rated!"); },
                [onDone](std::string const& e)  { onDone(false, e); }
            );
        },
        [onDone](std::string const& e) { onDone(false, e); }
    );
}

static void awardStarsForLevel(
    int64_t levelID,
    int blueStars,
    std::string const& username
) {
    if (username.empty() || levelID <= 0 || blueStars <= 0) return;

    // Проверяем, не проходил ли уже
    std::string checkSql =
        "SELECT COUNT(*) FROM completed_levels WHERE username='" +
        escapeSql(lower(username)) + "' AND level_id=" +
        std::to_string(levelID) + ";";

    runSql(checkSql,
        [=](matjson::Value const& root) {
            auto val = extractFirstString(root);
            if (!val || *val == "0" || val->empty()) {
                // Ещё не проходил — записываем и начисляем
                std::string insertSql =
                    "INSERT OR IGNORE INTO completed_levels "
                    "(username,level_id,completed_at) VALUES ('" +
                    escapeSql(lower(username)) + "'," +
                    std::to_string(levelID) + ",unixepoch());";

                runSql(insertSql,
                    [=](matjson::Value const&) {
                        std::string updateSql =
                            "INSERT INTO player_stars (username,stars,updated_at) VALUES ('" +
                            escapeSql(lower(username)) + "'," +
                            std::to_string(blueStars) + ",unixepoch()) "
                            "ON CONFLICT(username) DO UPDATE SET "
                            "stars=stars+" + std::to_string(blueStars) +
                            ",updated_at=unixepoch();";
                        runSql(updateSql, nullptr, nullptr);
                    }
                );
            }
        }
    );
}

static void fetchPlayerStars(
    std::string const& username,
    std::function<void(int)> onDone
) {
    std::string sql =
        "SELECT stars FROM player_stars WHERE username='" +
        escapeSql(lower(username)) + "' LIMIT 1;";

    runSql(sql,
        [onDone](matjson::Value const& root) {
            auto val = extractFirstString(root);
            int stars = 0;
            if (val) {
                try { stars = std::stoi(*val); } catch (...) {}
            }
            onDone(stars);
        },
        [onDone](std::string const&) { onDone(0); }
    );
}

// ────────────────────────────────────────────────────────────
//  Bootstrap
// ────────────────────────────────────────────────────────────
static void bootstrap() {
    if (g_bootstrapped) return;
    g_bootstrapped = true;
    g_tursoUrl   = normalizeUrl(kTursoUrlRaw);
    g_tursoToken = decryptToken();
    loadAdminsFromDb();
}

static int64_t getLevelID(GJGameLevel* l) {
    return l ? (int64_t)l->m_levelID : -1;
}

// ════════════════════════════════════════════════════════════
//  GJ-стиль: иконки для rate_type
// ════════════════════════════════════════════════════════════
static CCSprite* makeRateTypeIcon(std::string const& rt, float scale = 1.f) {
    const char* frame = "GJ_starsIcon_001.png";
    if      (rt == "featured")  frame = "GJ_featuredCoin_001.png";
    else if (rt == "epic")      frame = "GJ_epicCoin_001.png";
    else if (rt == "legendary") frame = "GJ_epicCoin2_001.png";
    else if (rt == "mythic")    frame = "GJ_epicCoin3_001.png";

    auto spr = CCSprite::createWithSpriteFrameName(frame);
    if (spr) spr->setScale(scale);
    return spr;
}

static CCSprite* makeDifficultyIcon(std::string const& diff, float scale = 1.f) {
    const char* frame = "difficulty_00_btn_001.png";
    if      (diff == "easy")       frame = "difficulty_01_btn_001.png";
    else if (diff == "normal")     frame = "difficulty_02_btn_001.png";
    else if (diff == "hard")       frame = "difficulty_03_btn_001.png";
    else if (diff == "harder")     frame = "difficulty_04_btn_001.png";
    else if (diff == "insane")     frame = "difficulty_05_btn_001.png";
    else if (diff == "easy_demon") frame = "difficulty_06_btn_001.png";
    else if (diff == "med_demon")  frame = "difficulty_07_btn_001.png";
    else if (diff == "hard_demon") frame = "difficulty_08_btn_001.png";
    else if (diff == "insane_demon") frame = "difficulty_09_btn_001.png";
    else if (diff == "extreme_demon") frame = "difficulty_10_btn_001.png";

    auto spr = CCSprite::createWithSpriteFrameName(frame);
    if (spr) spr->setScale(scale);
    return spr;
}

// ════════════════════════════════════════════════════════════
//  DeleteLevelPopup — диалог для ввода ID уровня (удаление рейта)
// ════════════════════════════════════════════════════════════
class DeleteLevelPopup : public geode::Popup<> {
protected:
    geode::TextInput* m_input = nullptr;

    bool setup() override {
        this->setTitle("Remove Rating");

        auto winSize = m_mainLayer->getContentSize();

        auto label = CCLabelBMFont::create("Enter Level ID:", "bigFont.fnt");
        label->setScale(0.5f);
        label->setPosition({ winSize.width / 2.f, winSize.height / 2.f + 30.f });
        m_mainLayer->addChild(label);

        m_input = geode::TextInput::create(200.f, "Level ID");
        m_input->setFilter("0123456789");
        m_input->setPosition({ winSize.width / 2.f, winSize.height / 2.f });
        m_mainLayer->addChild(m_input);

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        m_mainLayer->addChild(menu);

        auto confirmSpr = ButtonSprite::create(
            "Delete", "goldFont.fnt", "GJ_button_06.png", 0.8f);
        auto confirmBtn = CCMenuItemSpriteExtra::create(
            confirmSpr, this,
            menu_selector(DeleteLevelPopup::onConfirm));
        confirmBtn->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 40.f });
        menu->addChild(confirmBtn);

        return true;
    }

    void onConfirm(CCObject*) {
        auto text = m_input->getString();
        if (text.empty()) return;

        int64_t levelID = 0;
        try { levelID = std::stoll(text); } catch (...) { return; }
        if (levelID <= 0) return;

        this->setTouchEnabled(false);

        deleteRatedFromDb(levelID, [this](bool ok, std::string const& msg) {
            toast(msg, ok ? NotificationIcon::Success : NotificationIcon::Error);
            this->onClose(nullptr);
        });
        deleteSentFromDb(levelID, nullptr);
    }

public:
    static DeleteLevelPopup* create() {
        auto ret = new DeleteLevelPopup();
        if (ret && ret->initAnchored(280.f, 160.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ════════════════════════════════════════════════════════════
//  AdminRatePopup — окно оценки уровня (только для админов)
//  Имитирует GJRateStarsLayer по стилю
// ════════════════════════════════════════════════════════════
class AdminRatePopup : public geode::Popup<int64_t> {
private:
    int64_t     m_levelID   = 0;
    int         m_stars     = 1;
    std::string m_difficulty = "normal";
    std::string m_rateType  = "star";

    CCLabelBMFont*  m_starsLabel  = nullptr;
    CCMenuItemSpriteExtra* m_diffBtn  = nullptr;
    CCMenuItemSpriteExtra* m_typeBtn  = nullptr;

    static constexpr const char* kDiffs[] = {
        "auto","easy","normal","hard","harder",
        "insane","easy_demon","med_demon",
        "hard_demon","insane_demon","extreme_demon"
    };
    static constexpr const char* kTypes[] = {
        "star","featured","epic","legendary","mythic"
    };

    int m_diffIdx = 2; // normal
    int m_typeIdx = 0; // star

protected:
    bool setup(int64_t levelID) override {
        m_levelID = levelID;
        this->setTitle("Rate Level");

        auto w = m_mainLayer->getContentSize();
        float cx = w.width / 2.f;

        // Stars slider label
        auto starsTitle = CCLabelBMFont::create("Blue Stars:", "bigFont.fnt");
        starsTitle->setScale(0.45f);
        starsTitle->setPosition({ cx, w.height - 50.f });
        m_mainLayer->addChild(starsTitle);

        m_starsLabel = CCLabelBMFont::create("1", "bigFont.fnt");
        m_starsLabel->setScale(0.7f);
        m_starsLabel->setColor(ccc3(100, 200, 255));
        m_starsLabel->setPosition({ cx, w.height - 80.f });
        m_mainLayer->addChild(m_starsLabel);

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        m_mainLayer->addChild(menu);

        // Minus / Plus для звёзд
        auto minusSpr = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
        minusSpr->setScale(0.8f);
        auto minusBtn = CCMenuItemSpriteExtra::create(
            minusSpr, this, menu_selector(AdminRatePopup::onStarMinus));
        minusBtn->setPosition({ cx - 40.f, w.height - 80.f });
        menu->addChild(minusBtn);

        auto plusSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        plusSpr->setScale(0.8f);
        auto plusBtn = CCMenuItemSpriteExtra::create(
            plusSpr, this, menu_selector(AdminRatePopup::onStarPlus));
        plusBtn->setPosition({ cx + 40.f, w.height - 80.f });
        menu->addChild(plusBtn);

        // Difficulty
        auto diffTitle = CCLabelBMFont::create("Difficulty:", "bigFont.fnt");
        diffTitle->setScale(0.45f);
        diffTitle->setPosition({ cx, w.height - 120.f });
        m_mainLayer->addChild(diffTitle);

        auto diffIcon = makeDifficultyIcon(m_difficulty, 0.9f);
        if (!diffIcon) diffIcon = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
        m_diffBtn = CCMenuItemSpriteExtra::create(
            diffIcon, this, menu_selector(AdminRatePopup::onDiffCycle));
        m_diffBtn->setPosition({ cx, w.height - 150.f });
        menu->addChild(m_diffBtn);

        // Rate type
        auto typeTitle = CCLabelBMFont::create("Rate Type:", "bigFont.fnt");
        typeTitle->setScale(0.45f);
        typeTitle->setPosition({ cx, w.height - 185.f });
        m_mainLayer->addChild(typeTitle);

        auto typeIcon = makeRateTypeIcon(m_rateType, 0.9f);
        if (!typeIcon) typeIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        m_typeBtn = CCMenuItemSpriteExtra::create(
            typeIcon, this, menu_selector(AdminRatePopup::onTypeCycle));
        m_typeBtn->setPosition({ cx, w.height - 210.f });
        menu->addChild(m_typeBtn);

        // Confirm
        auto confirmSpr = ButtonSprite::create(
            "Rate!", "goldFont.fnt", "GJ_button_01.png", 0.9f);
        auto confirmBtn = CCMenuItemSpriteExtra::create(
            confirmSpr, this, menu_selector(AdminRatePopup::onConfirm));
        confirmBtn->setPosition({ cx, 35.f });
        menu->addChild(confirmBtn);

        return true;
    }

    void onStarMinus(CCObject*) {
        if (m_stars > 1) --m_stars;
        updateStarsLabel();
    }
    void onStarPlus(CCObject*) {
        if (m_stars < 20) ++m_stars;
        updateStarsLabel();
    }
    void updateStarsLabel() {
        m_starsLabel->setString(std::to_string(m_stars).c_str());
    }

    void onDiffCycle(CCObject*) {
        m_diffIdx = (m_diffIdx + 1) % 11;
        m_difficulty = kDiffs[m_diffIdx];

        auto newIcon = makeDifficultyIcon(m_difficulty, 0.9f);
        if (newIcon) m_diffBtn->setNormalImage(newIcon);
    }

    void onTypeCycle(CCObject*) {
        m_typeIdx = (m_typeIdx + 1) % 5;
        m_rateType = kTypes[m_typeIdx];

        auto newIcon = makeRateTypeIcon(m_rateType, 0.9f);
        if (newIcon) m_typeBtn->setNormalImage(newIcon);
    }

    void onConfirm(CCObject*) {
        this->setTouchEnabled(false);
        auto moderator = currentPlayerName();

        rateLevelInDb(m_levelID, m_stars, m_difficulty, m_rateType, moderator,
            [this](bool ok, std::string const& msg) {
                toast(msg, ok ? NotificationIcon::Success : NotificationIcon::Error);
                this->onClose(nullptr);
            }
        );
    }

public:
    static AdminRatePopup* create(int64_t levelID) {
        auto ret = new AdminRatePopup();
        if (ret && ret->initAnchored(300.f, 280.f, levelID)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ════════════════════════════════════════════════════════════
//  CustomRatesLayer — основное окно (выезжает сверху как GJDropDownLayer)
// ════════════════════════════════════════════════════════════
class CustomRatesLayer : public GJDropDownLayer {
private:
    Tab   m_tab    = Tab::Sent;
    int   m_page   = 0;
    int   m_reqSeq = 0;
    std::vector<LevelEntry> m_entries;

    CCNode*         m_listRoot   = nullptr;
    CCLabelBMFont*  m_pageLabel  = nullptr;
    CCLabelBMFont*  m_statusLabel = nullptr;

    std::array<CCMenuItemSpriteExtra*, 3> m_tabBtns { nullptr, nullptr, nullptr };

protected:
    bool init() {
        // GJDropDownLayer::init принимает заголовок и высоту
        if (!GJDropDownLayer::init("Custom Rates", 220.f)) return false;

        auto win = CCDirector::get()->getWinSize();
        float cx = win.width / 2.f;

        // Вкладки
        auto tabMenu = CCMenu::create();
        tabMenu->setPosition({ cx, win.height - 40.f });
        tabMenu->setZOrder(5);
        this->addChild(tabMenu);

        auto makeTab = [&](const char* text, Tab tab, float x) {
            auto spr = ButtonSprite::create(
                text, "goldFont.fnt", "GJ_button_04.png", 0.7f);
            auto btn = CCMenuItemSpriteExtra::create(
                spr, this, menu_selector(CustomRatesLayer::onTabPressed));
            btn->setTag((int)tab);
            btn->setPositionX(x);
            tabMenu->addChild(btn);
            return btn;
        };

        m_tabBtns[0] = makeTab("Sent",   Tab::Sent,   -130.f);
        m_tabBtns[1] = makeTab("Recent", Tab::Recent,    0.f);
        m_tabBtns[2] = makeTab("Top",    Tab::Top,     130.f);

        // Список
        m_listRoot = CCNode::create();
        m_listRoot->setPosition({ cx, win.height / 2.f });
        this->addChild(m_listRoot, 3);

        // Статус / страница
        m_statusLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_statusLabel->setScale(0.35f);
        m_statusLabel->setPosition({ cx, 52.f });
        m_statusLabel->setZOrder(5);
        this->addChild(m_statusLabel);

        m_pageLabel = CCLabelBMFont::create("Page 1/1", "goldFont.fnt");
        m_pageLabel->setScale(0.35f);
        m_pageLabel->setPosition({ cx, 36.f });
        m_pageLabel->setZOrder(5);
        this->addChild(m_pageLabel);

        // Навигация
        auto navMenu = CCMenu::create();
        navMenu->setPosition({ cx, 20.f });
        navMenu->setZOrder(5);
        this->addChild(navMenu);

        auto prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        prevSpr->setFlipX(true);
        auto prevBtn = CCMenuItemSpriteExtra::create(
            prevSpr, this, menu_selector(CustomRatesLayer::onPrevPage));
        prevBtn->setPositionX(-60.f);
        navMenu->addChild(prevBtn);

        auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto nextBtn = CCMenuItemSpriteExtra::create(
            nextSpr, this, menu_selector(CustomRatesLayer::onNextPage));
        nextBtn->setPositionX(60.f);
        navMenu->addChild(nextBtn);

        auto refreshSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        auto refreshBtn = CCMenuItemSpriteExtra::create(
            refreshSpr, this, menu_selector(CustomRatesLayer::onRefresh));
        refreshBtn->setPositionX(0.f);
        navMenu->addChild(refreshBtn);

        // Кнопка Delete (только для админов)
        if (isAdmin()) {
            auto delMenu = CCMenu::create();
            delMenu->setPosition({ win.width - 40.f, 20.f });
            delMenu->setZOrder(5);
            this->addChild(delMenu);

            auto delSpr = ButtonSprite::create(
                "Delete", "goldFont.fnt", "GJ_button_06.png", 0.55f);
            auto delBtn = CCMenuItemSpriteExtra::create(
                delSpr, this, menu_selector(CustomRatesLayer::onDeletePressed));
            delMenu->addChild(delBtn);
        }

        loadTab(Tab::Sent);
        return true;
    }

    void updateTabVisuals() {
        for (int i = 0; i < 3; ++i) {
            if (!m_tabBtns[i]) continue;
            bool active = (int)m_tab == i;
            m_tabBtns[i]->setColor(active ? ccc3(255, 220, 50) : ccc3(255, 255, 255));
        }
    }

    void loadTab(Tab tab) {
        m_tab  = tab;
        m_page = 0;
        updateTabVisuals();
        if (m_statusLabel) m_statusLabel->setString("Loading...");

        int token = ++m_reqSeq;
        this->retain();

        fetchLevels(tab,
            [this, token](std::vector<LevelEntry> entries) {
                if (token != m_reqSeq) { this->release(); return; }
                m_entries = std::move(entries);
                if (m_statusLabel)
                    m_statusLabel->setString(
                        ("Loaded: " + std::to_string(m_entries.size())).c_str());
                this->release();
                renderPage();
            },
            [this, token](std::string const& err) {
                if (token != m_reqSeq) { this->release(); return; }
                m_entries.clear();
                if (m_statusLabel) m_statusLabel->setString(err.c_str());
                this->release();
                renderPage();
            }
        );
    }

    void renderPage() {
        if (!m_listRoot) return;
        m_listRoot->removeAllChildrenWithCleanup(true);

        auto win = CCDirector::get()->getWinSize();
        float cx = win.width / 2.f;

        int total = std::max(1,
            (int)((m_entries.size() + kRowsPerPage - 1) / kRowsPerPage));
        m_page = std::clamp(m_page, 0, total - 1);

        if (m_pageLabel)
            m_pageLabel->setString(
                ("Page " + std::to_string(m_page + 1) +
                 "/" + std::to_string(total)).c_str());

        int start = m_page * kRowsPerPage;
        int end   = std::min(start + kRowsPerPage, (int)m_entries.size());

        if (start >= end) {
            auto lbl = CCLabelBMFont::create("No entries yet.", "goldFont.fnt");
            lbl->setScale(0.5f);
            lbl->setColor(ccc3(200, 200, 200));
            m_listRoot->addChild(lbl);
            return;
        }

        // Используем GJListLayer для красивого фона в стиле GD
        float rowH   = 32.f;
        float listW  = 356.f;
        float listH  = rowH * kRowsPerPage;

        for (int i = start; i < end; ++i) {
            auto const& e  = m_entries[i];
            int rowIdx     = i - start;
            float y        = listH / 2.f - rowIdx * rowH;

            // Фон строки
            auto rowBg = CCLayerColor::create(
                rowIdx % 2 == 0
                    ? ccc4(0, 0, 0, 50)
                    : ccc4(0, 0, 0, 20),
                listW, rowH - 2.f);
            rowBg->setAnchorPoint({ 0.5f, 0.5f });
            rowBg->setPosition({ -listW / 2.f, y - rowH / 2.f });
            m_listRoot->addChild(rowBg, 1);

            // Ранг
            auto rankLbl = CCLabelBMFont::create(
                ("#" + std::to_string(i + 1)).c_str(), "bigFont.fnt");
            rankLbl->setScale(0.28f);
            rankLbl->setColor(ccc3(255, 220, 50));
            rankLbl->setAnchorPoint({ 0.f, 0.5f });
            rankLbl->setPosition({ -listW / 2.f + 6.f, y });
            m_listRoot->addChild(rankLbl, 2);

            // Иконка сложности (для rated вкладок)
            if (m_tab != Tab::Sent) {
                auto diffSpr = makeDifficultyIcon(e.difficulty, 0.35f);
                if (diffSpr) {
                    diffSpr->setPosition({ -listW / 2.f + 42.f, y });
                    m_listRoot->addChild(diffSpr, 2);
                }
            }

            // Название / ID уровня (кликабельно)
            std::string nameStr = e.levelName.empty()
                ? ("ID: " + std::to_string(e.levelID))
                : e.levelName;

            auto nameBtn = CCMenuItemLabel::create(
                CCLabelBMFont::create(nameStr.c_str(), "bigFont.fnt"),
                this,
                menu_selector(CustomRatesLayer::onLevelRowPressed));
            nameBtn->setTag((int)e.levelID);

            // Масштаб с учётом длины
            auto* nameLbl = static_cast<CCLabelBMFont*>(nameBtn->getLabel());
            nameLbl->setScale(0.30f);
            nameLbl->setColor(ccc3(255, 255, 255));

            // Отдельное меню на строку
            auto rowMenu = CCMenu::create();
            rowMenu->setPosition({ -listW / 2.f + 60.f, y });
            m_listRoot->addChild(rowMenu, 3);
            rowMenu->addChild(nameBtn);

            // Правая часть
            if (m_tab == Tab::Sent) {
                std::string sentStr = e.sentBy.empty()
                    ? "—"
                    : ("by " + e.sentBy);
                auto sentLbl = CCLabelBMFont::create(sentStr.c_str(), "bigFont.fnt");
                sentLbl->setScale(0.22f);
                sentLbl->setColor(ccc3(170, 170, 170));
                sentLbl->setAnchorPoint({ 1.f, 0.5f });
                sentLbl->setPosition({ listW / 2.f - 6.f, y });
                m_listRoot->addChild(sentLbl, 2);
            } else {
                // Иконка rate_type
                auto typeIcon = makeRateTypeIcon(e.rateType, 0.45f);
                if (typeIcon) {
                    typeIcon->setPosition({ listW / 2.f - 60.f, y });
                    m_listRoot->addChild(typeIcon, 2);
                }

                // Синие звёзды
                auto starSpr = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
                if (starSpr) {
                    starSpr->setColor(ccc3(100, 200, 255));
                    starSpr->setScale(0.5f);
                    starSpr->setPosition({ listW / 2.f - 38.f, y });
                    m_listRoot->addChild(starSpr, 2);
                }

                auto starsLbl = CCLabelBMFont::create(
                    std::to_string(e.blueStars).c_str(), "bigFont.fnt");
                starsLbl->setScale(0.28f);
                starsLbl->setColor(ccc3(100, 200, 255));
                starsLbl->setAnchorPoint({ 0.f, 0.5f });
                starsLbl->setPosition({ listW / 2.f - 26.f, y });
                m_listRoot->addChild(starsLbl, 2);

                // Модератор
                std::string modStr = e.moderator.empty()
                    ? "?"
                    : e.moderator;
                auto modLbl = CCLabelBMFont::create(modStr.c_str(), "bigFont.fnt");
                modLbl->setScale(0.20f);
                modLbl->setColor(ccc3(90, 220, 255));
                modLbl->setAnchorPoint({ 1.f, 0.5f });
                modLbl->setPosition({ listW / 2.f - 6.f, y });
                m_listRoot->addChild(modLbl, 2);
            }
        }
    }

    void onTabPressed(CCObject* sender) {
        auto* node = typeinfo_cast<CCNode*>(sender);
        if (!node) return;
        Tab tab = (Tab)node->getTag();
        if (tab == m_tab) return;
        loadTab(tab);
    }

    void onPrevPage(CCObject*) {
        if (m_page > 0) { --m_page; renderPage(); }
    }

    void onNextPage(CCObject*) {
        int total = std::max(1,
            (int)((m_entries.size() + kRowsPerPage - 1) / kRowsPerPage));
        if (m_page + 1 < total) { ++m_page; renderPage(); }
    }

    void onRefresh(CCObject*) { loadTab(m_tab); }

    void onDeletePressed(CCObject*) {
        if (!isAdmin()) return;
        DeleteLevelPopup::create()->show();
    }

    void onLevelRowPressed(CCObject* sender) {
        auto* node = typeinfo_cast<CCNode*>(sender);
        if (!node) return;
        int64_t levelID = node->getTag();
        if (levelID <= 0) return;

        // Переход на страницу уровня через GD API
        auto* gm = GameManager::sharedState();
        auto* level = GJGameLevel::create();
        level->m_levelID = (int)levelID;
        level->m_levelName = g_nameCache.count(levelID)
            ? g_nameCache[levelID]
            : std::to_string(levelID);

        auto* scene = LevelInfoLayer::scene(level, false);
        CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, scene));
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
        auto* scene = CCDirector::get()->getRunningScene();
        if (!scene) return;
        auto* layer = CustomRatesLayer::create();
        if (!layer) return;
        layer->showLayer(false);
    }
};

} // namespace cr

// ════════════════════════════════════════════════════════════
//  ХУКИ
// ════════════════════════════════════════════════════════════

// ────────────────────────────────────────────────────────────
//  CreatorLayer — кнопка Featured ведёт на наше меню
// ────────────────────────────────────────────────────────────
class $modify(CRCreatorLayer, CreatorLayer) {
    bool init() override {
        if (!CreatorLayer::init()) return false;
        cr::bootstrap();

        // Находим кнопку featured и перехватываем её
        if (auto* menu = this->getChildByID("creator-buttons-menu")) {
            if (auto* featBtn = menu->getChildByID("featured-button")) {
                // Меняем callback
                if (auto* item = typeinfo_cast<CCMenuItemSpriteExtra*>(featBtn)) {
                    item->setTarget(this, menu_selector(CRCreatorLayer::onCustomRates));
                }
            }
        }
        return true;
    }

    void onCustomRates(CCObject*) {
        cr::CustomRatesLayer::show();
    }
};

// ────────────────────────────────────────────────────────────
//  LevelInfoLayer — кнопка отправки/оценки уровня
// ────────────────────────────────────────────────────────────
class $modify(CRLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        if (!m_level) return true;

        // Ищем меню с кнопками действий (где кнопка "like" и т.д.)
        auto* menu = this->getChildByID("left-side-menu");
        if (!menu) {
            menu = CCMenu::create();
            menu->setID("cr-side-menu");
            auto win = CCDirector::get()->getWinSize();
            menu->setPosition({ 24.f, win.height / 2.f });
            this->addChild(menu, 10);
        }

        if (menu->getChildByID("cr-rate-btn")) return true;

        // Кнопка как у RobTop — GJ_reportBtn или uploadBtn
        auto* spr = CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png");
        if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        if (spr) spr->setScale(0.85f);

        auto* btn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(CRLevelInfoLayer::onCRButton));
        btn->setID("cr-rate-btn");
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onCRButton(CCObject*) {
        if (!m_level) return;
        auto levelID = cr::getLevelID(m_level);
        if (levelID <= 0) return;

        if (cr::isAdmin()) {
            // Для админа — открываем окно оценки
            cr::AdminRatePopup::create(levelID)->show();
        } else {
            // Для обычного игрока — отправить на модерацию
            auto sender = cr::currentPlayerName();
            cr::sendLevelToDb(levelID, sender,
                [](bool ok, std::string const& msg) {
                    cr::toast(msg, ok
                        ? NotificationIcon::Success
                        : NotificationIcon::Error);
                }
            );
        }
    }
};

// ────────────────────────────────────────────────────────────
//  InfoLayer — "Rated by [moderator]" в комментариях
// ────────────────────────────────────────────────────────────
class $modify(CRInfoLayer, InfoLayer) {
    bool init(GJGameLevel* level, bool unk) {
        if (!InfoLayer::init(level, unk)) return false;
        if (!m_level) return true;

        auto levelID = cr::getLevelID(m_level);
        if (levelID <= 0) return true;

        auto win = CCDirector::get()->getWinSize();

        auto* label = CCLabelBMFont::create("", "goldFont.fnt");
        label->setScale(0.35f);
        label->setColor(ccc3(100, 200, 255));
        label->setID("cr-rated-by-label");
        label->setPosition({ win.width / 2.f, win.height - 14.f });
        label->setVisible(false);
        this->addChild(label, 100);

        label->retain();

        cr::fetchRatedMeta(levelID,
            [label](std::optional<cr::LevelEntry> info) {
                if (!info || info->moderator.empty()) {
                    label->setVisible(false);
                    label->release();
                    return;
                }
                auto text = std::string("Custom rated by ") + info->moderator +
                            " (" + std::to_string(info->blueStars) + " \u2605)";
                label->setString(text.c_str());
                label->setVisible(true);
                label->release();
            },
            [label](std::string const&) {
                label->setVisible(false);
                label->release();
            }
        );

        return true;
    }
};

// ────────────────────────────────────────────────────────────
//  PlayLayer — начисление синих звёзд по завершении уровня
// ────────────────────────────────────────────────────────────
class $modify(CRPlayLayer, PlayLayer) {
    void levelComplete() {
        PlayLayer::levelComplete();

        if (!m_level) return;
        if (m_isPracticeMode) return;

        auto levelID = cr::getLevelID(m_level);
        if (levelID <= 0) return;

        auto username = cr::currentPlayerName();

        // Проверяем, оценён ли уровень в нашей системе
        cr::fetchRatedMeta(levelID,
            [levelID, username](std::optional<cr::LevelEntry> info) {
                if (!info) return;
                if (info->blueStars <= 0) return;
                cr::awardStarsForLevel(levelID, info->blueStars, username);
            }
        );
    }
};

// ────────────────────────────────────────────────────────────
//  ProfilePage — счётчик синих звёзд
// ────────────────────────────────────────────────────────────
class $modify(CRProfilePage, ProfilePage) {
    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);
        if (!score) return;

        if (this->getChildByID("cr-blue-stars-label")) return;

        auto win = CCDirector::get()->getWinSize();
        std::string username = score->m_userName;

        auto* label = CCLabelBMFont::create("... \u2605", "goldFont.fnt");
        label->setScale(0.40f);
        label->setColor(ccc3(100, 200, 255));
        label->setID("cr-blue-stars-label");
        // Позиционируем под обычными звёздами
        label->setPosition({ win.width / 2.f + 100.f, win.height / 2.f - 50.f });
        this->addChild(label, 10);

        label->retain();

        cr::fetchPlayerStars(username,
            [label](int stars) {
                auto text = std::to_string(stars) + " \u2605";
                label->setString(text.c_str());
                label->release();
            }
        );
    }
};
