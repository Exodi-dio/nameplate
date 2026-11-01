#pragma once

#include "Render.hpp"
#include <Geode/Geode.hpp>

namespace nameplate {

class Popup : public geode::Popup {
protected:
    Style m_style = loadStyle();
    cocos2d::CCSprite* m_preview = nullptr;
    cocos2d::CCLabelBMFont* m_fontLabel = nullptr;
    cocos2d::CCSprite* m_swatches[4] = {nullptr, nullptr, nullptr, nullptr};

    void buildUI();
    void refresh();
    void rebuildPreview();
    void onColour(cocos2d::CCObject* sender);
    void onFontLeft(cocos2d::CCObject*);
    void onFontRight(cocos2d::CCObject*);
    void onToggleOutline(cocos2d::CCObject*);
    void onSave(cocos2d::CCObject*);

public:
    static Popup* create();
};

} // namespace nameplate
