#pragma once
#include "yaml-cpp/yaml.h"
#include <ResourceManager/ResourceManager.h>
#include "Serializers/SerializeTypes.h"
#include <Components/Animator/AnimationLibrary/AnimationLibrary.h>

namespace YAML {

    // glm::vec4
    template<>
    struct convert<glm::vec4> {
        static Node encode(const glm::vec4 &rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node &node, glm::vec4 &rhs) {
            if (!node.IsSequence() || node.size() != 4)
                return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

    // glm::vec3
    template<>
    struct convert<glm::vec3> {
        static Node encode(const glm::vec3 &rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node &node, glm::vec3 &rhs) {
            if (!node.IsSequence() || node.size() != 3)
                return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    // glm::vec2
    template<>
    struct convert<glm::vec2> {
        static Node encode(const glm::vec2 &rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            return node;
        }

        static bool decode(const Node &node, glm::vec2 &rhs) {
            if (!node.IsSequence() || node.size() != 2)
                return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    // glm::ivec2
    template<>
    struct convert<glm::ivec2> {
        static Node encode(const glm::ivec2 &rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            return node;
        }

        static bool decode(const Node &node, glm::ivec2 &rhs) {
            if (!node.IsSequence() || node.size() != 2)
                return false;
            rhs.x = node[0].as<int>();
            rhs.y = node[1].as<int>();
            return true;
        }
    };

    // glm::quat
    template<>
    struct convert<glm::quat> {
        static Node encode(const glm::quat &rhs) {
            Node node;
            node.push_back(rhs.w);
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node &node, glm::quat &rhs) {
            if (!node.IsSequence() || node.size() != 4)
                return false;
            rhs.w = node[0].as<float>();
            rhs.x = node[1].as<float>();
            rhs.y = node[2].as<float>();
            rhs.z = node[3].as<float>();
            return true;
        }
    };

    // glm::mat4
    template<>
    struct convert<glm::mat4> {
        static Node encode(const glm::mat4 &rhs) {
            Node node;
            node.SetStyle(EmitterStyle::Block);
            for (int i = 0; i < 4; ++i) {
                Node row;
                row.SetStyle(EmitterStyle::Flow);
                for (int j = 0; j < 4; ++j) {
                    row.push_back(rhs[i][j]);
                }
                node.push_back(row);
            }
            return node;
        }

        static bool decode(const Node &node, glm::mat4 &rhs) {
            if (!node.IsSequence() || node.size() != 4) {
                return false;
            }
            for (size_t i = 0; i < 4; ++i) {
                const Node &row = node[i];
                if (!row.IsSequence() || row.size() != 4) {
                    return false;
                }
                for (size_t j = 0; j < 4; ++j) {
                    rhs[i][j] = row[j].as<float>();
                }
            }
            return true;
        }
    };

    // Enum Sky Type
    template<>
    struct convert<Sky> {
        static Node encode(const Sky &type) {
            switch (type) {
                case PHYSICAL:
                    return Node("PHYSICAL");
                case HDRI:
                    return Node("HDRI");
                default:
                    return Node("");
            }
        }

        static bool decode(const Node &node, Sky &type) {
            if (!node.IsScalar()) {
                return false;
            }
            std::string str = node.as<std::string>();
            if (str == "PHYSICAL") {
                type = PHYSICAL;
            } else if (str == "HDRI") {
                type = HDRI;
            } else {
                return false;
            }
            return true;
        }
    };

    // Texture
    template<>
    struct convert<Texture> {
        static Node encode(const Texture &rhs) {
            Node node;
            node["textureID"] = rhs.id;
            node["texturePath"] = rhs.path;
            node["textureType"] = rhs.type;
            return node;
        }

        static bool decode(const Node &node, Texture &rhs) {
            if (!node.IsMap()) {
                return false;
            }
            rhs.id = node["textureID"].as<unsigned int>();
            rhs.path = node["texturePath"].as<std::string>();
            rhs.type = node["textureType"].as<std::string>();
            return true;
        }
    };

    //Animation Transition
    template<>
    struct convert<AnimationTransition> {
        static Node encode(const AnimationTransition &transition) {
            Node node;
            node["name"] = transition.name;
            node["duration"] = transition.duration;

            node["animationBase"] = transition.animationBase ? transition.animationBase->GetName() : "";
            node["animationTarget"] = transition.animationTarget ? transition.animationTarget->GetName() : "";
            return node;
        }

        static bool decode(const Node &node, AnimationTransition &transition) {

            if (!node.IsMap()) {
                return false;
            }

            if (!node["name"] || !node["duration"]) {
                return false;
            }

            transition.name = node["name"].as<std::string>();
            transition.duration = node["duration"].as<float>();

            AnimationLibrary &animLib = AnimationLibrary::GetInstance();
            transition.animationBase = animLib.GetAnimation(node["animationBase"].as<std::string>());
            transition.animationTarget = animLib.GetAnimation(node["animationTarget"].as<std::string>());

            return true;
        }
    };

} // namespace YAML

namespace Serialize {
    // glm::ivec2
    inline YAML::Emitter &operator<<(YAML::Emitter &out, const glm::ivec2 &v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    // glm::vec2
    inline YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec2 &v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }


    // glm::vec3
    inline YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec3 &v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    // glm::vec4
    inline YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec4 &v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }


    // glm::quat
    inline YAML::Emitter &operator<<(YAML::Emitter &out, const glm::quat &q) {
        out << YAML::Flow;
        out << YAML::BeginSeq << q.w << q.x << q.y << q.z << YAML::EndSeq;
        return out;
    }

    // glm::mat4
    inline YAML::Emitter &operator<<(YAML::Emitter &out, const glm::mat4 &mat) {
        out << YAML::BeginSeq;
        for (int i = 0; i < 4; ++i) {
            out << YAML::Flow;
            out << YAML::BeginSeq;
            for (int j = 0; j < 4; ++j) {
                out << mat[i][j];
            }
            out << YAML::EndSeq;
        }
        out << YAML::EndSeq;
        return out;
    }

    // time[idk if i will use it]
    inline YAML::Emitter &operator<<(YAML::Emitter &out, const std::filesystem::file_time_type &time) {
        out << YAML::BeginSeq;
        out << YAML::Flow;
        // out << ;
        out << YAML::EndSeq;
        return out;
    }

    // Texture
    inline YAML::Emitter &operator<<(YAML::Emitter &out, const Texture &texture) {
        out << YAML::BeginMap;
        out << YAML::Key << "textureID";
        out << YAML::Value << texture.id;
        out << YAML::Key << "texturePath";
        out << YAML::Value << texture.path;
        out << YAML::Key << "textureType";
        out << YAML::Value << texture.type;
        out << YAML::EndMap;
        return out;
    }

    // AnimationTransition
    inline YAML::Emitter &operator<<(YAML::Emitter &out, const AnimationTransition &animationTransition) {
        out << YAML::BeginMap;
        out << YAML::Key << "animationBase";
        out << YAML::Value << animationTransition.animationBase->GetName();
        out << YAML::Key << "animationTarget";
        out << YAML::Value << animationTransition.animationTarget->GetName();
        out << YAML::Key << "duration";
        out << YAML::Value << animationTransition.duration;
        out << YAML::Key << "name";
        out << YAML::Value << animationTransition.name;
        out << YAML::EndMap;
        return out;
    }

    
}; // namespace Serialize
