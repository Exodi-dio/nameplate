#include "Popup.hpp"

#include <algorithm>
#include <Geode/utils/CCColor.hpp>

using namespace geode::prelude;

namespace nameplate {

namespace {

CCSprite* swatch(ccColor3B colour) {
    // 1x1 texture keeps this independent of any spritesheet.
    unsigned char px[4] = {colour.r, colour.g, colour.b, 255};
    auto tex = new CCTexture2D();
    tex->initWithData(px, kCCTexture2DPixelFormat_RGBA8888, 1, 1, CCSize(1, 1));
    auto spr = CCSprite::createWithTexture(tex);
    tex->release();
    spr->setScale(26.f);
    return spr;
}

char const* slotLabel(int slot) {
    switch (slot) {
        case 0: return "Text top";
        case 1: return "Text bottom";
        case 2: return "Line top";
        default: return "Line bottom";
    }
}

} // namespace

Popup* Popup::create() {
    auto ret = new Popup();
    if (ret && ret->init(420.f, 300.f, "GJ_square01.png")) {
        ret->autorelease();
        ret->buildUI();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void Popup::buildUI() {
    this->setTitle("Nameplate");

    auto menu = this->m_buttonMenu;

    auto left = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_01.png"), this,
        menu_selector(Popup::onFontLeft)
    );
    left->setID("font-left"_spr);
    left->setPosition({-140.f, 92.f});
    menu->addChild(left);

    auto right = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_01.png"), this,
        menu_selector(Popup::onFontRight)
    );
    right->setID("font-right"_spr);
    right->setScaleX(-1.f);
    right->setPosition({140.f, 92.f});
    menu->addChild(right);

    m_fontLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_fontLabel->setScale(0.7f);
    m_fontLabel->setPosition({0.f, 92.f});
    m_buttonMenu->getParent()->addChild(m_fontLabel);

    for (int i = 0; i < 4; ++i) {
        auto item = CCMenuItemSpriteExtra::create(
            swatch(ccc3(255, 255, 255)), this,
            menu_selector(Popup::onColour)
        );
        item->setTag(i);
        item->setPosition({-150.f + i * 100.f, -66.f});
        menu->addChild(item);
        m_swatches[i] = static_cast<CCSprite*>(item->getNormalImage());

        auto label = CCLabelBMFont::create(slotLabel(i), "chatFont.fnt");
        label->setScale(0.55f);
        label->setPosition({-150.f + i * 100.f, -90.f});
        m_buttonMenu->getParent()->addChild(label);
    }

    auto save = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_01.png", 0.8f),
        this, menu_selector(Popup::onSave)
    );
    save->setID("save"_spr);
    save->setPosition({0.f, -114.f});
    menu->addChild(save);

    refresh();
}

void Popup::refresh() {
    if (m_fontLabel) {
        m_fontLabel->setString(fontName(m_style.font).c_str());
    }

    ccColor3B const colours[4] = {
        m_style.fillTop, m_style.fillBottom, m_style.lineTop, m_style.lineBottom
    };
    for (int i = 0; i < 4; ++i) {
        if (!m_swatches[i]) continue;
        m_swatches[i]->setColor(colours[i]);
        m_swatches[i]->setOpacity(m_style.outline || i < 2 ? 255 : 90);
    }

    rebuildPreview();
}

void Popup::rebuildPreview() {
    if (m_preview) {
        m_preview->removeFromParent();
        m_preview = nullptr;
    }

    auto name = GJAccountManager::get()->m_username;
    if (name.empty()) name = "Player";

    Style preview = m_style;
    preview.scale = 0.55f;
    m_preview = render(name, preview);
    m_preview->setPosition({0.f, 10.f});
    m_preview->setID("preview"_spr);
    m_buttonMenu->getParent()->addChild(m_preview, 10);
}

void Popup::onFontLeft(CCObject*) {
    auto const& ids = fontIds();
    auto it = std::find(ids.begin(), ids.end(), m_style.font);
    size_t index = (it == ids.end()) ? 0 : (size_t)(it - ids.begin());
    index = (index + ids.size() - 1) % ids.size();
    m_style.font = ids[index];
    refresh();
}

void Popup::onFontRight(CCObject*) {
    auto const& ids = fontIds();
    auto it = std::find(ids.begin(), ids.end(), m_style.font);
    size_t index = (it == ids.end()) ? 0 : (size_t)(it - ids.begin());
    index = (index + 1) % ids.size();
    m_style.font = ids[index];
    refresh();
}

void Popup::onColour(CCObject* sender) {
    int slot = sender ? sender->getTag() : 0;

    ccColor3B current = slot == 0 ? m_style.fillTop
                     : slot == 1 ? m_style.fillBottom
                     : slot == 2 ? m_style.lineTop
                                 : m_style.lineBottom;

    auto picker = ColorPickPopup::create(current);
    picker->setCallback([this, slot](ccColor4B const& c) {
        auto colour = ccc3(c.r, c.g, c.b);
        switch (slot) {
            case 0: m_style.fillTop = colour; break;
            case 1: m_style.fillBottom = colour; break;
            case 2: m_style.lineTop = colour; break;
            default: m_style.lineBottom = colour; break;
        }
        refresh();
    });
    picker->show();
}

void Popup::onToggleOutline(CCObject*) {
    m_style.outline = !m_style.outline;
    refresh();
}

void Popup::onSave(CCObject*) {
    saveStyle(m_style);
    Notification::create("Nameplate saved", NotificationIcon::Success)->show();
    this->onClose(nullptr);
}

} // namespace nameplate
