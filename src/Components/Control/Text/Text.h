#ifndef TEXT_H
#define TEXT_H

#include <ft2build.h>
#include "Components/Control/Control.h"
#include FT_FREETYPE_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <map>

namespace UI {
    class Text : public Control {
    public:
        Text();
        ~Text();
        void Render() override;
        void Update(float deltaTime) override;

        void SetText(const std::string &text);
        const std::string &GetText() const;
        void SetFontSize(float size);
        float GetFontSize() const;
        void SetColor(const glm::vec3 &color);
        const glm::vec3 &GetColor() const;

        static void InitializeFont(const std::string &fontPath = "res/fonts/Lubrifont/Lubrifont-Regular.ttf");
        static void CleanupFont();

    private:
        struct Character {
            float ax, ay; // advance
            float bw, bh; // bitmap width/height
            float bl, bt; // bitmap left/top
            float tx, ty; // texture coordinates
        };

        void setupMesh();

        bool m_Setup = false;

        std::string m_Text = "Text";
        float m_FontSize = 16.0f;
        glm::vec3 m_Color = glm::vec3(1.0f, 1.0f, 1.0f);

        GLuint m_VAO = 0;
        GLuint m_VBO = 0;

        static GLuint m_FontTexture; // one atlas for every char
        static std::map<char, Character> m_Characters;
        static unsigned int m_AtlasWidth;
        static unsigned int m_AtlasHeight;
        static bool m_FontInitialized;
        static const int m_MaxAtlasWidth = 1024;
    };
} // namespace UI

#endif // TEXT_H
