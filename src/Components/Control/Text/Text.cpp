#include "Text.h"
#include <Singletons/Ref/Ref.h>
#include <algorithm>
#include <iostream>
#include "GameObject/GameObject.h"
#include "Shader/Shader.h"

using namespace UI;

GLuint Text::m_FontTexture = 0;
std::map<char, Text::Character> Text::m_Characters;
unsigned int Text::m_AtlasWidth = 0;
unsigned int Text::m_AtlasHeight = 0;
bool Text::m_FontInitialized = false;

Text::Text() {
    name = "Text";
    icon = ICON_FA_FONT;
    
}

Text::~Text() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
    }
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
    }
}

void Text::Render() {
    if (!gameObject->m_enabled || !enabled || m_Text.empty())
        return;

    auto pShader = GetShader();
    if (!pShader || !m_FontInitialized)
        return;

    pShader->Use();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FontTexture);

    pShader->SetVec3("textColor", m_Color * Emission);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(Ref::ScreenResolution.x), 0.0f,
                                      static_cast<float>(Ref::ScreenResolution.y));
    pShader->SetMat4("projection", projection);

    glm::vec2 startPos = GetAnchoredPosition();
    float x = startPos.x;
    float y = startPos.y;
    float scale = m_FontSize / 48.0f;

    int visibleChars = 0;
    for (char c: m_Text) {
        auto it = m_Characters.find(c);
        if (it != m_Characters.end() && it->second.bw > 0 && it->second.bh > 0) {
            visibleChars++;
        }
    }

    if (visibleChars == 0) {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        return;
    }

    std::vector<float> vertices;
    vertices.reserve(visibleChars * 6 * 4); // 6 vertices * 4 components

    // add every char to vertices buff
    for (char c: m_Text) {
        auto it = m_Characters.find(c);
        if (it == m_Characters.end()) {
            x += 20.0f * scale;
            continue;
        }

        const Character &ch = it->second;

        // skip space
        if (ch.bw <= 0 || ch.bh <= 0) {
            x += ch.ax * scale;
            continue;
        }

        float xpos = x + ch.bl * scale;
        float ypos = y - (ch.bh - ch.bt) * scale;
        float w = ch.bw * scale;
        float h = ch.bh * scale;

        float tx = ch.tx;
        float ty = ch.ty;
        float tw = ch.bw / (float) m_AtlasWidth;
        float th = ch.bh / (float) m_AtlasHeight;

        vertices.insert(vertices.end(), {
                                                xpos, ypos, tx, ty + th, // bottom-left
                                                xpos + w, ypos, tx + tw, ty + th, // bottom-right
                                                xpos + w, ypos + h, tx + tw, ty, // top-right
                                        });

        vertices.insert(vertices.end(), {
                                                xpos, ypos, tx, ty + th, // bottom-left
                                                xpos + w, ypos + h, tx + tw, ty, // top-right
                                                xpos, ypos + h, tx, ty // top-left
                                        });

        x += ch.ax * scale;
    }

    if (!vertices.empty()) {
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

        // one draw call for entire text
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 4);

        glBindVertexArray(0);
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Text::Update(float deltaTime) {
    if (!m_FontInitialized)
    {

        InitializeFont();
        
    }

    if (!m_Setup)
    {
        setupMesh();
        m_Setup = true;
    }
}

void Text::SetText(const std::string &text) { m_Text = text; }

const std::string &Text::GetText() const { return m_Text; }

void Text::SetFontSize(float size) { m_FontSize = std::max(1.0f, size); }

float Text::GetFontSize() const { return m_FontSize; }

void Text::SetColor(const glm::vec3 &color) { m_Color = color; }

const glm::vec3 &Text::GetColor() const { return m_Color; }

void Text::setupMesh() {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Text::InitializeFont(const std::string &fontPath) {
    if (m_FontInitialized) {
        return;
    }

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font " << fontPath << std::endl;
        FT_Done_FreeType(ft);
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    // calculate atlas size
    unsigned int rowWidth = 0;
    unsigned int rowHeight = 0;
    m_AtlasWidth = 0;
    m_AtlasHeight = 0;

    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            continue;
        }

        if (rowWidth + face->glyph->bitmap.width + 1 >= m_MaxAtlasWidth) {
            m_AtlasWidth = std::max(m_AtlasWidth, rowWidth);
            m_AtlasHeight += rowHeight;
            rowWidth = 0;
            rowHeight = 0;
        }
        rowWidth += face->glyph->bitmap.width + 1;
        rowHeight = std::max(rowHeight, face->glyph->bitmap.rows);
    }

    m_AtlasWidth = std::max(m_AtlasWidth, rowWidth);
    m_AtlasHeight += rowHeight;

    glGenTextures(1, &m_FontTexture);
    glBindTexture(GL_TEXTURE_2D, m_FontTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_AtlasWidth, m_AtlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int xOffset = 0;
    int yOffset = 0;
    rowHeight = 0;

    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            continue;
        }

        if (xOffset + face->glyph->bitmap.width + 1 >= m_MaxAtlasWidth) {
            yOffset += rowHeight;
            rowHeight = 0;
            xOffset = 0;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED,
                        GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        Character character;
        character.ax = face->glyph->advance.x >> 6;
        character.ay = face->glyph->advance.y >> 6;
        character.bw = face->glyph->bitmap.width;
        character.bh = face->glyph->bitmap.rows;
        character.bl = face->glyph->bitmap_left;
        character.bt = face->glyph->bitmap_top;
        character.tx = xOffset / (float) m_AtlasWidth;
        character.ty = yOffset / (float) m_AtlasHeight;

        m_Characters[i] = character;

        rowHeight = std::max(rowHeight, face->glyph->bitmap.rows);
        xOffset += face->glyph->bitmap.width + 1;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    m_FontInitialized = true;
}

void Text::CleanupFont() {
    if (m_FontInitialized) {
        glDeleteTextures(1, &m_FontTexture);
        m_Characters.clear();
        m_FontInitialized = false;
    }
}
