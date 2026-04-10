#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

// ==========================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ И КОНФИГУРАЦИЯ
// ==========================================
bool g_isAdmin = false; // Кэш статуса админа

const std::string CURRENT_VERSION = "v1.0.0";
const std::string UPDATE_URL = "https://raw.githubusercontent.com/evgeniynekroz/custom-rates-mod/main/version.json";
const std::string TURSO_URL = "https://custom-rates-evgen.aws-eu-west-1.turso.io/v2/pipeline";

// Зашифрованный токен
const std::vector<unsigned char> ENC_TOKEN = {
    0xd4, 0xe3, 0xde, 0xe5, 0xf2, 0xc2, 0xae, 0xa6, 0x82, 0xa0, 0xfb, 0xdc, 0xce, 0xc8, 0xc2, 0xd1, 
    0x9c, 0x9c, 0x84, 0xba, 0xf8, 0xf4, 0xc6, 0xb8, 0xf3, 0xc6, 0x84, 0xf9, 0x84, 0xa2, 0xc1, 0xc2, 
    0xc2, 0xce, 0xda, 0xbc, 0xe3, 0xaa, 0xb4, 0x83, 0xd9, 0xd3, 0xfe, 0xe2, 0xf9, 0xe6, 0xa3, 0xac, 
    0xa4, 0x85, 0xf2, 0xd0, 0xe4, 0xd4, 0xc8, 0xd4, 0xa4, 0x80, 0xa7, 0x8c, 0x82, 0xd4, 0xee, 0xd8, 
    0xa1, 0xca, 0x99, 0xac, 0xfc, 0x87, 0xdb, 0xcf, 0xe7, 0xc4, 0xfd, 0xe9, 0xa6, 0x86, 0xa7, 0xa6, 
    0xd8, 0xd7, 0xd0, 0xc8, 0xa5, 0xdf, 0x89, 0x96, 0xf8, 0x93, 0xf6, 0xd3, 0xe0, 0xc0, 0xd4, 0xe6, 
    0xba, 0x82, 0x9e, 0xf9, 0x82, 0xd4, 0xfe, 0xdb, 0xfb, 0xc9, 0x99, 0xa4, 0xb7, 0x90, 0xe5, 0xc3, 
    0xe0, 0xc3, 0xc4, 0xd0, 0xb7, 0x95, 0x9a, 0x93, 0xdc, 0xc3, 0xc0, 0xc3, 0xfb, 0xdc, 0xa0, 0x99, 
    0xa1, 0x80, 0xd8, 0xed, 0xfd, 0xee, 0xfd, 0xe9, 0xa6, 0x86, 0xa7, 0xa6, 0xd8, 0xd5, 0xc3, 0xdb, 
    0xfd, 0xc8, 0xff, 0x85, 0xa4, 0x87, 0xdc, 0xcf, 0xe0, 0xd4, 0xfa, 0xcc, 0xff, 0x82, 0x9e, 0xf9, 
    0x81, 0xd4, 0xc0, 0xcc, 0xa5, 0xc9, 0x9a, 0x89, 0xa7, 0x93, 0xf5, 0xf1, 0xe0, 0xc2, 0xc4, 0xed, 
    0xa7, 0x82, 0x9a, 0x90, 0xc6, 0xc0, 0xc0, 0xdc, 0xa4, 0xcb, 0xa7, 0x85, 0xa5, 0x80, 0xdf, 0xaa, 
    0xba, 0xeb, 0xc1, 0xb1, 0xa4, 0xbb, 0x8b, 0x96, 0xc3, 0xf3, 0xc3, 0xe1, 0xdf, 0xe7, 0x9f, 0x83, 
    0xf4, 0xf9, 0xe9, 0xaf, 0xf0, 0xe5, 0xe1, 0xb4, 0x95, 0xf7, 0x87, 0xb3, 0xfc, 0xf1, 0xb9, 0xf8, 
    0xf5, 0xb3, 0xf4, 0xa4, 0x95, 0x98, 0xfc, 0xc2, 0xd5, 0xbb, 0xc5, 0xc3, 0xfb, 0xf7, 0x84, 0xa7, 
    0xf6, 0xb7, 0xfd, 0xd5, 0xd5, 0xf6, 0xb7, 0x98, 0xb8, 0xfd, 0xc6, 0xdb, 0xc2, 0xda, 0xe6, 0xee, 
    0xac, 0xa9, 0x8a, 0xa4, 0xe8, 0xec, 0xe5, 0xfd, 0xc5, 0xfd, 0x9d, 0x88, 0x83, 0xa6, 0x85, 0xaa, 
    0xc2, 0xf5, 0xc9, 0xd7, 0xfc, 0x8d, 0xaa
};

const std::string CRYPTO_KEY = "Nekroz2026";

std::string getAuthHeader() {
    std::string token;
    for (size_t i = 0; i < ENC_TOKEN.size(); i++) {
        unsigned char decrypted = ~(ENC_TOKEN[i] ^ CRYPTO_KEY[i % CRYPTO_KEY.length()]);
        token += (char)decrypted;
    }
    return "Bearer " + token;
}

// ==========================================
// ИНТЕРФЕЙС CUSTOM FEATURED
// ==========================================
class CustomRatesPopup : public geode::Popup<> {
protected:
    bool setup() override {
        this->setTitle("Custom Featured");

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        if (auto bg = typeinfo_cast<CCScale9Sprite*>(m_mainLayer->getChildByID("background"))) {
            bg->setColor({ 15, 15, 15 });
        }

        auto menu = CCMenu::create();
        menu->setPosition({ winSize.width / 2, winSize.height / 2 + 80 });
        m_mainLayer->addChild(menu);

        auto topSprite = ButtonSprite::create("Blue Star Top", "goldFont.fnt", "GJ_button_02.png", .7f);
        topSprite->setColor({ 0, 160, 255 });
        auto topBtn = CCMenuItemSpriteExtra::create(topSprite, this, menu_selector(CustomRatesPopup::onTop));
        topBtn->setPosition({ -100, 0 });
        menu->addChild(topBtn);

        auto tagsSprite = ButtonSprite::create("Tags (#Layout)", "goldFont.fnt", "GJ_button_01.png", .7f);
        auto tagsBtn = CCMenuItemSpriteExtra::create(tagsSprite, this, menu_selector(CustomRatesPopup::onTags));
        tagsBtn->setPosition({ 100, 0 });
        menu->addChild(tagsBtn);

        auto label = CCLabelBMFont::create("Загрузка уровней...", "bigFont.fnt");
        label->setScale(0.45f);
        m_mainLayer->addChildAtPosition(label, Anchor::Center);

        return true;
    }

    void onTop(CCObject*) { FLAlertLayer::create("Leaderboard", "Топ игроков в разработке", "OK")->show(); }
    void onTags(CCObject*) { FLAlertLayer::create("Tags", "Фильтр по тегам скоро появится", "OK")->show(); }

public:
    static CustomRatesPopup* create() {
        auto ret = new CustomRatesPopup();
        if (ret && ret->initAnchored(420.f, 280.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ==========================================
// ХУКИ
// ==========================================

// 1. Главное меню (Проверка БД и обновлений)
class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        
        // Проверка обновлений
        web::AsyncWebRequest()
            .fetch(UPDATE_URL)
            .text()
            .then([](std::string const& res) {
                if (res.find(CURRENT_VERSION) == std::string::npos && res.find("version") != std::string::npos) {
                    FLAlertLayer::create("Update Available", "Доступно обновление Custom Rates!", "OK")->show();
                }
            });

        // Асинхронный запрос к БД: проверяем, есть ли ник в таблице admins
        std::string myName = GJAccountManager::sharedState()->m_username;
        if (!myName.empty()) {
            std::string query = "{\"requests\":[{\"type\":\"execute\",\"stmt\":{\"sql\":\"SELECT 1 FROM admins WHERE username = '" + myName + "'\"}}]}";
            
            web::AsyncWebRequest()
                .header("Authorization", getAuthHeader())
                .header("Content-Type", "application/json")
                .bodyRaw(query)
                .post(TURSO_URL)
                .text()
                .then([](std::string const& res) {
                    // Turso возвращает пустой массив rows: [], если ничего не найдено.
                    // Если строка найдена, массив будет заполнен.
                    if (res.find("\"rows\":[]") == std::string::npos && 
                        res.find("\"rows\": []") == std::string::npos && 
                        res.find("\"rows\"") != std::string::npos) {
                        g_isAdmin = true; // Успех: игрок есть в БД
                    }
                });
        }
        return true;
    }
};

// 2. Вкладка Creator (Перехват кнопки Featured)
class $modify(MyCreatorLayer, CreatorLayer) {
    void onFeaturedLevels(CCObject* sender) {
        CustomRatesPopup::create()->show();
    }
};

// 3. Страница уровня (Рисуем кнопку, если g_isAdmin == true)
class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;

        // Проверяем глобальный флаг, полученный из БД
        if (g_isAdmin) {
            if (auto sideMenu = this->getChildByID("left-side-menu")) {
                auto sprite = CCSprite::create("blue_star.png"_spr);
                if (!sprite) {
                    sprite = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
                    sprite->setColor({ 0, 150, 255 });
                }
                auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MyLevelInfoLayer::onRate));
                sideMenu->addChild(btn);
                sideMenu->updateLayout();
            }
        }
        return true;
    }

    void onRate(CCObject*) {
        FLAlertLayer::create("Custom Rate", "Отправка в базу...", "OK")->show();
    }
};
