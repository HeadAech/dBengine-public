#ifndef SPRITE_H
#define SPRITE_H

#include "Components/Control/Control.h"
#include <glm/glm.hpp>
#include <Material/Material.h>


namespace UI {
	
	class Sprite : public Control
	{
		public:

			Sprite();
			~Sprite();

			void Render() override;
            void Update(float deltaTime) override;

			// properties

			glm::vec4 ModulateColor = { 0.0f, 0.0f, 0.0f, 1.0f };

			glm::vec4 Clipping = { 0, 0, 0, 0 };

			void SetTexture(std::string_view path);
            void SetTexture(std::shared_ptr<Texture> texture);

			bool HasTexture();

			std::shared_ptr<Texture> GetTexture();

			void RemoveTexture();

        private:
            void setupMesh();
			GLuint m_VAO = 0;
			GLuint m_VBO = 0;

			bool m_HasTexture = false;

			std::shared_ptr<Texture> m_Texture;
	};

}

#endif // !SPRITE_H
