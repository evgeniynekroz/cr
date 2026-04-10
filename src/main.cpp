#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

// Наследуемся от Popup<>, чтобы работал m_mainLayer и другие методы
class CustomRatesPopup : public geode::Popup<> {
protected:
    bool setup() override {
        // Устанавливаем заголовок
        this->setTitle("Custom Featured");

        auto winSize = CCDirector::get()->getWinSize();
        
        // Пытаемся найти фон, если нужно (как было в твоем логе)
        if (auto bg = typeinfo_cast<CCScale9Sprite*>(m_mainLayer->getChildByID("background"))) {
            // Тут можно что-то сделать с фоном
        }

        auto menu = CCMenu::create();
        m_mainLayer->addChild(menu);

        // Кнопка Top
        auto topSprite = ButtonSprite::create("Top");
        auto topBtn = CCMenuItemSpriteExtra::create(
            topSprite, 
            this, 
            menu_selector(CustomRatesPopup::onTop)
        );
        menu->addChild(topBtn);

        // Кнопка Tags
        auto tagsSprite = ButtonSprite::create("Tags");
        auto tagsBtn = CCMenuItemSpriteExtra::create(
            tagsSprite, 
            this, 
            menu_selector(CustomRatesPopup::onTags)
        );
        menu->addChild(tagsBtn);

        menu->alignItemsVerticallyWithPadding(10.f);

        // Пример использования лейбла с привязкой (Anchor::Center)
        auto label = CCLabelBMFont::create("Select a category", "bigFont.fnt");
        label->setScale(0.6f);
        m_mainLayer->addChildAtPosition(label, Anchor::Center, ccp(0, 50));

        return true;
    }

public:
    static CustomRatesPopup* create() {
        auto ret = new CustomRatesPopup();
        // Размеры окна 420x280
        if (ret && ret->initAnchored(420.f, 280.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void onTop(CCObject*) {
        // Заменили AsyncWebRequest на WebRequest
        web::WebRequest()
            .get("https://api.example.com/top") // Твой URL здесь
            .listen([](web::WebResponse* res) {
                if (res->ok()) {
                    log::info("Success: {}", res->string().unwrapOr("no data"));
                }
            });
    }

    void onTags(CCObject*) {
        web::WebRequest()
            .get("https://api.example.com/tags")
            .listen([](web::WebResponse* res) {
                if (res->ok()) {
                    log::info("Tags loaded");
                }
            });
    }
};

// Хукаем главное меню, чтобы добавить кнопку вызова нашего окна
class $modify(MyMenuLayer, MenuLayer) {
    bool init() override {
        if (!MenuLayer::init()) return false;

        auto myButton = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png"), // Временно, пока не добавишь свою звезду
            this,
            menu_selector(MyMenuLayer::onCustomPopup)
        );

        auto menu = this->getChildByID("bottom-menu");
        if (menu) {
            menu->addChild(myButton);
            menu->updateLayout();
        }

        return true;
    }

    void onCustomPopup(CCObject*) {
        CustomRatesPopup::create()->show();
    }
};
