#ifndef LOCAL_LIGHT_H
#define LOCAL_LIGHT_H

#include "Light.h"

class LocalLight : public Light {

	

public:
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    float shadowDistance = 50.0f;

    bool castShadows = false;

     LocalLight() = default;
    ~LocalLight() = default;

    float GetConstant();
    float GetLinear();
    float GetQuadratic();
    float GetShadowDistance();
    bool IsCastingShadows();

    void SetConstant(float newConstant);
    void SetLinear(float newLinear);
    void SetQuadratic(float newQuadratic);
    void SetShadowDistance(float newShadowDistance);
    void SetCastingShadows(bool isCasting);

};

#endif // !LOCAL_LIGHT_H