//
// Created by Hubert Klonowski on 19/03/2025.
//

#include "AudioListener.h"
#include "spdlog/spdlog.h"

AudioListener::AudioListener(glm::vec3 pos, glm::vec3 forward,
                             glm::vec3 up) {
    name = "Audio Listener";
    icon = ICON_FA_HEADPHONES;
    listener_pos = pos;
    listener_forward = forward;
    listener_up = up;
    }


AudioListener::~AudioListener() {}

void AudioListener::SetPosition(const glm::vec3 &pos) {
    listener_pos = pos;
}

void AudioListener::SetVectors(const glm::vec3 &forward, const glm::vec3 &up) {
    listener_forward = forward;
    listener_up = up;
}

void AudioListener::Update(float deltaTime) {
    listener_pos = this->gameObject->transform.GetLocalPosition();
    listener_forward = -this->gameObject->GetComponent<Camera>()->Front;
    listener_up = this->gameObject->GetComponent<Camera>()->Up;

    FMOD_3D_ATTRIBUTES attributes = {};
    attributes.position = {listener_pos.x, listener_pos.y, listener_pos.z};
    attributes.forward = {listener_forward.x, listener_forward.y, listener_forward.z};
    attributes.up = {listener_up.x, listener_up.y, listener_up.z};
    attributes.velocity = {0.0f, 0.0f, 0.0f};

    Ref::GetInstance().fmodSystem->setListenerAttributes(0, &attributes);
}