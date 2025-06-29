#include "LocalLight.h"

float LocalLight::GetConstant() { return this->constant; }

float LocalLight::GetLinear() { return this->linear; }

float LocalLight::GetQuadratic() { return this->quadratic; }

float LocalLight::GetShadowDistance()
{
    return this->shadowDistance;
}

bool LocalLight::IsCastingShadows()
{
    return castShadows;
}

void LocalLight::SetConstant(float newConstant) 
{ 
    this->constant = newConstant; 
    propertyWatch.Constant = true;
}

void LocalLight::SetLinear(float newLinear) 
{ 
    this->linear = newLinear; 
    propertyWatch.Linear = true;
}

void LocalLight::SetQuadratic(float newQuadratic) 
{ 
    this->quadratic = newQuadratic; 
    propertyWatch.Quadratic = true;
}

void LocalLight::SetShadowDistance(float newShadowDistance)
{
    this->shadowDistance = newShadowDistance;
}

void LocalLight::SetCastingShadows(bool isCasting)
{ this->castShadows = isCasting; }
