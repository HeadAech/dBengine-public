#ifndef BONE_H
#define BONE_H

#include <glm/gtx/quaternion.hpp>
#include <assimp/anim.h>
#include <Helpers/Util.h>

struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

struct BoneInfo {
    int id;
    glm::mat4 offset;
};

class Bone {
    public:
        Bone(const std::string &name, int ID, const aiNodeAnim *channel);

        void Update(float animationTime);

        int GetPositionIndex(float animationTime);

        int GetRotationIndex(float animationTime);

        int GetScaleIndex(float animationTime);

        glm::mat4 GetLocalTransform();
        std::string GetBoneName() const;
        int GetBoneID();

    private:

        float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

        glm::mat4 interpolatePosition(float animationTime);

        glm::mat4 interpolateRotation(float animationTime);

        glm::mat4 interpolateScaling(float animationTime);

        std::vector<KeyPosition> m_Positions;
        std::vector<KeyRotation> m_Rotations;
        std::vector<KeyScale> m_Scales;

        int m_NumPositions;
        int m_NumRotations;
        int m_NumScalings;

        glm::mat4 m_LocalTransform;
        std::string m_Name;
        int m_ID;
};

#endif // !BONE_H
