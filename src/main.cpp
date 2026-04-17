#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/InfoLayer.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/CreatorLayer.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/DifficultySprite.hpp>

#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/InfoLayer.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <Geode/ui/TextInput.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace geode::prelude;
using namespace geode::utils::web;

namespace cr {

// ════════════════════════════════════════════════════════════
//  Константы
// ════════════════════════════════════════════════════════════

static constexpr char const* kTursoUrlRaw =
    "libsql://custom-rates-evgen.aws-eu-west-1.turso.io";

static constexpr std::array<uint8_t,10> kXorKey = {
    0xb1,0x9a,0x94,0x8d,0x90,
    0x85,0xcd,0xcf,0xcd,0xc9
};

static constexpr uint8_t kEncryptedToken[] = {
    0xd4,0xe3,0xde,0xe5,0xf2,0xc2,0xae,0xa6,0x82,0xa0,0xfb,0xdc,0xce,0xc8,0xc2,0xd1,
    0x9c,0x9c,0x84,0xba,0xf8,0xf4,0xc6,0xb8,0xf3,0xc6,0x84,0xf9,0x84,0xa2,0xc1,0xc2,
    0xc2,0xce,0xda,0xbc,0xe3,0xaa,0xb4,0x83,0xd9,0xd3,0xfe,0xe2,0xf9,0xe6,0xa3,0xac,
    0xa4,0x85,0xf2,0xd0,0xe4,0xd4,0xc8,0xd4,0xa4,0x80,0xa7,0x8c,0x82,0xd4,0xee,0xd8,
    0xa4,0xc8,0x99,0xa4,0xfd,0x87,0xe5,0xfd,0xe7,0xc4,0xfd,0xe9,0xa6,0x86,0xa7,0xa6,
    0xd8,0xd7,0xd0,0xc8,0xa5,0xdf,0x89,0x96,0xf8,0x93,0xf6,0xd3,0xe0,0xc0,0xd4,0xe6,
    0xba,0x82,0x9e,0xf9,0x82,0xd4,0xfe,0xdb,0xfb,0xc9,0x99,0xa4,0xb7,0x90,0xe5,0xc3,
    0xe0,0xc3,0xc4,0xd0,0xb7,0x95,0x9a,0x93,0xdc,0xc3,0xc0,0xc3,0xfb,0xdc,0xa0,0x99,
    0xa1,0x80,0xd8,0xed,0xfd,0xee,0xfd,0xe9,0xa6,0x86,0xa7,0xa6,0xd8,0xd5,0xc3,0xdb,
    0xfd,0xc8,0xff,0x85,0xa4,0x87,0xdc,0xcf,0xe0,0xd4,0xfa,0xcc,0xff,0x82,0x9e,0xf9,
    0x81,0xd4,0xc0,0xcc,0xa5,0xc9,0x9a,0x89,0xa7,0x93,0xf5,0xf1,0xe0,0xc2,0xc4,0xed,
    0xa7,0x82,0x9a,0x90,0xc6,0xc0,0xc0,0xdc,0xa4,0xcb,0xa7,0x85,0xa5,0x80,0xdf,0xaa,
    0xba,0xfd,0xc2,0xcc,0xbc,0xbc,0x81,0xbd,0xd3,0xe0,0xa3,0xf4,0xfb,0xf3,0x81,0x85,
    0xac,0xf1,0xf0,0xe2,0xcb,0xbf,0xa1,0xc9,0xa3,0xe2,0xff,0x8f,0xfb,0xf3,0xde,0xa0,
    0xdf,0xc4,0xfd,0xbe,0xfe,0xe4,0x89,0xe0,0xac,0xcf,0xe8,0xfd,0x80,0x99,0xf8,0xa5,
    0xc2,0xd8,0xd2,0xf8,0xd9,0xf5,0x8e,0xa0,0xf9,0xa4,0xc1,0xc8,0xa1,0xd7,0xd8,0xbc,
    0x9b,0xf7,0xa0,0x84,0xe7,0xa2,0xe0,0xd7,0xa5,0xee,0xb4,0x96,0x98,0xe4,0xfe,0xe0,
    0xfb,0xd4,0xc7,0xb2,0xfe,0x8d,0xaa
};

static constexpr int   kRowsPerPage = 10;
static constexpr float kListW       = 356.f;
static constexpr float kListH       = 220.f;
static constexpr float kCellH       = 46.f;

// ════════════════════════════════════════════════════════════
//  Структуры
// ════════════════════════════════════════════════════════════

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

struct PlayerEntry {
    std::string username;
    int         stars = 0;
    int         rank  = 0;
};

// ════════════════════════════════════════════════════════════
//  Глобальное состояние
// ════════════════════════════════════════════════════════════

static bool        g_bootstrapped = false;
static std::string g_tursoUrl;
static std::string g_tursoToken;

static std::unordered_set<std::string> g_admins = {
    "mapperok232","kiyarikus","nenekroz","ujneft"
};
// levelID → название
static std::unordered_map<int64_t,std::string> g_nameCache;
// levelID → рейт инфо
static std::unordered_map<int64_t,LevelEntry>  g_rateCache;

using CancelToken = std::shared_ptr<std::atomic<bool>>;
static CancelToken makeCancelToken() {
    return std::make_shared<std::atomic<bool>>(false);
}

// ════════════════════════════════════════════════════════════
//  Утилиты
// ════════════════════════════════════════════════════════════

static std::string lower(std::string s) {
    std::transform(s.begin(),s.end(),s.begin(),
        [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}
static std::string escapeSql(std::string const& in) {
    std::string out; out.reserve(in.size()+8);
    for (char c:in){ if(c=='\'') out+="''"; else out+=c; }
    return out;
}
static std::string escapeJson(std::string const& in) {
    std::string out; out.reserve(in.size()+8);
    for (char c:in){
        switch(c){
            case '\\': out+="\\\\"; break;
            case '"':  out+="\\\""; break;
            case '\n': out+="\\n";  break;
            case '\r': out+="\\r";  break;
            case '\t': out+="\\t";  break;
            default:   out+=c;
        }
    }
    return out;
}
static std::string normalizeUrl(std::string url) {
    while(!url.empty()&&std::isspace((unsigned char)url.back()))  url.pop_back();
    while(!url.empty()&&std::isspace((unsigned char)url.front())) url.erase(url.begin());
    if(url.rfind("libsql://",0)==0) url="https://"+url.substr(9);
    else if(url.rfind("http://",0)!=0&&url.rfind("https://",0)!=0) url="https://"+url;
    while(!url.empty()&&url.back()=='/') url.pop_back();
    return url;
}
static std::string decryptToken() {
    std::string out; out.reserve(std::size(kEncryptedToken));
    for(size_t i=0;i<std::size(kEncryptedToken);++i)
        out.push_back((char)(kEncryptedToken[i]^kXorKey[i%kXorKey.size()]));
    return out;
}
static std::string currentPlayerName() {
    auto* gm=GameManager::sharedState();
    return gm?std::string(gm->m_playerName):"";
}
static bool isAdmin() {
    return g_admins.count(lower(currentPlayerName()))>0;
}
static void toast(std::string const& msg,
                  NotificationIcon icon=NotificationIcon::Success) {
    Notification::create(msg.c_str(),icon)->show();
}
static int64_t getLevelID(GJGameLevel* l) {
    return l?(int64_t)l->m_levelID:-1;
}

// ════════════════════════════════════════════════════════════
//  Конвертация difficulty string → GJDifficulty
// ════════════════════════════════════════════════════════════

static GJDifficulty diffStringToEnum(std::string const& d) {
    if(d=="easy")          return GJDifficulty::Easy;
    if(d=="normal")        return GJDifficulty::Normal;
    if(d=="hard")          return GJDifficulty::Hard;
    if(d=="harder")        return GJDifficulty::Harder;
    if(d=="insane")        return GJDifficulty::Insane;
    if(d=="easy_demon")    return GJDifficulty::HardDemon;   // демон-подвиды
    if(d=="med_demon")     return GJDifficulty::HardDemon;
    if(d=="hard_demon")    return GJDifficulty::HardDemon;
    if(d=="insane_demon")  return GJDifficulty::HardDemon;
    if(d=="extreme_demon") return GJDifficulty::HardDemon;
    return GJDifficulty::Auto;
}

// Возвращает demon sub-type (0=hard,1=easy,2=medium,3=insane,4=extreme)
static int demonSubType(std::string const& d) {
    if(d=="easy_demon")    return 1;
    if(d=="med_demon")     return 2;
    if(d=="insane_demon")  return 3;
    if(d=="extreme_demon") return 4;
    return 0; // hard_demon default
}

static bool isDemon(std::string const& d) {
    return d.find("demon")!=std::string::npos;
}

// ════════════════════════════════════════════════════════════
//  Иконки
// ════════════════════════════════════════════════════════════

static CCSprite* makeBlueStarIcon(float scale=1.f) {
    auto* spr=CCSprite::create("blue_star.png"_spr);
    if(!spr||spr->getContentSize().width<2.f){
        spr=CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        if(spr) spr->setColor(ccc3(100,200,255));
    }
    if(spr) spr->setScale(scale);
    return spr;
}

static CCSprite* makeRateTypeIcon(std::string const& rt,float scale=1.f) {
    const char* frame="GJ_starsIcon_001.png";
    if     (rt=="featured")  frame="GJ_featuredCoin_001.png";
    else if(rt=="epic")      frame="GJ_epicCoin_001.png";
    else if(rt=="legendary") frame="GJ_epicCoin2_001.png";
    else if(rt=="mythic")    frame="GJ_epicCoin3_001.png";
    auto* spr=CCSprite::createWithSpriteFrameName(frame);
    if(spr) spr->setScale(scale);
    return spr;
}

static CCSprite* makeDifficultyIcon(std::string const& diff,float scale=1.f) {
    const char* frame="difficulty_00_btn_001.png";
    if     (diff=="easy")          frame="difficulty_01_btn_001.png";
    else if(diff=="normal")        frame="difficulty_02_btn_001.png";
    else if(diff=="hard")          frame="difficulty_03_btn_001.png";
    else if(diff=="harder")        frame="difficulty_04_btn_001.png";
    else if(diff=="insane")        frame="difficulty_05_btn_001.png";
    else if(diff=="easy_demon")    frame="difficulty_06_btn_001.png";
    else if(diff=="med_demon")     frame="difficulty_07_btn_001.png";
    else if(diff=="hard_demon")    frame="difficulty_08_btn_001.png";
    else if(diff=="insane_demon")  frame="difficulty_09_btn_001.png";
    else if(diff=="extreme_demon") frame="difficulty_10_btn_001.png";
    auto* spr=CCSprite::createWithSpriteFrameName(frame);
    if(spr) spr->setScale(scale);
    return spr;
}

// ════════════════════════════════════════════════════════════
//  Сеть — runSql
// ════════════════════════════════════════════════════════════

static std::vector<matjson::Value> safeArray(matjson::Value const& val) {
    if(!val.isArray()) return {};
    auto res=val.asArray();
    if(!res.isOk()) return {};
    auto const& arr=res.unwrap();
    return {arr.begin(),arr.end()};
}

static std::optional<std::string> extractFirstString(matjson::Value const& root) {
    auto results=safeArray(root["results"]);
    if(results.empty()) return std::nullopt;
    auto const& first=results.front();
    if(first["error"].isObject()) return std::nullopt;
    auto rows=safeArray(first["response"]["result"]["rows"]);
    if(rows.empty()) return std::nullopt;
    auto cols=safeArray(rows.front());
    if(cols.empty()) return std::nullopt;
    auto const& cell=cols.front();
    if(cell.isString()) return cell.asString().unwrapOr("");
    if(cell.isObject()){
        auto v=cell["value"];
        if(v.isString()) return v.asString().unwrapOr("");
        if(v.isNumber()) return std::to_string(v.asInt().unwrapOr(0));
    }
    if(cell.isNumber()) return std::to_string(cell.asInt().unwrapOr(0));
    return std::nullopt;
}

static void runSql(
    std::string const& sql,
    std::function<void(matjson::Value const&)> onOk,
    std::function<void(std::string const&)>    onErr  = nullptr,
    CancelToken                                cancel = nullptr
) {
    std::string url   = g_tursoUrl+"/v2/pipeline";
    std::string body  = std::string(
        R"({"requests":[{"type":"execute","stmt":{"sql":")")+
        escapeJson(sql)+"\"}}]}";
    std::string token = g_tursoToken;

    std::thread([url,body,token,onOk,onErr,cancel](){
        if(cancel&&cancel->load()) return;
        WebRequest req;
        req.header("Authorization","Bearer "+token);
        req.header("Content-Type","application/json");
        req.bodyString(body);
        auto res=req.postSync(url);
        if(cancel&&cancel->load()) return;
        if(res.ok()){
            auto t=res.string();
            std::string text=t.isOk()?t.unwrap():"{}";
            auto parsed=matjson::parse(text).unwrapOr(matjson::Value());
            Loader::get()->queueInMainThread([onOk,parsed,cancel](){
                if(cancel&&cancel->load()) return;
                if(onOk) onOk(parsed);
            });
        } else {
            std::string err=std::string(res.errorMessage());
            if(err.empty()) err="HTTP "+std::to_string(res.code());
            Loader::get()->queueInMainThread([onErr,err,cancel](){
                if(cancel&&cancel->load()) return;
                if(onErr) onErr(err);
            });
        }
    }).detach();
}

// ════════════════════════════════════════════════════════════
//  Загрузка названий уровней — исправленный парсер GD API
// ════════════════════════════════════════════════════════════

static void fetchLevelNames(
    std::vector<int64_t> ids,
    std::function<void()> onDone,
    CancelToken cancel=nullptr
) {
    // Фильтруем только те, которых нет в кэше
    std::vector<int64_t> missing;
    for(auto id:ids)
        if(id>0&&!g_nameCache.count(id)) missing.push_back(id);

    if(missing.empty()){ if(onDone) onDone(); return; }

    // GD принимает max 100 за раз — берём первые 100
    if(missing.size()>100) missing.resize(100);

    std::string idList;
    for(size_t i=0;i<missing.size();++i){
        if(i) idList+=',';
        idList+=std::to_string(missing[i]);
    }

    // Сохраняем копию missing чтобы поставить хотя бы "ID xxx" если не найдено
    std::vector<int64_t> missingCopy=missing;

    std::string body=
        "gameVersion=22&binaryVersion=42&gdw=0"
        "&str="+idList+
        "&type=0&secret=Wmfd2893gb7";

    std::thread([body,missingCopy,onDone,cancel](){
        if(cancel&&cancel->load()) return;

        WebRequest req;
        req.header("Content-Type","application/x-www-form-urlencoded");
        req.bodyString(body);
        auto res=req.postSync(
            "https://www.boomlings.com/database/getGJLevels21.php");
        if(cancel&&cancel->load()) return;

        std::string text;
        if(res.ok()){
            auto t=res.string();
            if(t.isOk()) text=t.unwrap();
        }

        std::unordered_map<int64_t,std::string> found;

        // Ответ GD: <levels>#<creators>#<songs>
        // Levels: level1|level2|...
        // Каждый level: k1:v1:k2:v2:...
        // Поле 1 = levelID, поле 2 = levelName
        if(!text.empty()&&text!="-1"&&text.size()>2){
            // Берём секцию до первого #
            std::string levelSection=text;
            auto hp=text.find('#');
            if(hp!=std::string::npos) levelSection=text.substr(0,hp);

            // Каждый уровень разделён '|'
            size_t pos=0;
            while(pos<=levelSection.size()){
                size_t pipePos=levelSection.find('|',pos);
                std::string chunk=(pipePos==std::string::npos)
                    ?levelSection.substr(pos)
                    :levelSection.substr(pos,pipePos-pos);
                pos=(pipePos==std::string::npos)?levelSection.size()+1:pipePos+1;

                if(chunk.empty()) continue;

                // Парсим k:v пары
                std::unordered_map<int,std::string> kv;
                size_t cp=0;
                while(cp<chunk.size()){
                    // key
                    size_t c1=chunk.find(':',cp);
                    if(c1==std::string::npos) break;
                    std::string key=chunk.substr(cp,c1-cp);
                    cp=c1+1;
                    // value
                    size_t c2=chunk.find(':',cp);
                    std::string val=(c2==std::string::npos)
                        ?chunk.substr(cp)
                        :chunk.substr(cp,c2-cp);
                    cp=(c2==std::string::npos)?chunk.size():c2+1;
                    int ki=0;
                    try{ ki=std::stoi(key); } catch(...){ continue; }
                    kv[ki]=val;
                }

                // Ключ 1 = ID, ключ 2 = name
                if(kv.count(1)&&kv.count(2)){
                    int64_t id=0;
                    try{ id=std::stoll(kv[1]); } catch(...){ continue; }
                    if(id>0&&!kv[2].empty())
                        found[id]=kv[2];
                }
            }
        }

        Loader::get()->queueInMainThread([found,missingCopy,onDone,cancel](){
            if(cancel&&cancel->load()) return;
            for(auto const&[id,name]:found)
                g_nameCache[id]=name;
            // Для не найденных ставим заглушку чтобы не запрашивать повторно
            for(auto id:missingCopy)
                if(!g_nameCache.count(id))
                    g_nameCache[id]="ID "+std::to_string(id);
            if(onDone) onDone();
        });
    }).detach();
}

// ════════════════════════════════════════════════════════════
//  Парсинг БД
// ════════════════════════════════════════════════════════════

static LevelEntry parseSingleItem(matjson::Value const& item) {
    LevelEntry e;
    e.levelID   =item["level_id"].asInt().unwrapOr(0);
    e.blueStars =item["blue_stars"].asInt().unwrapOr(0);
    e.difficulty=item["difficulty"].asString().unwrapOr("normal");
    e.rateType  =item["rate_type"].asString().unwrapOr("star");
    e.moderator =item["moderator"].asString().unwrapOr("");
    e.sentBy    =item["sent_by"].asString().unwrapOr("");
    e.sentAt    =item["sent_at"].asInt().unwrapOr(0);
    e.ratedAt   =item["rated_at"].asInt().unwrapOr(0);
    if(g_nameCache.count(e.levelID))
        e.levelName=g_nameCache[e.levelID];
    return e;
}

static std::vector<LevelEntry> parseLevelsJson(std::string const& jsonStr) {
    std::vector<LevelEntry> out;
    auto parsed=matjson::parse(jsonStr).unwrapOr(matjson::Value());
    if(!parsed.isArray()) return out;
    for(auto const& item:safeArray(parsed)){
        auto e=parseSingleItem(item);
        if(e.levelID>0) out.push_back(std::move(e));
    }
    return out;
}

static std::optional<LevelEntry> parseOneLevelJson(std::string const& jsonStr) {
    auto parsed=matjson::parse(jsonStr).unwrapOr(matjson::Value());
    if(!parsed.isObject()) return std::nullopt;
    auto e=parseSingleItem(parsed);
    if(e.levelID<=0) return std::nullopt;
    return e;
}

// ════════════════════════════════════════════════════════════
//  БД операции
// ════════════════════════════════════════════════════════════

static void loadAdminsFromDb() {
    runSql(
        "SELECT COALESCE(json_group_array(lower(username)),'[]') AS data "
        "FROM admins ORDER BY username ASC;",
        [](matjson::Value const& root){
            auto data=extractFirstString(root);
            if(!data) return;
            auto parsed=matjson::parse(*data).unwrapOr(matjson::Value());
            for(auto const& item:safeArray(parsed))
                if(item.isString())
                    g_admins.insert(item.asString().unwrapOr(""));
        }
    );
}

static void fetchSentLevels(
    std::function<void(std::vector<LevelEntry>)> onOk,
    std::function<void(std::string const&)>      onErr=nullptr,
    CancelToken cancel=nullptr
) {
    runSql(
        "SELECT COALESCE(json_group_array(json_object("
        "'level_id',level_id,'sent_by',sent_by,'sent_at',sent_at"
        ")),'[]') AS data FROM "
        "(SELECT level_id,sent_by,sent_at FROM sent_levels "
        "ORDER BY sent_at DESC LIMIT 100);",
        [onOk,onErr,cancel](matjson::Value const& root){
            if(cancel&&cancel->load()) return;
            auto data=extractFirstString(root);
            if(!data){ if(onErr) onErr("DB error"); else onOk({}); return; }
            onOk(parseLevelsJson(*data));
        },
        [onOk,onErr,cancel](std::string const& e){
            if(cancel&&cancel->load()) return;
            if(onErr) onErr(e); else onOk({});
        },cancel
    );
}

static void fetchRecentLevels(
    std::function<void(std::vector<LevelEntry>)> onOk,
    std::function<void(std::string const&)>      onErr=nullptr,
    CancelToken cancel=nullptr
) {
    runSql(
        "SELECT COALESCE(json_group_array(json_object("
        "'level_id',level_id,'blue_stars',blue_stars,"
        "'difficulty',difficulty,'rate_type',rate_type,"
        "'moderator',moderator,'rated_at',rated_at"
        ")),'[]') AS data FROM "
        "(SELECT level_id,blue_stars,difficulty,rate_type,moderator,rated_at "
        "FROM rated_levels ORDER BY rated_at DESC LIMIT 100);",
        [onOk,onErr,cancel](matjson::Value const& root){
            if(cancel&&cancel->load()) return;
            auto data=extractFirstString(root);
            if(!data){ if(onErr) onErr("DB error"); else onOk({}); return; }
            onOk(parseLevelsJson(*data));
        },
        [onOk,onErr,cancel](std::string const& e){
            if(cancel&&cancel->load()) return;
            if(onErr) onErr(e); else onOk({});
        },cancel
    );
}

static void fetchRandomLevels(
    std::function<void(std::vector<LevelEntry>)> onOk,
    std::function<void(std::string const&)>      onErr=nullptr,
    CancelToken cancel=nullptr
) {
    runSql(
        "SELECT COALESCE(json_group_array(json_object("
        "'level_id',level_id,'blue_stars',blue_stars,"
        "'difficulty',difficulty,'rate_type',rate_type,"
        "'moderator',moderator,'rated_at',rated_at"
        ")),'[]') AS data FROM "
        "(SELECT level_id,blue_stars,difficulty,rate_type,moderator,rated_at "
        "FROM rated_levels ORDER BY RANDOM() LIMIT 20);",
        [onOk,onErr,cancel](matjson::Value const& root){
            if(cancel&&cancel->load()) return;
            auto data=extractFirstString(root);
            if(!data){ if(onErr) onErr("DB error"); else onOk({}); return; }
            onOk(parseLevelsJson(*data));
        },
        [onOk,onErr,cancel](std::string const& e){
            if(cancel&&cancel->load()) return;
            if(onErr) onErr(e); else onOk({});
        },cancel
    );
}

static void fetchTopPlayers(
    std::function<void(std::vector<PlayerEntry>)> onOk,
    std::function<void(std::string const&)>       onErr=nullptr,
    CancelToken cancel=nullptr
) {
    runSql(
        "SELECT COALESCE(json_group_array(json_object("
        "'username',username,'stars',stars"
        ")),'[]') AS data FROM "
        "(SELECT username,stars FROM player_stars "
        "ORDER BY stars DESC LIMIT 100);",
        [onOk,onErr,cancel](matjson::Value const& root){
            if(cancel&&cancel->load()) return;
            auto data=extractFirstString(root);
            if(!data){ if(onErr) onErr("DB error"); else onOk({}); return; }
            auto parsed=matjson::parse(*data).unwrapOr(matjson::Value());
            std::vector<PlayerEntry> out;
            int rank=1;
            for(auto const& item:safeArray(parsed)){
                PlayerEntry p;
                p.username=item["username"].asString().unwrapOr("?");
                p.stars   =item["stars"].asInt().unwrapOr(0);
                p.rank    =rank++;
                out.push_back(std::move(p));
            }
            onOk(out);
        },
        [onOk,onErr,cancel](std::string const& e){
            if(cancel&&cancel->load()) return;
            if(onErr) onErr(e); else onOk({});
        },cancel
    );
}

static void fetchRatedMeta(
    int64_t levelID,
    std::function<void(std::optional<LevelEntry>)> onOk,
    std::function<void(std::string const&)>        onErr=nullptr,
    CancelToken cancel=nullptr
) {
    // Сначала проверяем кэш
    if(g_rateCache.count(levelID)){
        onOk(g_rateCache[levelID]);
        return;
    }

    runSql(
        "SELECT json_object("
        "'level_id',level_id,'blue_stars',blue_stars,"
        "'difficulty',difficulty,'rate_type',rate_type,"
        "'moderator',moderator,'rated_at',rated_at"
        ") AS data FROM rated_levels WHERE level_id="+
        std::to_string(levelID)+" LIMIT 1;",
        [levelID,onOk,cancel](matjson::Value const& root){
            if(cancel&&cancel->load()) return;
            auto data=extractFirstString(root);
            if(!data){ onOk(std::nullopt); return; }
            auto entry=parseOneLevelJson(*data);
            if(entry) g_rateCache[levelID]=*entry;
            onOk(entry);
        },
        [onOk,onErr,cancel](std::string const& e){
            if(cancel&&cancel->load()) return;
            if(onErr) onErr(e); else onOk(std::nullopt);
        },cancel
    );
}

static void sendLevelToDb(
    int64_t levelID,std::string const& sender,
    std::function<void(bool,std::string const&)> onDone
) {
    runSql(
        "INSERT OR IGNORE INTO sent_levels (level_id,sent_by,sent_at) VALUES ("+
        std::to_string(levelID)+",'"+escapeSql(sender)+"',unixepoch());",
        [onDone](matjson::Value const&){ onDone(true,"Level sent for review!"); },
        [onDone](std::string const& e){ onDone(false,e); }
    );
}

static void deleteSentFromDb(int64_t levelID,
    std::function<void(bool,std::string const&)> onDone=nullptr)
{
    runSql(
        "DELETE FROM sent_levels WHERE level_id="+std::to_string(levelID)+";",
        [onDone](matjson::Value const&){ if(onDone) onDone(true,"Removed."); },
        [onDone](std::string const& e){ if(onDone) onDone(false,e); }
    );
}

static void deleteRatedFromDb(int64_t levelID,
    std::function<void(bool,std::string const&)> onDone)
{
    g_rateCache.erase(levelID);
    runSql(
        "DELETE FROM rated_levels WHERE level_id="+std::to_string(levelID)+";",
        [onDone](matjson::Value const&){ onDone(true,"Rating removed."); },
        [onDone](std::string const& e){ onDone(false,e); }
    );
}

static void rateLevelInDb(
    int64_t levelID,int blueStars,
    std::string const& difficulty,std::string const& rateType,
    std::string const& moderator,
    std::function<void(bool,std::string const&)> onDone
) {
    g_rateCache.erase(levelID);
    runSql(
        "DELETE FROM sent_levels WHERE level_id="+std::to_string(levelID)+";",
        [=](matjson::Value const&){
            runSql(
                "INSERT OR REPLACE INTO rated_levels "
                "(level_id,blue_stars,difficulty,rate_type,moderator,rated_at)"
                " VALUES ("+
                std::to_string(levelID)+","+
                std::to_string(blueStars)+",'"+
                escapeSql(difficulty)+"','"+
                escapeSql(rateType)+"','"+
                escapeSql(moderator)+"',unixepoch());",
                [onDone](matjson::Value const&){ onDone(true,"Level rated!"); },
                [onDone](std::string const& e){ onDone(false,e); }
            );
        },
        [onDone](std::string const& e){ onDone(false,e); }
    );
}

static void awardStarsForLevel(
    int64_t levelID,int blueStars,std::string const& username)
{
    if(username.empty()||levelID<=0||blueStars<=0) return;
    runSql(
        "SELECT COUNT(*) FROM completed_levels WHERE username='"+
        escapeSql(lower(username))+"' AND level_id="+std::to_string(levelID)+";",
        [=](matjson::Value const& root){
            auto val=extractFirstString(root);
            if(val&&*val!="0"&&!val->empty()) return;
            runSql(
                "INSERT OR IGNORE INTO completed_levels "
                "(username,level_id,completed_at) VALUES ('"+
                escapeSql(lower(username))+"',"+std::to_string(levelID)+
                ",unixepoch());",
                [=](matjson::Value const&){
                    runSql(
                        "INSERT INTO player_stars (username,stars,updated_at)"
                        " VALUES ('"+escapeSql(lower(username))+"',"+
                        std::to_string(blueStars)+
                        ",unixepoch()) ON CONFLICT(username) DO UPDATE"
                        " SET stars=stars+"+std::to_string(blueStars)+
                        ",updated_at=unixepoch();",
                        nullptr,nullptr
                    );
                }
            );
        }
    );
}

static void fetchPlayerStars(
    std::string const& username,
    std::function<void(int)> onDone,
    CancelToken cancel=nullptr
) {
    runSql(
        "SELECT stars FROM player_stars WHERE username='"+
        escapeSql(lower(username))+"' LIMIT 1;",
        [onDone,cancel](matjson::Value const& root){
            if(cancel&&cancel->load()) return;
            auto val=extractFirstString(root);
            int stars=0;
            if(val) try{ stars=std::stoi(*val); } catch(...){}
            onDone(stars);
        },
        [onDone,cancel](std::string const&){
            if(cancel&&cancel->load()) return;
            onDone(0);
        },cancel
    );
}

static void bootstrap() {
    if(g_bootstrapped) return;
    g_bootstrapped=true;
    g_tursoUrl  =normalizeUrl(kTursoUrlRaw);
    g_tursoToken=decryptToken();
    loadAdminsFromDb();
}

// ════════════════════════════════════════════════════════════
//  Применение рейта к GJGameLevel
//  Вызывается чтобы DifficultySprite обновилась
// ════════════════════════════════════════════════════════════

static void applyRateToLevel(GJGameLevel* level, LevelEntry const& e) {
    if(!level) return;
    // Звёзды
    level->m_stars    = e.blueStars;
    level->m_starRatings = e.blueStars;
    // Сложность
    if(isDemon(e.difficulty)){
        level->m_difficulty        = GJDifficulty::HardDemon;
        level->m_demonDifficulty   = demonSubType(e.difficulty);
        level->m_demon             = 1;
    } else {
        level->m_difficulty      = diffStringToEnum(e.difficulty);
        level->m_demon           = 0;
        level->m_demonDifficulty = 0;
    }
    // Рейт тип
    level->m_featured  = (e.rateType!="star") ? 1 : 0;
    level->m_isEpic    = (e.rateType=="epic"||e.rateType=="legendary"||e.rateType=="mythic") ? 1 : 0;
}

// ════════════════════════════════════════════════════════════
//  CRScrollLayer
// ════════════════════════════════════════════════════════════

class CRScrollLayer : public CCLayer {
    CCLayer* m_cont   = nullptr;
    float    m_totalH = 0.f;
    float    m_viewH  = 0.f;
    float    m_viewW  = 0.f;
    CCPoint  m_last   = {};
    bool     m_drag   = false;

    bool init(float w,float h){
        if(!CCLayer::init()) return false;
        m_viewW=w; m_viewH=h;
        setContentSize({w,h});
        m_cont=CCLayer::create();
        m_cont->setPosition({0.f,0.f});
        addChild(m_cont,1);
        setTouchEnabled(true);
        setTouchMode(kCCTouchesOneByOne);
        return true;
    }
    bool ccTouchBegan(CCTouch* t,CCEvent*) override {
        auto l=convertTouchToNodeSpace(t);
        if(l.x<0||l.x>m_viewW||l.y<0||l.y>m_viewH) return false;
        m_last=l; m_drag=true; return true;
    }
    void ccTouchMoved(CCTouch* t,CCEvent*) override {
        if(!m_drag||!m_cont) return;
        auto l=convertTouchToNodeSpace(t);
        float dy=l.y-m_last.y; m_last=l;
        float ny=m_cont->getPositionY()+dy;
        float mx=std::max(0.f,m_totalH-m_viewH);
        m_cont->setPositionY(std::clamp(ny,0.f,mx));
    }
    void ccTouchEnded(CCTouch*,CCEvent*) override { m_drag=false; }
    void ccTouchCancelled(CCTouch* t,CCEvent* e) override { ccTouchEnded(t,e); }
public:
    static CRScrollLayer* create(float w,float h){
        auto* r=new CRScrollLayer();
        if(r&&r->init(w,h)){ r->autorelease(); return r; }
        CC_SAFE_DELETE(r); return nullptr;
    }
    void clearCells(){
        if(m_cont) m_cont->removeAllChildrenWithCleanup(true);
        m_totalH=0.f;
        if(m_cont) m_cont->setPositionY(0.f);
    }
    void addCellAtY(CCLayer* cell,float y){
        if(!m_cont||!cell) return;
        cell->setPosition({0.f,y});
        m_cont->addChild(cell);
    }
    void setTotalHeight(float h){ m_totalH=h; }
};

// ════════════════════════════════════════════════════════════
//  LevelCell
// ════════════════════════════════════════════════════════════

class LevelCell : public CCLayer {
    LevelEntry m_entry;

    bool init(LevelEntry const& e,bool isSent,float w){
        if(!CCLayer::init()) return false;
        m_entry=e;
        setContentSize({w,kCellH});

        auto* bg=CCLayerColor::create(ccc4(0,0,0,55),w,kCellH);
        bg->setPosition({0,0}); addChild(bg,0);
        auto* line=CCLayerColor::create(ccc4(255,255,255,18),w,1.f);
        line->setPosition({0,0}); addChild(line,1);

        float cy=kCellH/2.f;
        float tx=isSent?8.f:42.f;

        if(!isSent){
            auto* di=makeDifficultyIcon(e.difficulty,0.58f);
            if(di){ di->setPosition({18.f,cy}); addChild(di,2); }
        }

        // Название — уже должно быть в e.levelName после fetchLevelNames
        std::string nm=e.levelName.empty()
            ?("ID "+std::to_string(e.levelID))
            :e.levelName;
        if(nm.size()>19) nm=nm.substr(0,18)+"…";

        auto* nl=CCLabelBMFont::create(nm.c_str(),"bigFont.fnt");
        nl->setScale(0.46f);
        nl->setColor(ccc3(255,255,255));
        nl->setAnchorPoint({0.f,0.5f});
        nl->setPosition({tx,cy+11.f});
        addChild(nl,2);

        if(isSent){
            std::string by=e.sentBy.empty()?"Unknown":e.sentBy;
            auto* bl=CCLabelBMFont::create(("by "+by).c_str(),"bigFont.fnt");
            bl->setScale(0.28f);
            bl->setColor(ccc3(150,200,255));
            bl->setAnchorPoint({0.f,0.5f});
            bl->setPosition({tx,cy-9.f});
            addChild(bl,2);
        } else {
            std::string mod=e.moderator.empty()?"?":e.moderator;
            auto* ml=CCLabelBMFont::create(("by "+mod).c_str(),"bigFont.fnt");
            ml->setScale(0.26f);
            ml->setColor(ccc3(80,210,255));
            ml->setAnchorPoint({0.f,0.5f});
            ml->setPosition({tx,cy-9.f});
            addChild(ml,2);

            // Тип рейта + синяя звезда + число звёзд — справа
            float rx=w-68.f;
            auto* ti=makeRateTypeIcon(e.rateType,0.42f);
            if(ti){ ti->setPosition({rx,cy}); addChild(ti,2); }
            auto* si=makeBlueStarIcon(0.42f);
            if(si){ si->setPosition({rx+18.f,cy}); addChild(si,2); }
            auto* sl=CCLabelBMFont::create(
                std::to_string(e.blueStars).c_str(),"bigFont.fnt");
            sl->setScale(0.34f);
            sl->setColor(ccc3(100,200,255));
            sl->setAnchorPoint({0.f,0.5f});
            sl->setPosition({rx+30.f,cy});
            addChild(sl,2);
        }

        auto* menu=CCMenu::create();
        menu->setPosition({0.f,0.f}); addChild(menu,3);
        auto* vb=CCMenuItemSpriteExtra::create(
            ButtonSprite::create("View","goldFont.fnt","GJ_button_01.png",0.5f),
            this,menu_selector(LevelCell::onView));
        vb->setPosition({w-24.f,cy-8.f});
        menu->addChild(vb);
        return true;
    }
    void onView(CCObject*){
        auto* lv=GJGameLevel::create();
        lv->m_levelID  =(int)m_entry.levelID;
        lv->m_levelName=m_entry.levelName.empty()
            ?std::to_string(m_entry.levelID)
            :m_entry.levelName;
        CCDirector::get()->pushScene(
            CCTransitionFade::create(0.5f,LevelInfoLayer::scene(lv,false)));
    }
public:
    static LevelCell* create(LevelEntry const& e,bool isSent,float w){
        auto* r=new LevelCell();
        if(r&&r->init(e,isSent,w)){ r->autorelease(); return r; }
        CC_SAFE_DELETE(r); return nullptr;
    }
};

// ════════════════════════════════════════════════════════════
//  PlayerCell
// ════════════════════════════════════════════════════════════

class PlayerCell : public CCLayer {
    bool init(PlayerEntry const& p,float w){
        if(!CCLayer::init()) return false;
        setContentSize({w,kCellH});
        auto* bg=CCLayerColor::create(ccc4(0,0,0,50),w,kCellH);
        bg->setPosition({0,0}); addChild(bg,0);
        auto* line=CCLayerColor::create(ccc4(255,255,255,18),w,1.f);
        line->setPosition({0,0}); addChild(line,1);
        float cy=kCellH/2.f;

        auto* rl=CCLabelBMFont::create(
            ("#"+std::to_string(p.rank)).c_str(),"bigFont.fnt");
        rl->setScale(0.48f);
        rl->setColor(p.rank==1?ccc3(255,215,0):
                     p.rank==2?ccc3(200,200,200):
                     p.rank==3?ccc3(205,127,50):ccc3(255,255,255));
        rl->setAnchorPoint({0.f,0.5f});
        rl->setPosition({6.f,cy});
        addChild(rl,2);

        auto* nl=CCLabelBMFont::create(p.username.c_str(),"bigFont.fnt");
        nl->setScale(0.46f);
        nl->setColor(ccc3(255,255,255));
        nl->setAnchorPoint({0.f,0.5f});
        nl->setPosition({52.f,cy});
        addChild(nl,2);

        auto* si=makeBlueStarIcon(0.42f);
        if(si){ si->setPosition({w-50.f,cy}); addChild(si,2); }
        auto* sl=CCLabelBMFont::create(
            std::to_string(p.stars).c_str(),"bigFont.fnt");
        sl->setScale(0.42f);
        sl->setColor(ccc3(100,200,255));
        sl->setAnchorPoint({0.f,0.5f});
        sl->setPosition({w-36.f,cy});
        addChild(sl,2);
        return true;
    }
public:
    static PlayerCell* create(PlayerEntry const& p,float w){
        auto* r=new PlayerCell();
        if(r&&r->init(p,w)){ r->autorelease(); return r; }
        CC_SAFE_DELETE(r); return nullptr;
    }
};

// ════════════════════════════════════════════════════════════
//  Общий базовый класс для list-сцен
// ════════════════════════════════════════════════════════════

class CRBaseScene : public CCLayer {
protected:
    CRScrollLayer* m_scroll    = nullptr;
    CCLabelBMFont* m_pageLbl   = nullptr;
    CCLabelBMFont* m_statusLbl = nullptr;
    int            m_page      = 0;

    // Создаёт общий каркас: фон, углы, заголовок, рамку+скролл,
    // статус, страницу, навигацию, кнопку назад
    bool initBase(const char* title) {
        if(!CCLayer::init()) return false;
        auto win=CCDirector::get()->getWinSize();
        float cx=win.width/2.f, cy=win.height/2.f;

        // Фон
        auto* bg=CCSprite::create("GJ_gradientBG.png");
        if(bg){
            bg->setScaleX(win.width/bg->getContentSize().width);
            bg->setScaleY(win.height/bg->getContentSize().height);
            bg->setPosition({cx,cy});
            bg->setColor(ccc3(0,102,255));
            addChild(bg,-1);
        }
        // Углы
        for(int fx=0;fx<2;++fx)
        for(int fy=0;fy<2;++fy){
            auto* s=CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
            if(!s) continue;
            s->setFlipX(fx); s->setFlipY(fy);
            s->setPosition({fx?win.width:0.f, fy?win.height:0.f});
            addChild(s,1);
        }
        // Заголовок
        auto* tl=CCLabelBMFont::create(title,"goldFont.fnt");
        tl->setScale(0.85f);
        tl->setPosition({cx,win.height-28.f});
        addChild(tl,2);

        // Рамка
        float lx=cx-kListW/2.f;
        float ly=cy-kListH/2.f+12.f;
        auto* frame=CCScale9Sprite::create("GJ_square01.png");
        frame->setContentSize({kListW+8.f,kListH+8.f});
        frame->setPosition({cx,cy+12.f});
        addChild(frame,1);

        // Скролл
        m_scroll=CRScrollLayer::create(kListW,kListH);
        m_scroll->setPosition({lx,ly});
        addChild(m_scroll,2);

        // Статус
        m_statusLbl=CCLabelBMFont::create("Loading...","goldFont.fnt");
        m_statusLbl->setScale(0.28f);
        m_statusLbl->setPosition({cx,ly-14.f});
        addChild(m_statusLbl,2);

        // Страница
        m_pageLbl=CCLabelBMFont::create("Page 1/1","goldFont.fnt");
        m_pageLbl->setScale(0.28f);
        m_pageLbl->setPosition({cx,ly-26.f});
        addChild(m_pageLbl,2);

        // Навигация
        auto* nav=CCMenu::create();
        nav->setPosition({cx,ly-40.f});
        addChild(nav,2);

        auto* ps=CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        if(ps) ps->setFlipX(true);
        auto* pb=CCMenuItemSpriteExtra::create(
            ps,this,menu_selector(CRBaseScene::onPrev));
        pb->setPositionX(-60.f); nav->addChild(pb);

        auto* ns=CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto* nb=CCMenuItemSpriteExtra::create(
            ns,this,menu_selector(CRBaseScene::onNext));
        nb->setPositionX(60.f); nav->addChild(nb);

        auto* rs=CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        auto* rb=CCMenuItemSpriteExtra::create(
            rs,this,menu_selector(CRBaseScene::onRefresh));
        rb->setPositionX(0.f); nav->addChild(rb);

        // Назад
        auto* bm=CCMenu::create();
        bm->setPosition({0.f,0.f}); addChild(bm,2);
        auto* bs=CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        auto* bb=CCMenuItemSpriteExtra::create(
            bs,this,menu_selector(CRBaseScene::onBack));
        bb->setPosition({28.f,win.height-28.f});
        bm->addChild(bb);

        setKeypadEnabled(true);
        return true;
    }

    void updatePageLabel(int total){
        if(m_pageLbl)
            m_pageLbl->setString(
                ("Page "+std::to_string(m_page+1)+"/"+
                 std::to_string(total)).c_str());
    }

    virtual void onPrev(CCObject*) {}
    virtual void onNext(CCObject*) {}
    virtual void onRefresh(CCObject*) {}
    void onBack(CCObject*){ CCDirector::get()->popScene(); }
    void keyBackClicked() override { CCDirector::get()->popScene(); }
};

// ════════════════════════════════════════════════════════════
//  CRListScene — Sent / Recent / Random
// ════════════════════════════════════════════════════════════

class CRListScene : public CRBaseScene {
public:
    enum class Mode { Sent, Recent, Random };
private:
    Mode   m_mode;
    int    m_reqSeq = 0;
    CancelToken m_cancel;
    std::vector<LevelEntry> m_entries;

    bool init(Mode mode){
        m_mode=mode;
        const char* title=
            mode==Mode::Sent  ?"Sent Levels":
            mode==Mode::Recent?"Recent Rates":"Random Levels";
        if(!initBase(title)) return false;
        loadData();
        return true;
    }

    void loadData(){
        if(m_statusLbl) m_statusLbl->setString("Loading...");
        if(m_cancel) m_cancel->store(true);
        m_cancel=makeCancelToken();
        int seq=++m_reqSeq;
        auto cancel=m_cancel;
        retain();

        auto onGot=[this,seq,cancel](std::vector<LevelEntry> entries){
            if(cancel->load()||seq!=m_reqSeq){ release(); return; }
            m_entries=std::move(entries);

            // Собираем ID для загрузки имён
            std::vector<int64_t> ids;
            ids.reserve(m_entries.size());
            for(auto const& e:m_entries) ids.push_back(e.levelID);

            fetchLevelNames(ids,[this,seq,cancel](){
                if(cancel->load()||seq!=m_reqSeq){ release(); return; }
                // Подставляем имена из кэша
                for(auto& e:m_entries)
                    if(g_nameCache.count(e.levelID))
                        e.levelName=g_nameCache[e.levelID];
                if(m_statusLbl)
                    m_statusLbl->setString(
                        ("Loaded: "+std::to_string(m_entries.size())).c_str());
                release();
                rebuildList();
            },cancel);
        };
        auto onErr=[this,seq,cancel](std::string const& err){
            if(cancel->load()||seq!=m_reqSeq){ release(); return; }
            m_entries.clear();
            if(m_statusLbl) m_statusLbl->setString(err.c_str());
            release();
            rebuildList();
        };

        switch(m_mode){
            case Mode::Sent:   fetchSentLevels(onGot,onErr,cancel);   break;
            case Mode::Recent: fetchRecentLevels(onGot,onErr,cancel); break;
            case Mode::Random: fetchRandomLevels(onGot,onErr,cancel); break;
        }
    }

    void rebuildList(){
        if(!m_scroll) return;
        m_scroll->clearCells();
        int total=std::max(1,(int)((m_entries.size()+kRowsPerPage-1)/kRowsPerPage));
        m_page=std::clamp(m_page,0,total-1);
        updatePageLabel(total);
        int start=m_page*kRowsPerPage;
        int end=std::min(start+kRowsPerPage,(int)m_entries.size());
        bool isSent=(m_mode==Mode::Sent);
        if(start>=end){ if(m_statusLbl) m_statusLbl->setString("No entries yet."); return; }
        float totalH=(end-start)*kCellH;
        m_scroll->setTotalHeight(totalH);
        for(int i=start;i<end;++i){
            float y=totalH-(i-start+1)*kCellH;
            auto* cell=LevelCell::create(m_entries[i],isSent,kListW);
            if(cell) m_scroll->addCellAtY(cell,y);
        }
    }

    void onPrev(CCObject*) override {
        if(m_page>0){ --m_page; rebuildList(); }
    }
    void onNext(CCObject*) override {
        int total=std::max(1,(int)((m_entries.size()+kRowsPerPage-1)/kRowsPerPage));
        if(m_page+1<total){ ++m_page; rebuildList(); }
    }
    void onRefresh(CCObject*) override { m_page=0; loadData(); }

    void onExit() override {
        if(m_cancel) m_cancel->store(true);
        CRBaseScene::onExit();
    }
public:
    static CCScene* scene(Mode mode){
        auto* sc=CCScene::create();
        auto* ly=new CRListScene();
        ly->m_mode=mode; // до init чтобы заголовок правильный
        if(ly&&ly->init(mode)){ ly->autorelease(); sc->addChild(ly); }
        return sc;
    }
};

// ════════════════════════════════════════════════════════════
//  CRTopScene — топ игроков
// ════════════════════════════════════════════════════════════

class CRTopScene : public CRBaseScene {
    int    m_reqSeq = 0;
    CancelToken m_cancel;
    std::vector<PlayerEntry> m_players;

    bool init(){
        if(!initBase("Top Players")) return false;
        loadData(); return true;
    }
    void loadData(){
        if(m_statusLbl) m_statusLbl->setString("Loading...");
        if(m_cancel) m_cancel->store(true);
        m_cancel=makeCancelToken();
        int seq=++m_reqSeq;
        auto cancel=m_cancel;
        retain();
        fetchTopPlayers(
            [this,seq,cancel](std::vector<PlayerEntry> p){
                if(cancel->load()||seq!=m_reqSeq){ release(); return; }
                m_players=std::move(p);
                if(m_statusLbl)
                    m_statusLbl->setString(
                        ("Players: "+std::to_string(m_players.size())).c_str());
                release(); rebuildList();
            },
            [this,seq,cancel](std::string const& e){
                if(cancel->load()||seq!=m_reqSeq){ release(); return; }
                m_players.clear();
                if(m_statusLbl) m_statusLbl->setString(e.c_str());
                release(); rebuildList();
            },m_cancel
        );
    }
    void rebuildList(){
        if(!m_scroll) return;
        m_scroll->clearCells();
        int total=std::max(1,(int)((m_players.size()+kRowsPerPage-1)/kRowsPerPage));
        m_page=std::clamp(m_page,0,total-1);
        updatePageLabel(total);
        int start=m_page*kRowsPerPage;
        int end=std::min(start+kRowsPerPage,(int)m_players.size());
        if(start>=end){ if(m_statusLbl) m_statusLbl->setString("No players yet."); return; }
        float totalH=(end-start)*kCellH;
        m_scroll->setTotalHeight(totalH);
        for(int i=start;i<end;++i){
            float y=totalH-(i-start+1)*kCellH;
            auto* cell=PlayerCell::create(m_players[i],kListW);
            if(cell) m_scroll->addCellAtY(cell,y);
        }
    }
    void onPrev(CCObject*) override {
        if(m_page>0){ --m_page; rebuildList(); }
    }
    void onNext(CCObject*) override {
        int total=std::max(1,(int)((m_players.size()+kRowsPerPage-1)/kRowsPerPage));
        if(m_page+1<total){ ++m_page; rebuildList(); }
    }
    void onRefresh(CCObject*) override { m_page=0; loadData(); }
    void onExit() override {
        if(m_cancel) m_cancel->store(true);
        CRBaseScene::onExit();
    }
public:
    static CCScene* scene(){
        auto* sc=CCScene::create();
        auto* ly=new CRTopScene();
        if(ly&&ly->init()){ ly->autorelease(); sc->addChild(ly); }
        return sc;
    }
};

// ════════════════════════════════════════════════════════════
//  DeleteLevelPopup
// ════════════════════════════════════════════════════════════

class DeleteLevelPopup : public CCLayer {
    geode::TextInput* m_input=nullptr;
    bool init() override {
        if(!CCLayer::init()) return false;
        auto win=CCDirector::get()->getWinSize();
        float cx=win.width/2.f, cy=win.height/2.f;
        addChild(CCLayerColor::create(ccc4(0,0,0,150)),0);
        auto* panel=CCScale9Sprite::create("GJ_square01.png");
        panel->setContentSize({280.f,160.f});
        panel->setPosition({cx,cy}); addChild(panel,1);
        auto* title=CCLabelBMFont::create("Remove Rating","goldFont.fnt");
        title->setScale(0.7f); title->setPosition({cx,cy+55.f}); addChild(title,2);
        auto* lbl=CCLabelBMFont::create("Enter Level ID:","bigFont.fnt");
        lbl->setScale(0.45f); lbl->setPosition({cx,cy+20.f}); addChild(lbl,2);
        m_input=geode::TextInput::create(200.f,"Level ID");
        m_input->setFilter("0123456789");
        m_input->setPosition({cx,cy-15.f}); addChild(m_input,2);
        auto* menu=CCMenu::create();
        menu->setPosition({0.f,0.f}); addChild(menu,2);
        auto* cb=CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Cancel","goldFont.fnt","GJ_button_06.png",0.7f),
            this,menu_selector(DeleteLevelPopup::onCancel));
        cb->setPosition({cx-70.f,cy-55.f}); menu->addChild(cb);
        auto* db=CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Delete","goldFont.fnt","GJ_button_01.png",0.7f),
            this,menu_selector(DeleteLevelPopup::onConfirm));
        db->setPosition({cx+70.f,cy-55.f}); menu->addChild(db);
        setKeypadEnabled(true); return true;
    }
    void keyBackClicked() override { removeFromParentAndCleanup(true); }
    void onCancel(CCObject*) { removeFromParentAndCleanup(true); }
    void onConfirm(CCObject*){
        if(!m_input) return;
        std::string text=std::string(m_input->getString());
        if(text.empty()) return;
        int64_t id=0;
        try{ id=std::stoll(text); } catch(...){ return; }
        if(id<=0) return;
        deleteRatedFromDb(id,[](bool ok,std::string const& msg){
            toast(msg,ok?NotificationIcon::Success:NotificationIcon::Error);
        });
        deleteSentFromDb(id);
        removeFromParentAndCleanup(true);
    }
public:
    static DeleteLevelPopup* create(){
        auto* r=new DeleteLevelPopup();
        if(r&&r->init()){ r->autorelease(); return r; }
        CC_SAFE_DELETE(r); return nullptr;
    }
    static void show(){
        auto* s=CCDirector::get()->getRunningScene();
        if(s){ auto* p=create(); if(p) s->addChild(p,99999); }
    }
};

// ════════════════════════════════════════════════════════════
//  AdminRatePopup
// ════════════════════════════════════════════════════════════

class AdminRatePopup : public CCLayer {
    int64_t m_levelID=0;
    int m_stars=1, m_diffIdx=2, m_typeIdx=0;
    CCLabelBMFont* m_starsLbl=nullptr;
    CCMenuItemSpriteExtra* m_diffBtn=nullptr;
    CCMenuItemSpriteExtra* m_typeBtn=nullptr;

    static constexpr const char* kDiffs[]=
        {"auto","easy","normal","hard","harder","insane",
         "easy_demon","med_demon","hard_demon","insane_demon","extreme_demon"};
    static constexpr const char* kTypes[]=
        {"star","featured","epic","legendary","mythic"};

    bool init(int64_t id){
        if(!CCLayer::init()) return false;
        m_levelID=id;
        auto win=CCDirector::get()->getWinSize();
        float cx=win.width/2.f, cy=win.height/2.f;
        addChild(CCLayerColor::create(ccc4(0,0,0,150)),0);
        auto* panel=CCScale9Sprite::create("GJ_square01.png");
        panel->setContentSize({300.f,300.f});
        panel->setPosition({cx,cy}); addChild(panel,1);
        auto* title=CCLabelBMFont::create("Rate Level","goldFont.fnt");
        title->setScale(0.75f); title->setPosition({cx,cy+130.f}); addChild(title,2);
        auto* menu=CCMenu::create();
        menu->setPosition({0.f,0.f}); addChild(menu,2);

        auto* st=CCLabelBMFont::create("Blue Stars","bigFont.fnt");
        st->setScale(0.45f); st->setPosition({cx,cy+95.f}); addChild(st,2);
        m_starsLbl=CCLabelBMFont::create("1","bigFont.fnt");
        m_starsLbl->setScale(0.85f);
        m_starsLbl->setColor(ccc3(100,200,255));
        m_starsLbl->setPosition({cx,cy+63.f}); addChild(m_starsLbl,2);

        auto* ms=CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
        if(ms) ms->setScale(0.7f);
        auto* mb=CCMenuItemSpriteExtra::create(
            ms,this,menu_selector(AdminRatePopup::onMinus));
        mb->setPosition({cx-38.f,cy+63.f}); menu->addChild(mb);
        auto* ps=CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        if(ps) ps->setScale(0.7f);
        auto* pb=CCMenuItemSpriteExtra::create(
            ps,this,menu_selector(AdminRatePopup::onPlus));
        pb->setPosition({cx+38.f,cy+63.f}); menu->addChild(pb);

        auto* dt=CCLabelBMFont::create("Difficulty","bigFont.fnt");
        dt->setScale(0.45f); dt->setPosition({cx,cy+25.f}); addChild(dt,2);
        auto* di=makeDifficultyIcon(kDiffs[m_diffIdx],0.85f);
        if(!di) di=CCSprite::createWithSpriteFrameName("difficulty_02_btn_001.png");
        m_diffBtn=CCMenuItemSpriteExtra::create(
            di,this,menu_selector(AdminRatePopup::onDiff));
        m_diffBtn->setPosition({cx,cy-8.f}); menu->addChild(m_diffBtn);

        auto* tt=CCLabelBMFont::create("Rate Type","bigFont.fnt");
        tt->setScale(0.45f); tt->setPosition({cx,cy-45.f}); addChild(tt,2);
        auto* ti=makeRateTypeIcon(kTypes[m_typeIdx],0.85f);
        if(!ti) ti=CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        m_typeBtn=CCMenuItemSpriteExtra::create(
            ti,this,menu_selector(AdminRatePopup::onType));
        m_typeBtn->setPosition({cx,cy-78.f}); menu->addChild(m_typeBtn);

        auto* cb=CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Cancel","goldFont.fnt","GJ_button_06.png",0.7f),
            this,menu_selector(AdminRatePopup::onCancel));
        cb->setPosition({cx-75.f,cy-125.f}); menu->addChild(cb);
        auto* rb=CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Rate!","goldFont.fnt","GJ_button_01.png",0.7f),
            this,menu_selector(AdminRatePopup::onConfirm));
        rb->setPosition({cx+75.f,cy-125.f}); menu->addChild(rb);
        setKeypadEnabled(true); return true;
    }
    void keyBackClicked() override { removeFromParentAndCleanup(true); }
    void onCancel(CCObject*) { removeFromParentAndCleanup(true); }
    void onConfirm(CCObject*){
        rateLevelInDb(m_levelID,m_stars,kDiffs[m_diffIdx],kTypes[m_typeIdx],
            currentPlayerName(),[this](bool ok,std::string const& msg){
                toast(msg,ok?NotificationIcon::Success:NotificationIcon::Error);
                removeFromParentAndCleanup(true);
            });
    }
    void onMinus(CCObject*){
        if(m_stars>1){--m_stars; if(m_starsLbl) m_starsLbl->setString(std::to_string(m_stars).c_str());}
    }
    void onPlus(CCObject*){
        if(m_stars<20){++m_stars; if(m_starsLbl) m_starsLbl->setString(std::to_string(m_stars).c_str());}
    }
    void onDiff(CCObject*){
        m_diffIdx=(m_diffIdx+1)%11;
        auto* i=makeDifficultyIcon(kDiffs[m_diffIdx],0.85f);
        if(i&&m_diffBtn) m_diffBtn->setNormalImage(i);
    }
    void onType(CCObject*){
        m_typeIdx=(m_typeIdx+1)%5;
        auto* i=makeRateTypeIcon(kTypes[m_typeIdx],0.85f);
        if(i&&m_typeBtn) m_typeBtn->setNormalImage(i);
    }
public:
    static AdminRatePopup* create(int64_t id){
        auto* r=new AdminRatePopup();
        if(r&&r->init(id)){ r->autorelease(); return r; }
        CC_SAFE_DELETE(r); return nullptr;
    }
    static void show(int64_t id){
        auto* s=CCDirector::get()->getRunningScene();
        if(s){ auto* p=create(id); if(p) s->addChild(p,99999); }
    }
};

// ════════════════════════════════════════════════════════════
//  CRMainScene
// ════════════════════════════════════════════════════════════

class CRMainScene : public CCLayer {
    bool init(){
        if(!CCLayer::init()) return false;
        auto win=CCDirector::get()->getWinSize();
        float cx=win.width/2.f, cy=win.height/2.f;

        auto* bg=CCSprite::create("GJ_gradientBG.png");
        if(bg){
            bg->setScaleX(win.width/bg->getContentSize().width);
            bg->setScaleY(win.height/bg->getContentSize().height);
            bg->setPosition({cx,cy});
            bg->setColor(ccc3(0,102,255));
            addChild(bg,-1);
        }
        for(int fx=0;fx<2;++fx)
        for(int fy=0;fy<2;++fy){
            auto* s=CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
            if(!s) continue;
            s->setFlipX(fx); s->setFlipY(fy);
            s->setPosition({fx?win.width:0.f,fy?win.height:0.f});
            addChild(s,1);
        }
        auto* tl=CCLabelBMFont::create("Custom Rates","goldFont.fnt");
        tl->setScale(0.9f);
        tl->setPosition({cx,win.height-30.f});
        addChild(tl,2);

        auto* menu=CCMenu::create();
        menu->setPosition({cx,cy});
        addChild(menu,2);

        struct B{ const char* lbl; int tag; float x; float y; };
        std::vector<B> btns={
            {"Sent",   0,-100.f, 55.f},
            {"Recent", 1, 100.f, 55.f},
            {"Random", 2,-100.f,-20.f},
            {"Top",    3, 100.f,-20.f},
        };
        for(auto const& b:btns){
            auto* s=ButtonSprite::create(
                b.lbl,"goldFont.fnt","GJ_button_01.png",0.8f);
            auto* btn=CCMenuItemSpriteExtra::create(
                s,this,menu_selector(CRMainScene::onBtn));
            btn->setTag(b.tag);
            btn->setPosition({b.x,b.y});
            menu->addChild(btn);
        }
        if(isAdmin()){
            auto* db=CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Delete","goldFont.fnt","GJ_button_06.png",0.7f),
                this,menu_selector(CRMainScene::onDelete));
            db->setPosition({0.f,-90.f});
            menu->addChild(db);
        }

        auto* bm=CCMenu::create();
        bm->setPosition({0.f,0.f}); addChild(bm,2);
        auto* bs=CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        auto* bb=CCMenuItemSpriteExtra::create(
            bs,this,menu_selector(CRMainScene::onBack));
        bb->setPosition({28.f,win.height-28.f});
        bm->addChild(bb);

        setKeypadEnabled(true);
        return true;
    }
    void onBtn(CCObject* sender){
        auto* node=typeinfo_cast<CCNode*>(sender);
        if(!node) return;
        switch(node->getTag()){
            case 0: CCDirector::get()->pushScene(CCTransitionFade::create(0.3f,CRListScene::scene(CRListScene::Mode::Sent)));   break;
            case 1: CCDirector::get()->pushScene(CCTransitionFade::create(0.3f,CRListScene::scene(CRListScene::Mode::Recent))); break;
            case 2: CCDirector::get()->pushScene(CCTransitionFade::create(0.3f,CRListScene::scene(CRListScene::Mode::Random))); break;
            case 3: CCDirector::get()->pushScene(CCTransitionFade::create(0.3f,CRTopScene::scene()));                           break;
        }
    }
    void onDelete(CCObject*){ DeleteLevelPopup::show(); }
    void onBack(CCObject*)  { CCDirector::get()->popScene(); }
    void keyBackClicked() override { CCDirector::get()->popScene(); }
public:
    static CCScene* scene(){
        auto* sc=CCScene::create();
        auto* ly=new CRMainScene();
        if(ly&&ly->init()){ ly->autorelease(); sc->addChild(ly); }
        return sc;
    }
    static void show(){
        CCDirector::get()->pushScene(
            CCTransitionFade::create(0.3f,scene()));
    }
};

} // namespace cr

// ════════════════════════════════════════════════════════════
//  ХУКИ
// ════════════════════════════════════════════════════════════

class $modify(CRCreatorLayer, CreatorLayer) {
    bool init() override {
        if(!CreatorLayer::init()) return false;
        cr::bootstrap();
        if(auto* menu=this->getChildByID("creator-buttons-menu")){
            if(auto* item=typeinfo_cast<CCMenuItemSpriteExtra*>(
                    menu->getChildByID("featured-button")))
                item->setTarget(this,menu_selector(CRCreatorLayer::onCustomRates));
        }
        return true;
    }
    void onCustomRates(CCObject*){ cr::CRMainScene::show(); }
};

// ════════════════════════════════════════════════════════════
//  LevelInfoLayer — сложность + синие звёзды из нашей БД
// ════════════════════════════════════════════════════════════

class $modify(CRLevelInfoLayer, LevelInfoLayer) {
    struct Fields { cr::CancelToken cancel; };

    bool init(GJGameLevel* level, bool challenge) {
        if(!LevelInfoLayer::init(level,challenge)) return false;
        if(!m_level) return true;

        auto id=cr::getLevelID(m_level);
        if(id<=0) return true;

        m_fields->cancel=cr::makeCancelToken();
        auto cancel=m_fields->cancel;

        cr::fetchRatedMeta(id,
            [this,cancel](std::optional<cr::LevelEntry> info){
                if(cancel->load()||!m_level) return;
                if(!info) return;

                // Применяем данные к уровню
                cr::applyRateToLevel(m_level,*info);

                // Обновляем DifficultySprite
                if(auto* diffSpr=typeinfo_cast<DifficultySprite*>(
                        this->getChildByID("difficulty-sprite"))){
                    diffSpr->updateDifficultyFrame(
                        m_level->m_difficulty,
                        (GJDifficultyName)0,
                        (GJFeatureState)m_level->m_featured);
                }

                // Обновляем звёзды в UI
                if(auto* starsLbl=typeinfo_cast<CCLabelBMFont*>(
                        this->getChildByID("stars-label"))){
                    starsLbl->setString(
                        std::to_string(info->blueStars).c_str());
                }

                // Добавляем иконку синей звезды и число под сложностью
                // (если ещё не добавлено)
                if(this->getChildByID("cr-star-icon")) return;

                // Находим позицию difficulty sprite
                CCNode* diffNode=this->getChildByID("difficulty-sprite");
                if(!diffNode) return;
                CCPoint diffPos=diffNode->getPosition();

                auto* starIcon=cr::makeBlueStarIcon(0.55f);
                if(starIcon){
                    starIcon->setID("cr-star-icon");
                    // Чуть ниже иконки сложности
                    starIcon->setPosition({
                        diffPos.x - 10.f,
                        diffPos.y - diffNode->getContentSize().height*0.5f - 18.f
                    });
                    this->addChild(starIcon,10);
                }

                auto* starLbl=CCLabelBMFont::create(
                    std::to_string(info->blueStars).c_str(),"bigFont.fnt");
                starLbl->setScale(0.45f);
                starLbl->setColor(ccc3(100,200,255));
                starLbl->setID("cr-star-count");
                starLbl->setAnchorPoint({0.f,0.5f});
                starLbl->setPosition({
                    diffPos.x + 4.f,
                    diffPos.y - diffNode->getContentSize().height*0.5f - 18.f
                });
                this->addChild(starLbl,10);

                // Модератор под названием
                if(!this->getChildByID("cr-mod-label")){
                    auto win=CCDirector::get()->getWinSize();
                    auto* modLbl=CCLabelBMFont::create(
                        ("Rated by "+info->moderator).c_str(),"goldFont.fnt");
                    modLbl->setScale(0.32f);
                    modLbl->setColor(ccc3(80,210,255));
                    modLbl->setID("cr-mod-label");
                    modLbl->setPosition({win.width/2.f, win.height-14.f});
                    this->addChild(modLbl,10);
                }
            },
            nullptr, cancel
        );

        // Кнопка отправки/рейта
        auto* menu=this->getChildByID("left-side-menu");
        if(!menu){
            menu=CCMenu::create();
            menu->setID("cr-side-menu");
            auto win=CCDirector::get()->getWinSize();
            menu->setPosition({24.f,win.height/2.f});
            addChild(menu,10);
        }
        if(!menu->getChildByID("cr-rate-btn")){
            auto* spr=CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png");
            if(!spr) spr=CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
            if(spr) spr->setScale(0.85f);
            auto* btn=CCMenuItemSpriteExtra::create(
                spr,this,menu_selector(CRLevelInfoLayer::onCRButton));
            btn->setID("cr-rate-btn");
            menu->addChild(btn);
            menu->updateLayout();
        }
        return true;
    }

    void onCRButton(CCObject*){
        if(!m_level) return;
        auto id=cr::getLevelID(m_level);
        if(id<=0) return;
        if(cr::isAdmin()){
            cr::AdminRatePopup::show(id);
        } else {
            cr::sendLevelToDb(id,cr::currentPlayerName(),
                [](bool ok,std::string const& msg){
                    cr::toast(msg,ok?NotificationIcon::Success:NotificationIcon::Error);
                });
        }
    }

    void onExit() override {
        if(m_fields->cancel) m_fields->cancel->store(true);
        LevelInfoLayer::onExit();
    }
};

// ════════════════════════════════════════════════════════════
//  InfoLayer — метка "Rated by"
// ════════════════════════════════════════════════════════════

class $modify(CRInfoLayer, InfoLayer) {
    struct Fields { cr::CancelToken cancel; };

    bool init(GJGameLevel* level,GJUserScore* score,GJLevelList* list){
        if(!InfoLayer::init(level,score,list)) return false;
        if(!m_level) return true;
        auto id=cr::getLevelID(m_level);
        if(id<=0) return true;

        m_fields->cancel=cr::makeCancelToken();
        auto cancel=m_fields->cancel;
        auto win=CCDirector::get()->getWinSize();

        auto* label=CCLabelBMFont::create("","goldFont.fnt");
        label->setScale(0.33f);
        label->setColor(ccc3(100,200,255));
        label->setID("cr-rated-by-label");
        label->setPosition({win.width/2.f,win.height-14.f});
        label->setVisible(false);
        addChild(label,100);

        cr::fetchRatedMeta(id,
            [label,cancel](std::optional<cr::LevelEntry> info){
                if(cancel->load()||!label->getParent()) return;
                if(!info||info->moderator.empty()){
                    label->setVisible(false); return;
                }
                label->setString(
                    ("Rated by "+info->moderator+
                     " ("+std::to_string(info->blueStars)+" stars)").c_str());
                label->setVisible(true);
            },
            [label,cancel](std::string const&){
                if(cancel->load()||!label->getParent()) return;
                label->setVisible(false);
            },cancel
        );
        return true;
    }
    void onExit() override {
        if(m_fields->cancel) m_fields->cancel->store(true);
        InfoLayer::onExit();
    }
};

// ════════════════════════════════════════════════════════════
//  PlayLayer — выдача звёзд при прохождении
// ════════════════════════════════════════════════════════════

class $modify(CRPlayLayer, PlayLayer) {
    void levelComplete(){
        PlayLayer::levelComplete();
        if(!m_level||m_isPracticeMode) return;
        auto id  =cr::getLevelID(m_level);
        if(id<=0) return;
        auto user=cr::currentPlayerName();
        if(user.empty()) return;

        cr::fetchRatedMeta(id,
            [id,user](std::optional<cr::LevelEntry> info){
                if(!info||info->blueStars<=0) return;
                cr::awardStarsForLevel(id,info->blueStars,user);
                // Уведомление
                cr::toast(
                    "+"+std::to_string(info->blueStars)+" blue stars!",
                    NotificationIcon::Success);
            }
        );
    }
};

// ════════════════════════════════════════════════════════════
//  ProfilePage — синие звёзды игрока
// ════════════════════════════════════════════════════════════

class $modify(CRProfilePage, ProfilePage) {
    struct Fields { cr::CancelToken cancel; };

    void loadPageFromUserInfo(GJUserScore* score){
        ProfilePage::loadPageFromUserInfo(score);
        if(!score) return;
        if(getChildByID("cr-blue-stars-label")) return;

        m_fields->cancel=cr::makeCancelToken();
        auto cancel=m_fields->cancel;
        auto win=CCDirector::get()->getWinSize();

        // Иконка синей звезды
        auto* star=cr::makeBlueStarIcon(0.5f);
        if(star){
            star->setID("cr-blue-star-icon");
            star->setPosition({win.width/2.f+60.f,win.height/2.f-52.f});
            addChild(star,10);
        }

        // Лейбл
        auto* label=CCLabelBMFont::create("...","goldFont.fnt");
        label->setScale(0.38f);
        label->setColor(ccc3(100,200,255));
        label->setID("cr-blue-stars-label");
        label->setAnchorPoint({0.f,0.5f});
        label->setPosition({win.width/2.f+74.f,win.height/2.f-52.f});
        addChild(label,10);

        std::string username=score->m_userName;
        cr::fetchPlayerStars(username,[label,cancel](int stars){
            if(cancel->load()||!label->getParent()) return;
            label->setString(std::to_string(stars).c_str());
        },cancel);
    }
    void onExit() override {
        if(m_fields->cancel) m_fields->cancel->store(true);
        ProfilePage::onExit();
    }
};
