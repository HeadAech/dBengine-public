#pragma once

#include <Serializers/SerializeTypes.h>
#include <yaml-cpp/yaml.h>
#include <memory>
#include <string>
#include <vector>
#include <dBengine/EngineDebug/EngineDebug.h>
#include <Serializers/SerializeEmitters.h>

namespace Serialize {
    class MaterialSerializer {

    public:
        static MaterialSerializer &GetInstance();
        MaterialSerializer();
        void SetMaterial(Material *material);
        void Serialize(const std::string &filepath, Material* material);
        bool Deserialize(const std::string &filepath, Material* material);
        void SerializeForModel(YAML::Emitter &out, Material *material);
        bool DeserializeForModel(const YAML::Node &node, Material *material);
        
    private:
        Material *material = nullptr;
    };
} // namespace Serialize
