
#version 410 core
out vec4 FragColor;
out vec4 BrightColor;

#define MAX_SHADOW_CASTERS 32

const float PI = 3.14159265359;
const float Epsilon = 0.00001;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
    flat int visible;
    vec3 LocalPos;
    vec3 LocalNormal;
    //vec4 FragPosLightSpaces[MAX_SHADOW_CASTERS];
} fs_in;

struct Material {
    sampler2D diffuse;
    sampler2D normal;
    sampler2D metallic;
    sampler2D roughness;
    sampler2D ambientOcclusion;
    sampler2D specular;
    sampler2D height;
    sampler2D emissive;

    float shininess;
};


struct MaterialMap {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    float specular;
    float height;
    vec3 emissive;

};

struct DirLight {
    bool isOn;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
    float ambientIntensity;
};

struct PointLight {
    bool isOn;
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float intensity;
    float ambientIntensity;
};

struct SpotLight {
    bool isOn;
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float intensity;
    float ambientIntensity;
};

#define MAX_POINT_LIGHTS 32
#define MAX_SPOT_LIGHTS 32

uniform int numPointLights;
uniform int numSpotLights;

uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform Material material;

uniform sampler2DArray shadowMapArray;
uniform samplerCubeArray shadowCubeMapArray;

uniform float farPlane;

layout(std140) uniform LightSpaceData {
    mat4 lightSpaceMatrices[MAX_SHADOW_CASTERS];
    vec4 lightPositions[MAX_SHADOW_CASTERS]; // vec4 dla std140 alignment
};


uniform vec3 viewPos;

uniform bool u_Triplanar = false;
uniform float u_TriplanarRepeat = 1.0;

// Ibl stuff
uniform samplerCube irradianceMap;
uniform bool useIrradiance;

uniform float u_FadeStart;

// Functions for PBR: Cook-Torrance BRDF and other helpers

// normal distribution calculation
// tells how many microfacets are aligned with normal H vector
// shape of the reflection based on roughness
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / max(denom, 0.001);
}

// occlusion
// checks how much light actually gets to the surface and be reflected
// used when others occlude others
// self shadowing
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

// joins two geometry factors for observer and light source,, shadows
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// calculates how much light is reflected based on the viewing angle
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}



float ShadowCalculation(int lightIndex, vec4 lightPos, vec3 lightDir, vec3 normal)  
{  
  vec4 fragPosLightSpace  =  lightSpaceMatrices[lightIndex] * vec4(fs_in.FragPos, 1.0);  

  float shadow = 0.0f;  
  // Sets lightCoords to cull space  
  vec3 lightCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;  

  if(lightCoords.z <= 1.0f)  
  {  
      // Get from [-1, 1] range to [0, 1] range just like the shadow map  
      lightCoords = (lightCoords + 1.0f) / 2.0f;  
      float currentDepth = lightCoords.z;  
      // Prevents shadow acne  
      float bias = max(0.00025f * (1.0 - dot(normal, lightDir)), 0.0005f);

      // Smoothens out the shadows with random offset  
      int sampleRadius = 2;
      vec2 pixelSize = 1.0 / textureSize(shadowMapArray, 0).xy;
      for(int y = -sampleRadius; y <= sampleRadius; y++)  
      {  
          for(int x = -sampleRadius; x <= sampleRadius; x++)  
          {  

              vec3 sampleCoord = vec3(lightCoords.xy + vec2(x, y) * pixelSize, float(lightIndex));
            
             float closestDepth = texture(shadowMapArray, sampleCoord).r;
              if (currentDepth > closestDepth + bias)  
                  shadow += 1.0f;
          }      
      }  
      // Get average shadow  
      shadow /= pow((sampleRadius * 2 + 1), 2);
      shadow = clamp(shadow, 0.0, 0.95);
  }  

  return shadow;  
}

float BlinnPhongSpecular(vec3 lightDir, vec3 normal, vec3 viewDir, float shininess) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    return spec;
}


vec3 CalculateDirectionalPBR(vec3 N, vec3 V, MaterialMap materialMap, DirLight dirLight, float shadow)
{
	if (!dirLight.isOn) return vec3(0);
    vec3 L = normalize(-dirLight.direction); 
    vec3 H = normalize(V + L);    

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, materialMap.albedo, materialMap.metallic);
    float NDF = DistributionGGX(N, H, materialMap.roughness);
    float G   = GeometrySmith(N, V, L, materialMap.roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F; // sepcular factor
    vec3 kD = vec3(1.0) - kS; //diffuse factor
    kD *= 1.0 - materialMap.metallic;

    float NdotL = max(dot(N, L), 0.0); // cos angle normal-light
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
    vec3 specular = numerator / denominator;
    // cook-terrance brdf specular reflections

    vec3 Lo = (kD * materialMap.albedo / PI + specular) * dirLight.diffuse * NdotL;
    
	Lo *= (1.0 - shadow) * dirLight.intensity;

	float aoTested = materialMap.ao == 0 ? 1 : materialMap.ao;

    // Ambient (with AO)
    vec3 ambient = vec3(0.03) * materialMap.albedo * aoTested * dirLight.ambient * dirLight.ambientIntensity;
    return ambient + Lo;
}

vec3 CalcPointLightPBR(PointLight light, vec3 N, vec3 fragPos, vec3 V, int idx, MaterialMap materialMap)
{
    if (!light.isOn)
        return vec3(0.0);

    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);
    
    vec3 F0 = mix(vec3(0.04), materialMap.albedo, materialMap.metallic);
    float NDF = DistributionGGX(N, H, materialMap.roughness);
    float G   = GeometrySmith(N, V, L, materialMap.roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = (NDF * G * F) / denom;
    //specular *= materialMap.specular;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - materialMap.metallic);

    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = kD * materialMap.albedo / PI;
    vec3 Lo = (diffuse + specular) * light.diffuse * NdotL;

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    Lo *= attenuation * light.intensity;

	float shadow = 0.0;
    vec3 fragToLight = light.position - fragPos;
    float currentDepth = length(fragToLight); // normalized depth in [0,1]
    
    //float bias = max(0.01f * (0.01f - dot(N, L)), 0.001);
    float angle = max(dot(N, L), 0.0);
    float bias = clamp(0.15 * (1.0 - angle), 0.01, 0.001);

	// Not really a radius, more like half the width of a square
	int sampleRadius = 2;
	float offset = 0.05;
    float samples = 0;

	for(int z = -sampleRadius; z <= sampleRadius; z++)
	{
		for(int y = -sampleRadius; y <= sampleRadius; y++)
		{
		    for(int x = -sampleRadius; x <= sampleRadius; x++)
		    {
                
                vec3 sampleOffset = vec3(x, y, z) * offset;

                vec3 dir = -(fragToLight + sampleOffset);

		        float closestDepth = 0.0;
                closestDepth = texture(shadowCubeMapArray, vec4(normalize(dir), float(idx))).r;
               
				// Remember that we divided by the farPlane?
				// Also notice how the currentDepth is not in the range [0, 1]
				closestDepth *= farPlane;
				if (currentDepth - bias > closestDepth) {
					shadow += 2.0;
                }
                samples += 2.0;
		    }    
		}
	}

	// Average shadow
	shadow /= samples;
    shadow = clamp(shadow, 0.0, 0.99);
    float camDist = length(viewPos - fs_in.FragPos);
    float fadeEnd = u_FadeStart * 1.1f;
    float fade = clamp(
        (fadeEnd - camDist) 
        / (fadeEnd - u_FadeStart),
        0.0,
        1.0
    );
    shadow *= fade;

    Lo *= (1.0 - shadow) * light.intensity;
	
	float aoTested = materialMap.ao == 0 ? 1 : materialMap.ao;

    vec3 ambient = light.ambient * light.ambientIntensity * materialMap.albedo * aoTested;
    return ambient + Lo;
}

vec3 CalcSpotLightPBR(SpotLight light, vec3 N, vec3 fragPos, vec3 V, int lightIndex, MaterialMap materialMap)
{
    if (!light.isOn)
        return vec3(0.0);

    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);


    // Cook-Torrance BRDF
    vec3 F0 = mix(vec3(0.04), materialMap.albedo, materialMap.metallic);
    float NDF = DistributionGGX(N, H, materialMap.roughness);
    float G   = GeometrySmith(N, V, L, materialMap.roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float denom = 4.0 * NdotV * NdotL + 0.001;
    vec3 specular = (NDF * G * F) / denom;
    //specular *= materialMap.specular;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - materialMap.metallic);

    vec3 diffuse = kD * materialMap.albedo / PI;
    vec3 Lo = (diffuse + specular) * light.diffuse * NdotL;
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Spotlight intensity (cutoff)
    float theta = dot(L, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // Shadow
    float shadow = ShadowCalculation(lightIndex, lightPositions[lightIndex], L, N); // Or just 0.0 if no shadow

    Lo *= attenuation * intensity * light.intensity * (1.0 - shadow);

    vec3 ambient = light.ambient * light.ambientIntensity * materialMap.albedo * materialMap.ao;

    return ambient + Lo;
}

vec3 triplanarBlendWeights(vec3 N) {
    vec3 w = abs(N);
    w = pow(w, vec3(8.0));
    return w / (w.x + w.y + w.z);
}

vec4 triplanarSampleRGBA(sampler2D tex, vec3 pos, vec3 normal, float scale, vec4 channelMask) {
    vec3 w = triplanarBlendWeights(normal);
    vec2 uvXY = pos.xy * scale;
    vec2 uvYZ = pos.zy * scale;
    vec2 uvZX = pos.xz * scale;
    vec4 cXY = texture(tex, uvXY) * channelMask;
    vec4 cYZ = texture(tex, uvYZ) * channelMask;
    vec4 cZX = texture(tex, uvZX) * channelMask;
    return cXY * w.z + cYZ * w.x + cZX * w.y;
}

vec3 triplanarNormal(sampler2D normalTex, vec3 pos, vec3 worldN, float scale) {
    vec3 w = triplanarBlendWeights(worldN);
    vec2 uvXY = pos.xy * scale;
    vec2 uvYZ = pos.zy * scale;
    vec2 uvZX = pos.xz * scale;
    vec3 nXY = texture(normalTex, uvXY).rgb * 2.0 - 1.0;
    vec3 nYZ = texture(normalTex, uvYZ).rgb * 2.0 - 1.0;
    vec3 nZX = texture(normalTex, uvZX).rgb * 2.0 - 1.0;

    vec3 worldN_XY = normalize(nXY.x * vec3(1,0,0) + nXY.y * vec3(0,1,0) + nXY.z * vec3(0,0,1));
    vec3 worldN_YZ = normalize(nYZ.x * vec3(0,0,1) + nYZ.y * vec3(0,1,0) + nYZ.z * vec3(1,0,0));
    vec3 worldN_ZX = normalize(nZX.x * vec3(1,0,0) + nZX.y * vec3(0,0,1) + nZX.z * vec3(0,1,0));

    return normalize(worldN_XY * w.z + worldN_YZ * w.x + worldN_ZX * w.y);
    //vec3 blended = normalize(worldN_XY * w.z + worldN_YZ * w.x + worldN_ZX * w.y);
    //return normalize(fs_in.TBN * blended);
}

MaterialMap GetMaterialMap()
{
    MaterialMap materialMap;

    materialMap.albedo     = pow(texture(material.diffuse, fs_in.TexCoords).rgb, vec3(2.2)); // gamma correction
	materialMap.metallic  = texture(material.metallic, fs_in.TexCoords).r;
	materialMap.roughness = texture(material.roughness, fs_in.TexCoords).r;
    materialMap.ao        = texture(material.ambientOcclusion, fs_in.TexCoords).r;
    materialMap.specular = texture(material.specular, fs_in.TexCoords).r;
    materialMap.height = texture(material.height, fs_in.TexCoords).r;
    materialMap.emissive = texture(material.emissive, fs_in.TexCoords).rgb;

    return materialMap;

}

MaterialMap GetTriplanarMaterialMap()
{
    MaterialMap materialMap;

    vec3 localN = normalize(fs_in.LocalNormal);
    
    vec3 rawAlbedo = triplanarSampleRGBA(material.diffuse, fs_in.LocalPos, localN, u_TriplanarRepeat, vec4(1,1,1,1)).rgb;
    materialMap.albedo    = pow(rawAlbedo, vec3(2.2));
    materialMap.metallic  = triplanarSampleRGBA(material.metallic, fs_in.LocalPos,  localN, u_TriplanarRepeat, vec4(1,0,0,0)).r;
    materialMap.roughness = triplanarSampleRGBA(material.roughness, fs_in.LocalPos,  localN, u_TriplanarRepeat, vec4(1,0,0,0)).r;
    materialMap.ao        = triplanarSampleRGBA(material.ambientOcclusion, fs_in.LocalPos,  localN, u_TriplanarRepeat, vec4(1,0,0,0)).r;
    materialMap.specular  = triplanarSampleRGBA(material.specular, fs_in.LocalPos,  localN, u_TriplanarRepeat, vec4(1,0,0,0)).r;
    materialMap.height    = triplanarSampleRGBA(material.height, fs_in.LocalPos,  localN, u_TriplanarRepeat, vec4(1,0,0,0)).r;
    materialMap.emissive  = triplanarSampleRGBA(material.emissive, fs_in.LocalPos,  localN, u_TriplanarRepeat, vec4(1,1,1,1)).rgb;
    
    return materialMap;
}


void main()
{

	vec3 geomN = normalize(fs_in.Normal);

    vec3 norm = texture(material.normal, fs_in.TexCoords).rgb;
    
    if (all(equal(norm, vec3(0.0))) || all(equal(norm, vec3(1.0)))) {
        //FragColor = vec4(1.0, 0.0, 1.0, 1.0); // Magenta if texture is invalid
        norm = geomN;
    } else 
    {
        norm = norm * 2.0 - 1.0;

    }

	vec3 N = u_Triplanar
    ? triplanarNormal(material.normal, fs_in.LocalPos, normalize(fs_in.LocalNormal), u_TriplanarRepeat)
    : normalize(fs_in.TBN * norm);

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    //FragColor = vec4(N * 0.5 + 0.5, 1.0);
   // return;
   
    MaterialMap materialMap = u_Triplanar ? GetTriplanarMaterialMap() : GetMaterialMap();



	vec3 V = normalize(viewPos - fs_in.FragPos); // Wektor do kamery

    int lightIdx = 0;
    //vec3 result = CalcDirLight(dirLight, norm, viewDir, lightIdx, lightPositions[lightIdx]);
	float dirLightShadow = ShadowCalculation(lightIdx, lightPositions[lightIdx], normalize(dirLight.direction), N);

    vec3 result = CalculateDirectionalPBR(N, V, materialMap, dirLight, dirLightShadow);

	lightIdx++;
    // phase 2: point lights
    for(int i = 0; i < numPointLights; i++) {
        result += CalcPointLightPBR(pointLights[i], N, fs_in.FragPos, viewDir, i, materialMap);
    }
    // phase 3: spot light
    for(int i = 0; i < numSpotLights; i++) {
        //result += CalcSpotLight(spotLights[i], norm, fs_in.FragPos, viewDir, lightIdx, lightPositions[lightIdx]);
        result += CalcSpotLightPBR(spotLights[i], N, fs_in.FragPos, V, lightIdx, materialMap);
        lightIdx++;
    }
    
    //IBL stuff
    if(useIrradiance){
    vec3 N_ir = normalize(fs_in.Normal);
    vec3 V_ir = normalize(viewPos - fs_in.FragPos);
    vec3 R = reflect(-V_ir, N_ir); // Do specular ibl-a pozniej.

    vec3 F0 = mix(vec3(0.04), materialMap.albedo, materialMap.metallic);
    vec3 kS = fresnelSchlick(max(dot(N_ir, V_ir), 0.0), F0);
    vec3 kD = (1.0 - kS) * (1.0 - materialMap.metallic);
    vec3 irradiance = texture(irradianceMap, N_ir).rgb;
    vec3 diffuse = irradiance*0.1 * materialMap.albedo * kD;
    float aoTested = materialMap.ao == 0 ? 1 : materialMap.ao;
    vec3 ambient = diffuse * aoTested;
    result += ambient;
    

    //Debug stuff
    //FragColor = vec4(irradiance, 1.0); //Irradiance map
    //FragColor = vec4(N_ir * 0.5 + 0.5, 1.0); // Normals
    }

    //float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    //BrightColor = vec4(result, 1.0);
//    if(brightness > 1.0)
//        BrightColor = vec4(result, 1.0);
//    else
//        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
//
    FragColor = vec4(result, 1.0);
}
