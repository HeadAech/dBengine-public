//
// Created by Hubert Klonowski on 19/03/2025.
//

#ifndef AUDIOLISTENER_H
#define AUDIOLISTENER_H
#include "Component/Component.h"
#include "Components/Camera/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "fmod.hpp"
#include "fmod_errors.h"
#include "fmod_studio.hpp"
#include "Signal/Signals.h"
#include <Singletons/Ref/Ref.h>

class AudioListener : public Component {
    private:
        Ref &ref = Ref::GetInstance();
    public:
        glm::vec3 listener_pos;
        glm::vec3 listener_vel;
        glm::vec3 listener_forward;
        glm::vec3 listener_up;

        AudioListener(glm::vec3 pos, glm::vec3 forward, glm::vec3 up);
        ~AudioListener();
        void SetPosition(const glm::vec3 &pos);
        void SetVectors(const glm::vec3 &forward, const glm::vec3 &up);
        void Update(float deltaTime) override;
};

#endif //AUDIOLISTENER_H
