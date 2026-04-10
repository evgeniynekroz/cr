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

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

using namespace geode::prelude;

namespace cr {
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
        Notification::create(message.c_str(), icon)->show();
    }

    static void bootstrap() {
        if (g_bootstrapped) return;
        g_bootstrapped = true;
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

    // Offline / build-safe stubs.
    static void fetchLevels(
        Tab,
        std::function<void(std::vector<LevelEntry>)> onOk,
        std::function<void(std::string const&)> = nullptr
    ) {
        if (onOk) onOk({});
    }

    static void fetchRatedMeta(
        int64_t,
        std::function<void(std::optional<LevelEntry>)> onOk,
        std::function<void(std::string const&)> = nullptr
    ) {
        if (onOk) onOk(std::nullopt);
    }

    static void sendLevelToDb(
        int64_t,
        std::string const&,
        std::function<void(bool, std::string const&)> onDone
    ) {
        auto msg = "HTTP disabled for now (offline build-safe mode)";
        toast(msg, NotificationIcon::Error);
        if (onDone) onDone(false, msg);
    }

    static void deleteSentFromDb(
        int64_t,
        std::function<void(bool, std::string const&)> onDone
    ) {
        auto msg = "HTTP disabled for now (offline build-safe mode)";
        toast(msg, NotificationIcon::Error);
        if (onDone) onDone(false, msg);
    }

    static void rateLevelInDb(
        int64_t,
        int,
        std::string const&,
        std::function<void(bool, std::string const&)> onDone
    ) {
        auto msg = "HTTP disabled for now (offline build-safe mode)";
        toast(msg, NotificationIcon::Error);
        if (onDone) onDone(false, msg);
    }

    class CustomRatesLayer : public CCLayerColor {
    private:
        Tab m_tab = Tab::Sent;
        int m_page = 0;
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

            m_statusLabel = CCLabelBMFont::create("Offline mode", "bigFont.fnt");
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

            m_entries.clear();
            if (m_statusLabel) {
                m_statusLabel->setString("Offline mode");
            }

            fetchLevels(
                tab,
                [this](std::vector<LevelEntry> entries) {
                    m_entries = std::move(entries);
                    if (m_statusLabel) {
                        m_statusLabel->setString((std::string("Loaded: ") + std::to_string(m_entries.size())).c_str());
                    }
                    renderPage();
                },
                [this](std::string const& err) {
                    if (m_statusLabel) {
                        m_statusLabel->setString(err.c_str());
                    }
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

            deleteSentFromDb(levelID, [this](bool ok, std::string const& msg) {
                toast(msg, ok ? NotificationIcon::Success : NotificationIcon::Error);
                if (ok && m_tab == Tab::Sent) {
                    loadTab(Tab::Sent);
                }
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

        auto label = CCLabelBMFont::create("offline mode", "bigFont.fnt");
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

        label->setVisible(false);
    }
};
