#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <glm/glm.hpp>
#include "Component/Component.h"
#include "Shader/Shader.h"
#include <vector>
#include <memory>
#include <Material/Material.h>

struct Particle
{
    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec4 Color;
    float LifeTime;
    float Size;

    float InitialLifeTime;
};

struct ParticleInstanceData
{
    glm::vec3 Offset;
    float Size;
    glm::vec4 Color;
};

enum class EmissionShapeType
{
    Point,
    Sphere,
    Cube
};

class ParticleSystem : public Component
{

    public:
        ParticleSystem();
        ~ParticleSystem() override;
        
        void Update(float deltaTime) override;

        void Render() override;

        void SetMaxParticles(int maxParticles);
        void SetSpawnRate(float spawnRate);
        void SetTexture(std::shared_ptr<Texture> texture);
        void SetShader(std::shared_ptr<Shader> shaderProgram);

        void Init();

        bool IsInitialized();

        void Emit();
        void Stop();

        bool IsEmitting() const;

        std::shared_ptr<Texture> GetTexture() const;

        EmissionShapeType EmissionShape = EmissionShapeType::Sphere;
        bool DrawEmissionShape = false;
        
        // for cube shape
        glm::vec3 CubeSize = glm::vec3(1.0f); // width, height, depth (x, y, z)
        // for sphere shape
        float SphereRadius = 1.0f;
        bool SphereSurfaceOnly = false;

         // properties
        bool Emitting = false;
        unsigned int MaxParticles = 100;
        float SpawnRate = 0;

        bool OneShot = false;

        float MinLifeTime = 0.5f;
        float MaxLifeTime = 1.0f;


        float MinSize = 1.0f;
        float MaxSize = 1.0f;

        bool ScaleOverTime = false;
        float BeginScaling = 0.5f;

        bool FadeOverTime = false;
        float BeginFading = 0.5f;

        glm::vec3 Gravity = glm::vec3(0.0f, -9.81f, 0.0f);

        glm::vec4 Albedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        glm::vec3 MinInitialVelocity = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 MaxInitialVelocity = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 CameraPosition;

        bool BlendingAdditive = false;
        float EmissionMultiplier = 1.0f;

        bool UseLocalSpace = false;


        bool BillboardY = false;

        bool m_Initialized = false;
        std::shared_ptr<Texture> m_Texture;

    private:
        
        void cleanup();

        void initParticles();
        void initBuffers();

        int firstUnusedParticle();

        void respawnParticle(Particle &particle, const glm::vec3 &emitterPos, const glm::mat4& rotationMatrix);

        void correctRanges();

        void drawDebug();



        std::vector<Particle> m_Particles;

        
        float m_SpawnAccumulator = 0;

        int m_LastUsedParticle = 0;

        // shaders
        std::shared_ptr<Shader> m_Shader;
        
        // buff
        unsigned int m_VAO = 0;
        unsigned int m_QuadVBO = 0;
        unsigned int m_InstanceVBO = 0;

       

};

#endif // !PARTICLE_SYSTEM_H
