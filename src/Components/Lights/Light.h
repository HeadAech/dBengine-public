#ifndef LIGHT_H
#define LIGHT_H 

#include "Component/Component.h"
#include "glm/glm.hpp"

struct PropertyWatch
{
    bool Position = true;

    bool Diffuse = true;
    bool Specular = true;
    bool Ambient = true;
    bool Intensity = true;
    bool AmbientIntensity = true;

    bool Constant = true;
    bool Linear = true;
    bool Quadratic = true;
    
    bool Direction = true;
    bool InnerCutOff = true;
    bool OuterCutOff = true;

    bool IsDirty()
    {
        return Position || Diffuse || Specular || Ambient || Intensity || AmbientIntensity || Constant || Linear || Quadratic ||
               Direction || InnerCutOff || OuterCutOff;
    }
};

class Light : public Component {

	

public:
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 ambient;


    float intensity = 1.0f;
    float ambientIntensity = 0.01f;

    PropertyWatch propertyWatch;


    Light() = default;
    virtual ~Light() = default;

    glm::vec3 GetDiffuse();
    glm::vec3 GetSpecular();
    glm::vec3 GetAmbient();
    float GetIntensity();
    float GetAmbientIntensity();

    void SetDiffuse(glm::vec3 newDiffuse);
    void SetSpecular(glm::vec3 newSpecular);
    void SetAmbient(glm::vec3 newAmbient);

    void SetIntensity(float newIntensity);
    void SetAmbientIntensity(float newIntensity);
    
};

#endif // !LIGHT_H
