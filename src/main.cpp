#include <Geode/Geode.hpp>

// Обязательные инклюды для HTTP и событий
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/InfoLayer.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/binding/CreatorLayer.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/GJDropDownLayer.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>

#include <Geode/modify/InfoLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/PlayLayer.hpp>

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

// Определяем тип задачи для HTTP запросов
using WebTask = geode::Task<geode::Result<geode::utils::web::WebResponse>>;

namespace cr {

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

enum class Tab { Sent = 0, Recent = 1, Top = 2 };

struct LevelEntry {
    int64_t     levelID   = 0;
    int         blueStars = 0;
    std::string difficulty;
    std::string rateType;
    std::string moderator;
    std::string sentBy;
    std::string levelName;
    int64_t     sentAt  = 0;
    int64_t     ratedAt = 0;
};

static bool        g_bootstrapped = false;
static std::string g_tursoUrl;
static std::string g_tursoToken;

static std::unordered_set<std::string> g_admins = {
    "mapperok232", "kiyarikus", "nenekroz", "ujneft"
};
static std::unordered_map<int64_t, std::string> g_nameCache;

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
    else if (url.rfind("http://",  0) != 0 &&
             url.rfind("https://", 0) != 0)
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
    auto* gm = GameManager::sharedState();
    return gm ? std::string(gm->m_playerName) : "";
}

static bool isAdmin() {
    return g_admins.count(lower(currentPlayerName())) > 0;
}

static void toast(std::string const& msg,
                  NotificationIcon icon = NotificationIcon::Success) {
    Notification::create(msg.c_str(), icon)->show();
}

static std::string pipelineUrl() {
    return g_tursoUrl + "/v2/pipeline";
}

static std::string makePipelineBody(std::string const& sql) {
    return std::string(R"({"requests":[{"type":"execute","stmt":{"sql":")") +
           escapeJson(sql) + "\"}}]}";
}

static std::vector<matjson::Value> safeArray(matjson::Value const& val) {
    if (!val.isArray()) return {};
    auto res = val.asArray();
    if (!res) return {};
    auto const& actualArr = res.unwrap();
    return std::vector<matjson::Value>(actualArr.begin(), actualArr.end());
}

static std::optional<std::string> extractFirstString(matjson::Value const& root) {
    auto results = safeArray(root["results"]);
    if (results.empty()) return std::nullopt;

    auto const& first = results.front();
    if (first["error"].isObject()) return std::nullopt;

    auto rows = safeArray(first["response"]["result"]["rows"]);
    if (rows.empty()) return std::nullopt;

    auto cols = safeArray(rows.front());
    if (cols.empty()) return std::nullopt;

    auto const& cell = cols.front();
    if (cell.isString()) return cell.asString().unwrapOr("");
    if (cell.isObject()) {
        auto v = cell["value"];
        if (v.isString()) return v.asString().unwrapOr("");
        if (v.isNumber()) return std::to_string(v.asInt().unwrapOr(0));
    }
    if (cell.isNumber()) return std::to_string(cell.asInt().unwrapOr(0));

    return std::nullopt;
}

struct SqlTask {
    geode::EventListener<WebTask> listener;
    std::function<void(matjson::Value const&)> onOk;
    std::function<void(std::string const&)>    onErr;
    bool done = false;

    static void run(
        std::string const& sql,
        std::function<void(matjson::Value const&)> onOk,
        std::function<void(std::string const&)>    onErr = nullptr
    ) {
        auto* t  = new SqlTask();
        t->onOk  = std::move(onOk);
        t->onErr = std::move(onErr);

        auto req = web::WebRequest();
        req.header("Authorization", "Bearer " + g_tursoToken);
        req.header("Content-Type",  "application/json");
        req.bodyString(makePipelineBody(sql));

        t->listener.bind([t](WebTask::Event* ev) {
            if (t->done) return;
            if (auto* res = ev->getValue()) {
                t->done = true;
                if (res->ok()) {
                    auto const& webRes = res->value();
                    auto text    = webRes.string().unwrapOr("{}");
                    auto parsed  = matjson::parse(text).unwrapOr(matjson::Value());
                    if (t->onOk) t->onOk(parsed);
                } else {
                    auto err = res->error();
                    if (t->onErr) t->onErr(err);
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

static std::vector<LevelEntry> parseLevelsJson(std::string const& jsonStr) {
    std::vector<LevelEntry> out;
    auto parsed = matjson::parse(jsonStr).unwrapOr(matjson::Value());
    if (!parsed.isArray()) return out;

    for (auto const& item : safeArray(parsed)) {
        LevelEntry e;
        e.levelID    = item["level_id"].asInt().unwrapOr(0);
        e.blueStars  = item["blue_stars"].asInt().unwrapOr(0);
        e.difficulty = item["difficulty"].asString().unwrapOr("normal");
        e.rateType   = item["rate_type"].asString().unwrapOr("star");
        e.moderator  = item["moderator"].asString().unwrapOr("");
        e.sentBy     = item["sent_by"].asString().unwrapOr("");
        e.sentAt     = item["sent_at"].asInt().unwrapOr(0);
        e.ratedAt    = item["rated_at"].asInt().unwrapOr(0);

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

static void loadAdminsFromDb() {
    runSql(
        "SELECT COALESCE(json_group_array(lower(username)),'[]') AS data "
        "FROM admins ORDER BY username ASC;",
        [](matjson::Value const& root) {
            auto data = extractFirstString(root);
            if (!data) return;
            auto parsed = matjson::parse(*data).unwrapOr(matjson::Value());
            for (auto const& item : safeArray(parsed)) {
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
                  "(SELECT level_id,blue_stars,difficulty,rate_type,"
                  "moderator,rated_at FROM rated_levels "
                  "ORDER BY rated_at DESC LIMIT 100);";
            break;
        case Tab::Top:
            sql = "SELECT COALESCE(json_group_array(json_object("
                  "'level_id',level_id,'blue_stars',blue_stars,"
                  "'difficulty',difficulty,'rate_type',rate_type,"
                  "'moderator',moderator,'rated_at',rated_at"
                  ")),'[]') AS data FROM "
                  "(SELECT level_id,blue_stars,difficulty,rate_type,"
                  "moderator,rated_at FROM rated_levels "
                  "ORDER BY blue_stars DESC,rated_at DESC LIMIT 100);";
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
    runSql(
        "INSERT OR IGNORE INTO sent_levels (level_id,sent_by,sent_at) VALUES (" +
        std::to_string(levelID) + ",'" + escapeSql(sender) + "',unixepoch());",
        [onDone](matjson::Value const&) { onDone(true,  "Level sent for review!"); },
        [onDone](std::string const& e)  { onDone(false, e); }
    );
}

static void deleteSentFromDb(
    int64_t levelID,
    std::function<void(bool, std::string const&)> onDone = nullptr
) {
    runSql(
        "DELETE FROM sent_levels WHERE level_id=" + std::to_string(levelID) + ";",
        [onDone](matjson::Value const&) { if (onDone) onDone(true,  "Removed."); },
        [onDone](std::string const& e)  { if (onDone) onDone(false, e); }
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
    runSql(
        "DELETE FROM sent_levels WHERE level_id=" + std::to_string(levelID) + ";",
        [=](matjson::Value const&) {
            runSql(
                "INSERT OR REPLACE INTO rated_levels "
                "(level_id,blue_stars,difficulty,rate_type,moderator,rated_at) VALUES (" +
                std::to_string(levelID) + "," +
                std::to_string(blueStars) + ",'" +
                escapeSql(difficulty) + "','" +
                escapeSql(rateType) + "','" +
                escapeSql(moderator) + "',unixepoch());",
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

    runSql(
        "SELECT COUNT(*) FROM completed_levels WHERE username='" +
        escapeSql(lower(username)) + "' AND level_id=" +
        std::to_string(levelID) + ";",
        [=](matjson::Value const& root) {
            auto val = extractFirstString(root);
            if (val && *val != "0" && !val->empty()) return;

            runSql(
                "INSERT OR IGNORE INTO completed_levels "
                "(username,level_id,completed_at) VALUES ('" +
                escapeSql(lower(username)) + "'," +
                std::to_string(levelID) + ",unixepoch());",
                [=](matjson::Value const&) {
                    runSql(
                        "INSERT INTO player_stars (username,stars,updated_at) "
                        "VALUES ('" + escapeSql(lower(username)) + "'," +
                        std::to_string(blueStars) + ",unixepoch()) "
                        "ON CONFLICT(username) DO UPDATE SET "
                        "stars=stars+" + std::to_string(blueStars) +
                        ",updated_at=unixepoch();",
                        nullptr, nullptr
                    );
                }
            );
        }
    );
}

static void fetchPlayerStars(
    std::string const& username,
    std::function<void(int)> onDone
) {
    runSql(
        "SELECT stars FROM player_stars WHERE username='" +
        escapeSql(lower(username)) + "' LIMIT 1;",
        [onDone](matjson::Value const& root) {
            auto val = extractFirstString(root);
            int stars = 0;
            if (val) { try { stars = std::stoi(*val); } catch (...) {} }
            onDone(stars);
        },
        [onDone](std::string const&) { onDone(0); }
    );
}

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

static CCSprite* makeRateTypeIcon(std::string const& rt, float scale = 1.f) {
    const char* frame = "GJ_starsIcon_001.png";
    if      (rt == "featured")  frame = "GJ_featuredCoin_001.png";
    else if (rt == "epic")      frame = "GJ_epicCoin_001.png";
    else if (rt == "legendary") frame = "GJ_epicCoin2_001.png";
    else if (rt == "mythic")    frame = "GJ_epicCoin3_001.png";
    auto* spr = CCSprite::createWithSpriteFrameName(frame);
    if (spr) spr->setScale(scale);
    return spr;
}

static CCSprite* makeDifficultyIcon(std::string const& diff, float scale = 1.f) {
    const char* frame = "difficulty_00_btn_001.png";
    if      (diff == "easy")           frame = "difficulty_01_btn_001.png";
    else if (diff == "normal")         frame = "difficulty_02_btn_001.png";
    else if (diff == "hard")           frame = "difficulty_03_btn_001.png";
    else if (diff == "harder")         frame = "difficulty_04_btn_001.png";
    else if (diff == "insane")         frame = "difficulty_05_btn_001.png";
    else if (diff == "easy_demon")     frame = "difficulty_06_btn_001.png";
    else if (diff == "med_demon")      frame = "difficulty_07_btn_001.png";
    else if (diff == "hard_demon")     frame = "difficulty_08_btn_001.png";
    else if (diff == "insane_demon")   frame = "difficulty_09_btn_001.png";
    else if (diff == "extreme_demon")  frame = "difficulty_10_btn_001.png";
    auto* spr = CCSprite::createWithSpriteFrameName(frame);
    if (spr) spr->setScale(scale);
    return spr;
}

class DeleteLevelPopup : public CCLayer {
private:
    geode::TextInput* m_input = nullptr;

    bool init() override {
        if (!CCLayer::init()) return false;
        auto win = CCDirector::get()->getWinSize();
        float cx = win.width  / 2.f;
        float cy = win.height / 2.f;

        auto* bg = CCLayerColor::create(ccc4(0, 0, 0, 150));
        this->addChild(bg, 0);

        auto* panel = CCScale9Sprite::create("GJ_square01.png");
        panel->setContentSize({ 280.f, 160.f });
        panel->setPosition({ cx, cy });
        this->addChild(panel, 1);

        auto* title = CCLabelBMFont::create("Remove Rating", "goldFont.fnt");
        title->setScale(0.7f);
        title->setPosition({ cx, cy + 55.f });
        this->addChild(title, 2);

        auto* label = CCLabelBMFont::create("Enter Level ID:", "bigFont.fnt");
        label->setScale(0.45f);
        label->setPosition({ cx, cy + 20.f });
        this->addChild(label, 2);

        m_input = geode::TextInput::create(200.f, "Level ID");
        m_input->setFilter("0123456789");
        m_input->setPosition({ cx, cy - 15.f });
        this->addChild(m_input, 2);

        auto* menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        this->addChild(menu, 2);

        auto* cancelSpr = ButtonSprite::create("Cancel", "goldFont.fnt", "GJ_button_06.png", 0.7f);
        auto* cancelBtn = CCMenuItemSpriteExtra::create(cancelSpr, this, menu_selector(DeleteLevelPopup::onCancel));
        cancelBtn->setPosition({ cx - 70.f, cy - 55.f });
        menu->addChild(cancelBtn);

        auto* confirmSpr = ButtonSprite::create("Delete", "goldFont.fnt", "GJ_button_01.png", 0.7f);
        auto* confirmBtn = CCMenuItemSpriteExtra::create(confirmSpr, this, menu_selector(DeleteLevelPopup::onConfirm));
        confirmBtn->setPosition({ cx + 70.f, cy - 55.f });
        menu->addChild(confirmBtn);

        this->setKeypadEnabled(true);
        return true;
    }

    void keyBackClicked() override { this->removeFromParentAndCleanup(true); }
    void onCancel(CCObject*) { this->removeFromParentAndCleanup(true); }

    void onConfirm(CCObject*) {
        if (!m_input) return;
        std::string text = std::string(m_input->getString());
        if (text.empty()) return;

        int64_t levelID = 0;
        try { levelID = std::stoll(text); } catch (...) { return; }
        if (levelID <= 0) return;

        deleteRatedFromDb(levelID, [](bool ok, std::string const& msg) {
            toast(msg, ok ? NotificationIcon::Success : NotificationIcon::Error);
        });
        deleteSentFromDb(levelID);
        this->removeFromParentAndCleanup(true);
    }

public:
    static DeleteLevelPopup* create() {
        auto* ret = new DeleteLevelPopup();
        if (ret && ret->init()) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
    static void show() {
        auto* scene = CCDirector::get()->getRunningScene();
        if (!scene) return;
        auto* popup = DeleteLevelPopup::create();
        if (popup) scene->addChild(popup, 99999);
    }
};

class AdminRatePopup : public CCLayer {
private:
    int64_t     m_levelID  = 0;
    int         m_stars    = 1;
    int         m_diffIdx  = 2;
    int         m_typeIdx  = 0;

    CCLabelBMFont*         m_starsLabel = nullptr;
    CCMenuItemSpriteExtra* m_diffBtn    = nullptr;
    CCMenuItemSpriteExtra* m_typeBtn    = nullptr;

    static constexpr const char* kDiffs[] = {
        "auto","easy","normal","hard","harder",
        "insane","easy_demon","med_demon",
        "hard_demon","insane_demon","extreme_demon"
    };
    static constexpr const char* kTypes[] = {
        "star","featured","epic","legendary","mythic"
    };

    bool init(int64_t levelID) {
        if (!CCLayer::init()) return false;
        m_levelID = levelID;

        auto win = CCDirector::get()->getWinSize();
        float cx = win.width  / 2.f;
        float cy = win.height / 2.f;

        auto* bg = CCLayerColor::create(ccc4(0, 0, 0, 150));
        this->addChild(bg, 0);

        auto* panel = CCScale9Sprite::create("GJ_square01.png");
        panel->setContentSize({ 300.f, 290.f });
        panel->setPosition({ cx, cy });
        this->addChild(panel, 1);

        auto* title = CCLabelBMFont::create("Rate Level", "goldFont.fnt");
        title->setScale(0.75f);
        title->setPosition({ cx, cy + 125.f });
        this->addChild(title, 2);

        auto* btnMenu = CCMenu::create();
        btnMenu->setPosition({ 0.f, 0.f });
        this->addChild(btnMenu, 2);

        auto* starsLbl = CCLabelBMFont::create("Blue Stars", "bigFont.fnt");
        starsLbl->setScale(0.45f);
        starsLbl->setPosition({ cx, cy + 90.f });
        this->addChild(starsLbl, 2);

        m_starsLabel = CCLabelBMFont::create("1", "bigFont.fnt");
        m_starsLabel->setScale(0.85f);
        m_starsLabel->setColor(ccc3(100, 200, 255));
        m_starsLabel->setPosition({ cx, cy + 58.f });
        this->addChild(m_starsLabel, 2);

        auto* minSpr = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
        minSpr->setScale(0.7f);
        auto* minBtn = CCMenuItemSpriteExtra::create(minSpr, this, menu_selector(AdminRatePopup::onStarMinus));
        minBtn->setPosition({ cx - 38.f, cy + 58.f });
        btnMenu->addChild(minBtn);

        auto* plusSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        plusSpr->setScale(0.7f);
        auto* plusBtn = CCMenuItemSpriteExtra::create(plusSpr, this, menu_selector(AdminRatePopup::onStarPlus));
        plusBtn->setPosition({ cx + 38.f, cy + 58.f });
        btnMenu->addChild(plusBtn);

        auto* diffLbl = CCLabelBMFont::create("Difficulty", "bigFont.fnt");
        diffLbl->setScale(0.45f);
        diffLbl->setPosition({ cx, cy + 20.f });
        this->addChild(diffLbl, 2);

        auto* diffIcon = makeDifficultyIcon(kDiffs[m_diffIdx], 0.85f);
        if (!diffIcon) diffIcon = CCSprite::createWithSpriteFrameName("difficulty_02_btn_001.png");
        m_diffBtn = CCMenuItemSpriteExtra::create(diffIcon, this, menu_selector(AdminRatePopup::onDiffCycle));
        m_diffBtn->setPosition({ cx, cy - 12.f });
        btnMenu->addChild(m_diffBtn);

        auto* typeLbl = CCLabelBMFont::create("Rate Type", "bigFont.fnt");
        typeLbl->setScale(0.45f);
        typeLbl->setPosition({ cx, cy - 48.f });
        this->addChild(typeLbl, 2);

        auto* typeIcon = makeRateTypeIcon(kTypes[m_typeIdx], 0.85f);
        if (!typeIcon) typeIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        m_typeBtn = CCMenuItemSpriteExtra::create(typeIcon, this, menu_selector(AdminRatePopup::onTypeCycle));
        m_typeBtn->setPosition({ cx, cy - 78.f });
        btnMenu->addChild(m_typeBtn);

        auto* cancelSpr = ButtonSprite::create("Cancel", "goldFont.fnt", "GJ_button_06.png", 0.7f);
        auto* cancelBtn = CCMenuItemSpriteExtra::create(cancelSpr, this, menu_selector(AdminRatePopup::onCancel));
        cancelBtn->setPosition({ cx - 75.f, cy - 120.f });
        btnMenu->addChild(cancelBtn);

        auto* rateSpr = ButtonSprite::create("Rate!", "goldFont.fnt", "GJ_button_01.png", 0.7f);
        auto* rateBtn = CCMenuItemSpriteExtra::create(rateSpr, this, menu_selector(AdminRatePopup::onConfirm));
        rateBtn->setPosition({ cx + 75.f, cy - 120.f });
        btnMenu->addChild(rateBtn);

        this->setKeypadEnabled(true);
        return true;
    }

    void keyBackClicked() override { this->removeFromParentAndCleanup(true); }
    void onCancel(CCObject*) { this->removeFromParentAndCleanup(true); }

    void onConfirm(CCObject*) {
        auto moderator = currentPlayerName();
        rateLevelInDb(m_levelID, m_stars, kDiffs[m_diffIdx], kTypes[m_typeIdx], moderator,
            [this](bool ok, std::string const& msg) {
                toast(msg, ok ? NotificationIcon::Success : NotificationIcon::Error);
                this->removeFromParentAndCleanup(true);
            }
        );
    }

    void onStarMinus(CCObject*) { if (m_stars > 1) { --m_stars; updateStarsLabel(); } }
    void onStarPlus(CCObject*) { if (m_stars < 20) { ++m_stars; updateStarsLabel(); } }
    void updateStarsLabel() { if (m_starsLabel) m_starsLabel->setString(std::to_string(m_stars).c_str()); }
    void onDiffCycle(CCObject*) {
        m_diffIdx = (m_diffIdx + 1) % 11;
        auto* icon = makeDifficultyIcon(kDiffs[m_diffIdx], 0.85f);
        if (icon && m_diffBtn) m_diffBtn->setNormalImage(icon);
    }
    void onTypeCycle(CCObject*) {
        m_typeIdx = (m_typeIdx + 1) % 5;
        auto* icon = makeRateTypeIcon(kTypes[m_typeIdx], 0.85f);
        if (icon && m_typeBtn) m_typeBtn->setNormalImage(icon);
    }

public:
    static AdminRatePopup* create(int64_t levelID) {
        auto* ret = new AdminRatePopup();
        if (ret && ret->init(levelID)) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
    static void show(int64_t levelID) {
        auto* scene = CCDirector::get()->getRunningScene();
        if (!scene) return;
        auto* popup = AdminRatePopup::create(levelID);
        if (popup) scene->addChild(popup, 99999);
    }
};

class CustomRatesLayer : public GJDropDownLayer {
private:
    Tab   m_tab    = Tab::Sent;
    int   m_page   = 0;
    int   m_reqSeq = 0;
    std::vector<LevelEntry> m_entries;
    CCNode*        m_listRoot    = nullptr;
    CCLabelBMFont* m_pageLabel   = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    std::array<CCMenuItemSpriteExtra*, 3> m_tabBtns{ nullptr, nullptr, nullptr };

    bool init() {
        if (!GJDropDownLayer::init("Custom Rates")) return false;

        auto win = CCDirector::get()->getWinSize();
        float cx = win.width / 2.f;

        auto* tabMenu = CCMenu::create();
        tabMenu->setPosition({ cx, win.height - 40.f });
        tabMenu->setZOrder(5);
        this->addChild(tabMenu);

        auto makeTab = [&](const char* text, Tab tab, float x) {
            auto* spr = ButtonSprite::create(text, "goldFont.fnt", "GJ_button_04.png", 0.65f);
            auto* btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(CustomRatesLayer::onTabPressed));
            btn->setTag((int)tab);
            btn->setPositionX(x);
            tabMenu->addChild(btn);
            return btn;
        };

        m_tabBtns[0] = makeTab("Sent",   Tab::Sent,   -120.f);
        m_tabBtns[1] = makeTab("Recent", Tab::Recent,    0.f);
        m_tabBtns[2] = makeTab("Top",    Tab::Top,     120.f);

        m_listRoot = CCNode::create();
        m_listRoot->setPosition({ cx, win.height / 2.f });
        this->addChild(m_listRoot, 3);

        m_statusLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_statusLabel->setScale(0.32f);
        m_statusLabel->setPosition({ cx, 52.f });
        m_statusLabel->setZOrder(5);
        this->addChild(m_statusLabel);

        m_pageLabel = CCLabelBMFont::create("Page 1/1", "goldFont.fnt");
        m_pageLabel->setScale(0.32f);
        m_pageLabel->setPosition({ cx, 36.f });
        m_pageLabel->setZOrder(5);
        this->addChild(m_pageLabel);

        auto* navMenu = CCMenu::create();
        navMenu->setPosition({ cx, 20.f });
        navMenu->setZOrder(5);
        this->addChild(navMenu);

        auto* prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        prevSpr->setFlipX(true);
        auto* prevBtn = CCMenuItemSpriteExtra::create(prevSpr, this, menu_selector(CustomRatesLayer::onPrevPage));
        prevBtn->setPositionX(-60.f);
        navMenu->addChild(prevBtn);

        auto* nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto* nextBtn = CCMenuItemSpriteExtra::create(nextSpr, this, menu_selector(CustomRatesLayer::onNextPage));
        nextBtn->setPositionX(60.f);
        navMenu->addChild(nextBtn);

        auto* refreshSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        auto* refreshBtn = CCMenuItemSpriteExtra::create(refreshSpr, this, menu_selector(CustomRatesLayer::onRefresh));
        refreshBtn->setPositionX(0.f);
        navMenu->addChild(refreshBtn);

        if (isAdmin()) {
            auto* delMenu = CCMenu::create();
            delMenu->setPosition({ win.width - 50.f, 20.f });
            delMenu->setZOrder(5);
            this->addChild(delMenu);
            auto* delSpr = ButtonSprite::create("Delete", "goldFont.fnt", "GJ_button_06.png", 0.55f);
            auto* delBtn = CCMenuItemSpriteExtra::create(delSpr, this, menu_selector(CustomRatesLayer::onDeletePressed));
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
        m_tab = tab;
        m_page = 0;
        updateTabVisuals();
        if (m_statusLabel) m_statusLabel->setString("Loading...");
        int token = ++m_reqSeq;
        this->retain();

        fetchLevels(tab,
            [this, token](std::vector<LevelEntry> entries) {
                if (token != m_reqSeq) { this->release(); return; }
                m_entries = std::move(entries);
                if (m_statusLabel) m_statusLabel->setString(("Loaded: " + std::to_string(m_entries.size())).c_str());
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

        int total = std::max(1, (int)((m_entries.size() + kRowsPerPage - 1) / kRowsPerPage));
        m_page = std::clamp(m_page, 0, total - 1);
        if (m_pageLabel) m_pageLabel->setString(("Page " + std::to_string(m_page + 1) + "/" + std::to_string(total)).c_str());

        int start = m_page * kRowsPerPage;
        int end = std::min(start + kRowsPerPage, (int)m_entries.size());

        if (start >= end) {
            auto* lbl = CCLabelBMFont::create("No entries yet.", "goldFont.fnt");
            lbl->setScale(0.5f);
            lbl->setColor(ccc3(180, 180, 180));
            m_listRoot->addChild(lbl);
            return;
        }

        float rowH  = 30.f;
        float listW = 340.f;
        float listH = rowH * kRowsPerPage;

        for (int i = start; i < end; ++i) {
            auto const& e = m_entries[i];
            int   rowIdx  = i - start;
            float y       = listH / 2.f - rowIdx * rowH;

            auto* rowBg = CCLayerColor::create(rowIdx % 2 == 0 ? ccc4(0,0,0,50) : ccc4(0,0,0,20), listW, rowH - 2.f);
            rowBg->setAnchorPoint({ 0.5f, 0.5f });
            rowBg->setPosition({ -listW / 2.f, y - rowH / 2.f });
            m_listRoot->addChild(rowBg, 1);

            auto* rankLbl = CCLabelBMFont::create(("#" + std::to_string(i + 1)).c_str(), "bigFont.fnt");
            rankLbl->setScale(0.26f);
            rankLbl->setColor(ccc3(255, 220, 50));
            rankLbl->setAnchorPoint({ 0.f, 0.5f });
            rankLbl->setPosition({ -listW / 2.f + 4.f, y });
            m_listRoot->addChild(rankLbl, 2);

            if (m_tab != Tab::Sent) {
                auto* diffSpr = makeDifficultyIcon(e.difficulty, 0.32f);
                if (diffSpr) {
                    diffSpr->setPosition({ -listW / 2.f + 38.f, y });
                    m_listRoot->addChild(diffSpr, 2);
                }
            }

            std::string nameStr = e.levelName.empty() ? ("ID: " + std::to_string(e.levelID)) : e.levelName;
            auto* rowMenu = CCMenu::create();
            rowMenu->setPosition({ -listW / 2.f + 58.f, y });
            m_listRoot->addChild(rowMenu, 3);
            auto* nameLblRaw = CCLabelBMFont::create(nameStr.c_str(), "bigFont.fnt");
            nameLblRaw->setScale(0.28f);
            nameLblRaw->setColor(ccc3(255, 255, 255));
            auto* nameBtn = CCMenuItemLabel::create(nameLblRaw, this, menu_selector(CustomRatesLayer::onLevelRowPressed));
            nameBtn->setTag((int)e.levelID);
            rowMenu->addChild(nameBtn);

            if (m_tab == Tab::Sent) {
                std::string sentStr = e.sentBy.empty() ? "—" : ("by " + e.sentBy);
                auto* sentLbl = CCLabelBMFont::create(sentStr.c_str(), "bigFont.fnt");
                sentLbl->setScale(0.20f);
                sentLbl->setColor(ccc3(170, 170, 170));
                sentLbl->setAnchorPoint({ 1.f, 0.5f });
                sentLbl->setPosition({ listW / 2.f - 4.f, y });
                m_listRoot->addChild(sentLbl, 2);
            } else {
                auto* typeIcon = makeRateTypeIcon(e.rateType, 0.40f);
                if (typeIcon) {
                    typeIcon->setPosition({ listW / 2.f - 58.f, y });
                    m_listRoot->addChild(typeIcon, 2);
                }
                auto* starSpr = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
                if (starSpr) {
                    starSpr->setColor(ccc3(100, 200, 255));
                    starSpr->setScale(0.45f);
                    starSpr->setPosition({ listW / 2.f - 36.f, y });
                    m_listRoot->addChild(starSpr, 2);
                }
                auto* starsLbl = CCLabelBMFont::create(std::to_string(e.blueStars).c_str(), "bigFont.fnt");
                starsLbl->setScale(0.26f);
                starsLbl->setColor(ccc3(100, 200, 255));
                starsLbl->setAnchorPoint({ 0.f, 0.5f });
                starsLbl->setPosition({ listW / 2.f - 24.f, y });
                m_listRoot->addChild(starsLbl, 2);
                auto* modLbl = CCLabelBMFont::create((e.moderator.empty() ? "?" : e.moderator).c_str(), "bigFont.fnt");
                modLbl->setScale(0.18f);
                modLbl->setColor(ccc3(90, 220, 255));
                modLbl->setAnchorPoint({ 1.f, 0.5f });
                modLbl->setPosition({ listW / 2.f - 4.f, y });
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

    void onPrevPage(CCObject*) { if (m_page > 0) { --m_page; renderPage(); } }
    void onNextPage(CCObject*) {
        int total = std::max(1, (int)((m_entries.size() + kRowsPerPage - 1) / kRowsPerPage));
        if (m_page + 1 < total) { ++m_page; renderPage(); }
    }
    void onRefresh(CCObject*) { loadTab(m_tab); }
    void onDeletePressed(CCObject*) { if (!isAdmin()) return; DeleteLevelPopup::show(); }
    void onLevelRowPressed(CCObject* sender) {
        auto* node = typeinfo_cast<CCNode*>(sender);
        if (!node) return;
        int64_t levelID = node->getTag();
        if (levelID <= 0) return;
        auto* level = GJGameLevel::create();
        level->m_levelID   = (int)levelID;
        level->m_levelName = g_nameCache.count(levelID) ? g_nameCache[levelID] : std::to_string(levelID);
        auto* scene = LevelInfoLayer::scene(level, false);
        CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, scene));
    }
public:
    static CustomRatesLayer* create() {
        auto* ret = new CustomRatesLayer();
        if (ret && ret->init()) { ret->autorelease(); return ret; }
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

class $modify(CRCreatorLayer, CreatorLayer) {
    bool init() override {
        if (!CreatorLayer::init()) return false;
        cr::bootstrap();
        if (auto* menu = this->getChildByID("creator-buttons-menu")) {
            if (auto* item = typeinfo_cast<CCMenuItemSpriteExtra*>(menu->getChildByID("featured-button"))) {
                item->setTarget(this, menu_selector(CRCreatorLayer::onCustomRates));
            }
        }
        return true;
    }
    void onCustomRates(CCObject*) { cr::CustomRatesLayer::show(); }
};

class $modify(CRLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        if (!m_level) return true;

        auto* menu = this->getChildByID("left-side-menu");
        if (!menu) {
            menu = CCMenu::create();
            menu->setID("cr-side-menu");
            auto win = CCDirector::get()->getWinSize();
            menu->setPosition({ 24.f, win.height / 2.f });
            this->addChild(menu, 10);
        }
        if (menu->getChildByID("cr-rate-btn")) return true;

        auto* spr = CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png");
        if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        if (spr) spr->setScale(0.85f);

        auto* btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(CRLevelInfoLayer::onCRButton));
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
            cr::AdminRatePopup::show(levelID);
        } else {
            auto sender = cr::currentPlayerName();
            cr::sendLevelToDb(levelID, sender,
                [](bool ok, std::string const& msg) {
                    cr::toast(msg, ok ? NotificationIcon::Success : NotificationIcon::Error);
                }
            );
        }
    }
};

class $modify(CRInfoLayer, InfoLayer) {
    bool init(GJGameLevel* level, GJUserScore* score, GJLevelList* list) {
        if (!InfoLayer::init(level, score, list)) return false;
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
                auto text = "Rated by " + info->moderator + " (" + std::to_string(info->blueStars) + " stars)";
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

class $modify(CRPlayLayer, PlayLayer) {
    void levelComplete() {
        PlayLayer::levelComplete();
        if (!m_level || m_isPracticeMode) return;
        auto levelID = cr::getLevelID(m_level);
        if (levelID <= 0) return;
        auto username = cr::currentPlayerName();
        cr::fetchRatedMeta(levelID,
            [levelID, username](std::optional<cr::LevelEntry> info) {
                if (!info || info->blueStars <= 0) return;
                cr::awardStarsForLevel(levelID, info->blueStars, username);
            }
        );
    }
};

class $modify(CRProfilePage, ProfilePage) {
    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);
        if (!score) return;
        if (this->getChildByID("cr-blue-stars-label")) return;

        auto win = CCDirector::get()->getWinSize();
        auto* label = CCLabelBMFont::create("... blue stars", "goldFont.fnt");
        label->setScale(0.38f);
        label->setColor(ccc3(100, 200, 255));
        label->setID("cr-blue-stars-label");
        label->setPosition({ win.width / 2.f + 95.f, win.height / 2.f - 52.f });
        this->addChild(label, 10);

        std::string username = score->m_userName;
        label->retain();

        cr::fetchPlayerStars(username,
            [label](int stars) {
                auto text = std::to_string(stars) + " blue stars";
                label->setString(text.c_str());
                label->release();
            }
        );
    }
};
