#ifndef BUTTON_H
#define BUTTON_H

#include <Material/Material.h>
#include <functional>
#include <glad/glad.h>
#include "Components/Control/Control.h"

namespace UI {
    class Button : public Control {


    public:
        Button();
        ~Button();

        void Render() override;
        void Update(float deltaTime) override;


        // properties

        glm::vec3 Color = {1.0f, 1.0f, 1.0f};

        bool Pressed = false;

        void OnClick(std::function<void()> callback);


        void SetTexture(std::string_view path);
        void SetTexture(std::shared_ptr<Texture> texture);

        void SetHoverTexture(std::string_view path);
        void SetHoverTexture(std::shared_ptr<Texture> texture);

        bool HasTexture();

        std::shared_ptr<Texture> GetTexture();

        std::shared_ptr<Texture> GetHoverTexture();
        
        void RemoveTexture();
        void RemoveHoverTexture();

    private:
        bool isHovered(glm::vec2 &mousePos);

        void setupMesh();


        std::function<void()> m_OnClickCallback;


        GLuint m_VAO = 0;
        GLuint m_VBO = 0;

		bool m_HasTexture = false;

		std::shared_ptr<Texture> m_Texture;
        std::shared_ptr<Texture> m_HoverTexture;

	
	};
}

#endif // !BUTTON_H
