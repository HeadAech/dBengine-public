#include "PointLight.h"
#include <Signal/Signals.h>

PointLight::PointLight() { 
	name = "Point Light"; 
    icon = ICON_FA_LIGHTBULB_O;
	SetAmbient({0.55f, 0.55f, 0.55f});
    SetDiffuse({1.0f, 1.0f, 1.0f});
    SetSpecular({1.0f, 1.0f, 1.0f});
    SetIntensity(10);

    propertyWatch.InnerCutOff = false;
    propertyWatch.OuterCutOff = false;
    propertyWatch.Direction = false;
}

