#include "Render.hpp"

#include <Geode/utils/file.hpp>
#include <sstream>
#include <unordered_map>

using namespace geode::prelude;
using namespace cocos2d;


namespace nameplate {


namespace {

// glyph
struct Glyph {
	int x = 0, y = 0, w = 0, h = 0, adv = 0;
};

// atlas data
struct Font {
	int width = 0, height = 0, lineHeight = 0, baseline = 0;
	std::vector<unsigned char> pixels;
	std::unordered_map<int, Glyph> glyphs;
};



// font list
std::vector<std::pair<std::string, std::string>> const& list() {
	static std::vector<std::pair<std::string, std::string>> const fonts = {
		{"pressstart2p", "Press Start"},
		{"silkscreen", "Silkscreen"},
		{"vt323", "VT323"},
		{"audiowide", "Audiowide"},
		{"russoone", "Russo One"},
		{"bungee", "Bungee"},
		{"rubikmonoone", "Rubik Mono"},
		{"creepster", "Creepster"},
	};
	return fonts;
}

std::unordered_map<std::string, Font>& cache() {
	static std::unordered_map<std::string, Font> fonts;
	return fonts;
}



// load atlas from resources
Font* load(std::string const& id) {
	auto& fonts = cache();
	if (auto it = fonts.find(id); it != fonts.end()) {
		return &it->second;
	}

	auto dir = Mod::get()->getResourcesDir() / "fonts" / id;

	auto meta = geode::utils::file::readString(dir / "atlas.txt");
	if (!meta) {
		log::warn("nameplate: no atlas for {}", id);
		return nullptr;
	}

	Font font;
	std::istringstream in(meta.unwrap());
	in >> font.width >> font.height >> font.lineHeight >> font.baseline;
	if (!in || font.width <= 0 || font.height <= 0) {
		log::warn("nameplate: bad atlas header for {}", id);
		return nullptr;
	}

	int cp = 0;
	Glyph g;
	while (in >> cp >> g.x >> g.y >> g.w >> g.h >> g.adv) {
		font.glyphs[cp] = g;
	}

	auto raw = geode::utils::file::readBinary(dir / "atlas.raw");
	if (!raw) {
		log::warn("nameplate: no atlas pixels for {}", id);
		return nullptr;
	}
	font.pixels = raw.unwrap();

	if (font.pixels.size() < (size_t)font.width * font.height) {
		log::warn("nameplate: atlas too short for {}", id);
		return nullptr;
	}

	return &(fonts[id] = std::move(font));
}



// lerp color
ccColor3B mix(ccColor3B a, ccColor3B b, float t) {
	if (t < 0.f) t = 0.f;
	if (t > 1.f) t = 1.f;
	return ccc3(
		(GLubyte)(a.r + (b.r - a.r) * t),
		(GLubyte)(a.g + (b.g - a.g) * t),
		(GLubyte)(a.b + (b.b - a.b) * t)
	);
}

} // namespace


std::vector<std::string> const& fontIds() {
	static std::vector<std::string> ids = [] {
		std::vector<std::string> out;
		for (auto const& f : list()) out.push_back(f.first);
		return out;
	}();
	return ids;
}

std::string const& fontName(std::string const& id) {
	for (auto const& f : list()) {
		if (f.first == id) return f.second;
	}
	return list().front().second;
}

Style loadStyle() {
	auto mod = Mod::get();
	Style s;
	s.font = mod->getSavedValue<std::string>("font", s.font);
	s.outline = mod->getSavedValue<bool>("outline", s.outline);
	s.scale = mod->getSavedValue<float>("scale", s.scale);

	auto colour = [&](char const* key, ccColor3B fallback) {
		int packed = mod->getSavedValue<int>(key, 0);
		if (!packed) return fallback;
		return ccc3((packed >> 16) & 0xFF, (packed >> 8) & 0xFF, packed & 0xFF);
	};

	s.fillTop = colour("fill-top", s.fillTop);
	s.fillBottom = colour("fill-bottom", s.fillBottom);
	s.lineTop = colour("line-top", s.lineTop);
	s.lineBottom = colour("line-bottom", s.lineBottom);
	return s;
}

void saveStyle(Style const& s) {
	auto mod = Mod::get();
	mod->setSavedValue("font", s.font);
	mod->setSavedValue("outline", s.outline);
	mod->setSavedValue("scale", s.scale);

	auto pack = [](ccColor3B c) {
		return (c.r << 16) | (c.g << 8) | c.b;
	};
	mod->setSavedValue("fill-top", pack(s.fillTop));
	mod->setSavedValue("fill-bottom", pack(s.fillBottom));
	mod->setSavedValue("line-top", pack(s.lineTop));
	mod->setSavedValue("line-bottom", pack(s.lineBottom));
}



// render text
CCSprite* render(std::string const& text, Style const& style) {
	auto font = load(style.font);
	if (!font) {
		return CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
	}

	int const radius = style.outline ? 3 : 0;
	int const pad = radius + 1;

	int width = pad * 2;
	for (unsigned char ch : text) {
		auto it = font->glyphs.find(ch);
		width += (it != font->glyphs.end()) ? it->second.adv : 12;
	}
	int height = font->lineHeight + pad * 2;
	if (width < 4) width = 4;
	if (height < 4) height = 4;

	std::vector<unsigned char> mask((size_t)width * height, 0);

	int pen = pad;
	for (unsigned char ch : text) {
		auto it = font->glyphs.find(ch);
		int adv = 12;
		if (it != font->glyphs.end()) {
			auto const& g = it->second;
			adv = g.adv;
			int top = pad + font->baseline - g.h;
			for (int gy = 0; gy < g.h; ++gy) {
				int dstY = top + gy;
				if (dstY < 0 || dstY >= height) continue;
				for (int gx = 0; gx < g.w; ++gx) {
					int dstX = pen + gx;
					if (dstX < 0 || dstX >= width) continue;
					size_t src = (size_t)(g.y + gy) * font->width + (g.x + gx);
					if (src >= font->pixels.size()) continue;
					unsigned char a = font->pixels[src];
					if (a > mask[(size_t)dstY * width + dstX]) {
						mask[(size_t)dstY * width + dstX] = a;
					}
				}
			}
		}
		pen += adv;
	}

	std::vector<unsigned char> outline;
	if (radius > 0) {
		outline.assign(mask.size(), 0);
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				if (!mask[(size_t)y * width + x]) continue;
				for (int dy = -radius; dy <= radius; ++dy) {
					int ny = y + dy;
					if (ny < 0 || ny >= height) continue;
					for (int dx = -radius; dx <= radius; ++dx) {
						int nx = x + dx;
						if (nx < 0 || nx >= width) continue;
						outline[(size_t)ny * width + nx] = 255;
					}
				}
			}
		}
	}

	std::vector<unsigned char> rgba((size_t)width * height * 4, 0);
	for (int y = 0; y < height; ++y) {
		float t = height > 1 ? (float)y / (height - 1) : 0.f;
		auto fill = mix(style.fillTop, style.fillBottom, t);
		auto line = mix(style.lineTop, style.lineBottom, t);
		for (int x = 0; x < width; ++x) {
			size_t i = (size_t)y * width + x;
			unsigned char a = mask[i];
			ccColor3B c = fill;
			if (!a && !outline.empty() && outline[i]) {
				a = outline[i];
				c = line;
			}
			if (!a) continue;
			size_t o = i * 4;
			rgba[o + 0] = c.r;
			rgba[o + 1] = c.g;
			rgba[o + 2] = c.b;
			rgba[o + 3] = a;
		}
	}

	auto tex = new CCTexture2D();
	bool ok = tex->initWithData(
		rgba.data(), kCCTexture2DPixelFormat_RGBA8888, width, height,
		CCSize((float)width, (float)height)
	);
	if (!ok) {
		tex->release();
		return CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
	}

	auto sprite = CCSprite::createWithTexture(tex);
	tex->release();
	if (!sprite) {
		return CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
	}

	sprite->setFlipY(true);
	sprite->setScale(style.scale);
	return sprite;
}


} // namespace nameplate
