#include "Light.h"

glm::vec3 Light::GetDiffuse() { return this->diffuse; }

glm::vec3 Light::GetSpecular() { return this->specular; }

glm::vec3 Light::GetAmbient() { return this->ambient; }

float Light::GetIntensity() { return this->intensity; }

float Light::GetAmbientIntensity() { return this->ambientIntensity; }

void Light::SetAmbient(glm::vec3 newAmbient) 
{ 
	this->ambient = newAmbient; 
	propertyWatch.Ambient = true;
}

void Light::SetDiffuse(glm::vec3 newDiffuse) 
{ 
	this->diffuse = newDiffuse; 
	propertyWatch.Diffuse = true;
}

void Light::SetSpecular(glm::vec3 newSpecular) 
{ 
	this->specular = newSpecular; 
	propertyWatch.Specular = true;
}

void Light::SetIntensity(float newIntensity) 
{ 
	this->intensity = newIntensity; 
	propertyWatch.Intensity = true;
}

void Light::SetAmbientIntensity(float newIntensity) 
{ 
	this->ambientIntensity = newIntensity; 
	propertyWatch.AmbientIntensity = true;
}

