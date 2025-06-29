#version 410 core
in vec3 Position;
out vec4 FragColor;

uniform vec3 cameraPos;
uniform float density;
uniform int samples;
uniform vec3 fogColor;
uniform float scattering;
uniform mat4 model;
uniform samplerCube skybox;
uniform vec3 aabbMin;       // world space
uniform vec3 aabbMax;       // world space
uniform vec3 aabbMinLocal;  // model space
uniform vec3 aabbMaxLocal;  // model space

void main() {
    vec3 rayDir = normalize(Position - cameraPos);
    vec3 invDir = 1.0 / rayDir;

    vec3 cubeMin = aabbMin;
    vec3 cubeMax = aabbMax;

    // Ray intersection with AABB
    vec3 t0s = (cubeMin - cameraPos) * invDir;
    vec3 t1s = (cubeMax - cameraPos) * invDir;
    vec3 tsm = min(t0s, t1s);
    vec3 tgr = max(t0s, t1s);
    float tNear = max(max(tsm.x, tsm.y), tsm.z);
    float tFar = min(min(tgr.x, tgr.y), tgr.z);

    if (tNear >= tFar || tFar <= 0.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }
    tNear = max(tNear, 0.0);

    // Ray-march
    float dt = (tFar - tNear) / float(samples);
    vec3 pos = cameraPos + rayDir * (tNear + dt * 0.5);
    float trans = 1.0;
    vec3 col = vec3(0.0);

    mat4 invModel = inverse(model);

    for (int i = 0; i < samples; ++i) {
        vec3 localPos = (invModel * vec4(pos, 1.0)).xyz;

        // density AABB model space
        vec3 aabbCenterLocal = (aabbMinLocal + aabbMaxLocal) * 0.5;
        vec3 aabbExtentsLocal = (aabbMaxLocal - aabbMinLocal) * 0.5;
        vec3 dist = abs(localPos - aabbCenterLocal) / aabbExtentsLocal;
        float maxDist = max(max(dist.x, dist.y), dist.z);
        float edgeDist = 1.0 - maxDist;
        float falloff = smoothstep(-0.1, 0.1, edgeDist);
        float localDensity = density * falloff;

        // Scatter
        float scatter = scattering;
        vec3 contrib = fogColor * localDensity * scatter * trans * dt;
        col += contrib;
        trans *= exp(-localDensity * dt);
        pos += rayDir * dt;
    }

    FragColor = vec4(col, 1.0 - trans);
}