#ifndef SCENESERIALIZER_H
#define SCENESERIALIZER_H

#pragma once

#include <Serializers/SerializeTypes.h>
#include <yaml-cpp/yaml.h>
#include <memory>
#include <string>
#include <vector>
#include <dBengine/EngineDebug/EngineDebug.h>
#include <Components/Animator/AnimationLibrary/AnimationLibrary.h>
#include <Serializers/SerializeEmitters.h>

#include <yaml-cpp/yaml.h>
namespace Serialize {
    class SceneSerializer {
    public:
        static SceneSerializer &GetInstance();

        SceneSerializer();

        bool Serialize(Scene *scene, const std::string &filePath);
        bool DeserializeToScene(Scene *scene, const std::string &filePath);
        bool DeserializeToMap(Scene *scene, const std::string &filePath);
        bool createGameObjectFromScene(Scene *scene, const std::string &filePath, GameObject *parent, bool overwriteUUID = false);
    private:
        
        void SetScene(Scene *scene);

        bool m_SerializeGameObject(YAML::Emitter &out, GameObject *gameObject);

        bool m_SerializeComponent(YAML::Emitter &out, Component *component);
        bool m_SerializeTransformComponent(YAML::Emitter &out, GameObject *gameObject);
        bool m_SerializeComponentDefaultData(YAML::Emitter &out, Component *component);
        bool m_SerializeLightComponentDefaultData(YAML::Emitter &out, Light *component);
        bool m_SerializeControlComponentDefaultData(YAML::Emitter &out, UI::Control *component);

        bool m_DeserializeGameObject(const YAML::Node &gameObject, GameObject *object, bool isAppended = false, bool overwriteUUID = false, const std::string &realChildUUID = "");

        bool m_DeserializeComponent(const YAML::Node &component, GameObject *gameObject, const std::string &type);
        bool m_DeserializeTransformComponent(const YAML::Node &node, GameObject *gameObject);
        bool m_DeserializeComponentDefaultData(const YAML::Node &node, Component *component);
        bool m_DeserializeLightComponentDefaultData(const YAML::Node &node, Light *component);
        bool m_DeserializeControlComponentDefaultData(const YAML::Node &node, UI::Control *component);

        Scene *scene = nullptr;
        std::unordered_map<std::string, YAML::Node> _rootObjects;
        std::unordered_map<std::string, unsigned long long int> _modifyTimes;

    };
} // namespace Serialize

#endif // SCENESERIALIZER_H