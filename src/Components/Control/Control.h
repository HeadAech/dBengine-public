#ifndef CONTROL_H
#define CONTROL_H

#include <Component/Component.h>
#include <glm/glm.hpp>
#include <memory>
#include "Shader/Shader.h"

namespace UI {
    
    enum class Anchor
    {
        TopLeft,
        TopCenter,
        TopRight,
        MiddleLeft,
        MiddleCenter,
        MiddleRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };
    
    /// <summary>
    /// Base class for UI components like buttons, sliders, etc.
    /// </summary>
    class Control : public Component {

    public:
        Control();
        ~Control();

        glm::vec2 Position = {0.0f, 0.0f};
        glm::vec2 Size = {1.0f, 1.0f};
        float Rotation = 0.0f;

		float Emission = 1.0f;

		void SetShader(std::shared_ptr<Shader> pShader);
		std::shared_ptr<Shader> GetShader() const;

        Anchor anchor = Anchor::TopLeft;

        glm::vec2 GetAnchoredPosition() const;

		private:
		
			std::shared_ptr<Shader> p_Shader;
	};
}

#endif // !CONTROL_H
