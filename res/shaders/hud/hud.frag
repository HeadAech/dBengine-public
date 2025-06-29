#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_BloomBlur;
uniform float bloomStrength = 0.04f;

uniform sampler2D u_HUDTexture;    
uniform float     u_FisheyeStrength = 0.05;
uniform float     u_ChromaticStrength = 0.001;
uniform vec2      u_Resolution;
  
// shadow parameters
uniform vec3      u_ShadowColor = vec3(0, 0.0, 1);    
uniform float     u_ShadowRadius = 0.0001;   
uniform float     u_ShadowIntensity = 0.05;
uniform vec2      u_ShadowOffset = vec2(0.005, 0.01);

uniform float     u_Time;

uniform float     u_DisplacementStrength = 0.004; // max shift in UVs, e.g. 0.03
uniform float     u_DisplacementSpeed = 10.0;    // speed of glitch changes, e.g. 10.0
uniform float     u_ScanlineHeight = 3.0;      // height of each band in pixels, e.g. 4.0
uniform float     u_ScanlineProbability = 0.002;// chance (0–1) that a band glitches each frame, e.g. 0.2

uniform bool u_DistortionEnabled = true;


const vec2 OFFSETS[8] = vec2[](
    vec2( 1.0, 0.0), vec2(-1.0, 0.0),
    vec2( 0.0, 1.0), vec2( 0.0,-1.0),
    vec2( 0.7, 0.7), vec2(-0.7, 0.7),
    vec2( 0.7,-0.7), vec2(-0.7,-0.7)
);

float hash12(vec2 p) {
    float h = dot(p, vec2(127.1, 311.7));
    return fract(sin(h + u_Time * u_DisplacementSpeed) * 43758.5453123);
}
vec2 applyFisheye(vec2 uv, float strength) {
    vec2 p = uv * 2.0 - 1.0;
    float r = length(p);
    float k = 1.0 + strength * r * r;
    p /= k;
    return (p + 1.0) * 0.5;
}

vec3 bloom(vec2 coords) {
    vec3 hudColor = texture(u_HUDTexture, coords).rgb;
    vec3 bloomColor = texture(u_BloomBlur, coords).rgb;
    return mix(hudColor, bloomColor, bloomStrength); // linear interpolation
}

void main(){
    if (!u_DistortionEnabled)
    {
        //FragColor = texture(u_HUDTexture, TexCoords);
        vec4 hud = texture(u_HUDTexture, TexCoords);
        vec3 bloomCol = texture(u_BloomBlur, TexCoords).rgb;
        vec3 finalCol = hud.rgb + bloomCol * bloomStrength;
        FragColor = vec4(finalCol, hud.a);
        return;
    }

     // 1) fisheye warp
    vec2 uv = applyFisheye(TexCoords, u_FisheyeStrength);

    // circular mask
    vec2 center = vec2(0.5);
    float d = length(uv - center);
    if (d > 1.0){
        vec3 bloomCol = texture(u_BloomBlur, uv).rgb;
        FragColor = vec4(bloomCol * bloomStrength, 1.0);
        discard;
    }

    // 2) determine displacement for this scanline
    float pixelY = uv.y * u_Resolution.y;
    float bandID = floor(pixelY / u_ScanlineHeight);
    // random for this band
    float rnd = hash12(vec2(bandID, 0.0));
    float doGlitch = step(1.0 - u_ScanlineProbability, rnd);
    // shift amount
    float shiftAmount = (hash12(vec2(bandID, 1.0)) * 2.0 - 1.0)
                        * u_DisplacementStrength * doGlitch;
    uv.x += shiftAmount;

    // 3) soft shadow glow (unchanged)
    float shadow = 0.0;
    for(int i=0;i<8;++i){
         float r1 = hash12(uv + OFFSETS[i] * 13.1);
        float r2 = hash12(uv + OFFSETS[i] * 17.7);
        vec2 noise = (vec2(r1, r2) - 0.5) * 0.005;

        vec2 sampleUV = uv
                      + OFFSETS[i] * u_ShadowRadius
                      + u_ShadowOffset
                      + noise;
        sampleUV = clamp(sampleUV, 0.0, 1.0);
        shadow += texture(u_HUDTexture, sampleUV).a;
    }
    shadow = (shadow/8.0) * u_ShadowIntensity;

    // 4) animated chromatic aberration
    vec2 dir   = normalize(max(uv - center, 1e-6));
    float base = u_ChromaticStrength * d;
    float wob  = sin(u_Time*3.0 + d*12.0) * base * 0.3;
    vec2 uvR   = uv + dir*(base + wob);
    vec2 uvB   = uv - dir*(base - wob);

    float r = texture(u_HUDTexture, uvR).r;
    float g = texture(u_HUDTexture, uv ).g;
    float b = texture(u_HUDTexture, uvB).b;
    float a = texture(u_HUDTexture, uv ).a;

    // 5) composite glow + UI
    vec3 colorWithBloom = bloom(uv);
    vec3  col = mix(u_ShadowColor * shadow, colorWithBloom, a);
    float outA = max(shadow, a);

    FragColor = vec4(col, outA);
}