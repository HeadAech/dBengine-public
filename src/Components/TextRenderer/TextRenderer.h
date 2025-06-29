//
// Created by Hubert Klonowski on 02/04/2025.
//

#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <glm/glm.hpp>
#include <map>

#include "Component/Component.h"
#include "Shader/Shader.h"

struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // size of glyph
    glm::ivec2   Bearing;   // offset from baseline to left/top of glyph
    unsigned int Advance;   // horizontal offset to advance to next glyph
};


// A renderer class for rendering text displayed by a font loaded using the
// FreeType library. A single font is loaded, processed into a list of Character
// items for later rendering.
class TextRenderer : public Component
{
public:
    // holds a list of pre-compiled Characters
    std::map<char, Character> Characters;
    // constructor
    TextRenderer(std::string text, glm::vec2 position, int fontSize, glm::vec3 color);
    ~TextRenderer();
    // pre-compiles a list of characters from the given font
    void Load(std::string font, unsigned int fontSize);
    // renders a string of text using the precompiled list of characters
    void RenderText();

    void Update(float deltaTime);

    void afterLoadedScene();

    int fontSize;
    std::string text;
    glm::vec2 position;
    glm::vec2 scale;
    glm::vec3 color;


    

private:
    // render state
    unsigned int VAO, VBO;
};


#endif //TEXTRENDERER_H
