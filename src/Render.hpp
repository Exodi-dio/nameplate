#pragma once

#include <Geode/Geode.hpp>
#include <string>
#include <vector>

namespace nameplate {

struct Style {
    std::string font = "pressstart2p";
    cocos2d::ccColor3B fillTop{255, 214, 64};
    cocos2d::ccColor3B fillBottom{255, 122, 24};
    cocos2d::ccColor3B lineTop{56, 28, 0};
    cocos2d::ccColor3B lineBottom{18, 8, 0};
    bool outline = true;
    float scale = 1.f;
};

Style loadStyle();
void saveStyle(Style const& style);

std::vector<std::string> const& fontIds();
std::string const& fontName(std::string const& id);

// Draws text into a texture and returns it as a sprite. Never returns null.
cocos2d::CCSprite* render(std::string const& text, Style const& style);

} // namespace nameplate
