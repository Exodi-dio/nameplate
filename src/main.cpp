#include <Geode/Geode.hpp>
#include "Render.hpp"
#include "Popup.hpp"

using namespace geode::prelude;

class $modify(ProfilePage) {

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        if (!score || !score->isCurrentUser()) return;

        // Remove any old plate so reloading the page doesn't stack them.
        auto old = this->getChildByID("nameplate"_spr);
        if (old) old->removeFromParent();

        auto spr = nameplate::render(score->m_userName, nameplate::loadStyle());
        spr->setID("nameplate"_spr);
        spr->setPosition({0.f, -34.f});
        spr->setZOrder(10);

        if (m_usernameLabel) {
            auto pos = m_usernameLabel->getPosition();
            spr->setPosition({pos.x, pos.y - 26.f});
        }

        this->addChild(spr);
    }

    bool init(int accountID, bool ownProfile) {
        if (!ProfilePage::init(accountID, ownProfile)) return false;

        // Open the font/colour picker. Placed by the username label.
        auto btn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_editIcon_001.png"),
            this, menu_selector(ProfilePage::onNameplate)
        );
        btn->setID("nameplate-open"_spr);
        btn->setPosition({246.f, 126.f});
        m_buttonMenu->addChild(btn);

        return true;
    }

    void onNameplate(CCObject*) {
        nameplate::Popup::create()->show();
    }
};
