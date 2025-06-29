#include <string>
#include <imgui.h>
#ifndef FONT_H
#define FONT_H

enum FontType
{
    REGULAR,
    BOLD,
    ITALIC,
    THIN,
    LIGHT
};

struct Font
{
    FontType type;

    ImFont *pFont;
    
    Font(FontType type, ImFont *pFont) 
        :  type(type), pFont(pFont) {}
};

struct FontFamily
{
    std::string name;
    std::vector<Font> fonts;

    FontFamily() = default;
    FontFamily(const std::string &name) : name(name) {}

    void AddFont(FontType type, ImFont *pFont) { fonts.emplace_back(type, pFont); }

    ImFont *GetFont(FontType type) {
        for (const auto &font: fonts) {
            if (font.type == type)
                return font.pFont;
        }
        return nullptr;
    }
};

#endif // !FONT_H
