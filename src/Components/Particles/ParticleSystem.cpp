#include "ParticleSystem.h"
#include <glad/glad.h>
#include "GameObject/GameObject.h"
#include <Helpers/Util.h>
#include <Helpers/Random.h>
#include <Helpers/DebugDraw.h>

ParticleSystem::ParticleSystem()
{ 
    name = "Particle System";
    icon = ICON_FA_LINODE;
    m_Particles.resize(MaxParticles);
}

ParticleSystem::~ParticleSystem() { cleanup(); }

void ParticleSystem::Update(float deltaTime) 
{

    if (!m_Initialized) {
        Init();
    }
    
    // correctRanges();

    const glm::vec3 emitterPos = gameObject->transform.GetGlobalPosition();

    glm::mat4 parentRot = glm::mat4(1);
    if (gameObject->parent)
        parentRot = glm::mat4_cast(gameObject->parent->transform.GetQuatRotation());

    glm::mat4 rotationMatrix = parentRot * glm::mat4_cast(gameObject->transform.GetQuatRotation());

    if (Emitting && !OneShot)
    {

        m_SpawnAccumulator += SpawnRate * deltaTime;
        int newParticles = static_cast<int>(m_SpawnAccumulator);
        m_SpawnAccumulator -= newParticles;


        for (int i = 0; i < newParticles; ++i) {
            int idx = firstUnusedParticle();
            respawnParticle(m_Particles[idx], emitterPos, rotationMatrix);
        }

    }

    if (Emitting && OneShot)
    {
        int count = static_cast<int>(SpawnRate); // can change to maxparticles to emit with full force
        for (int i = 0; i < count; ++i) {
            int idx = firstUnusedParticle();
            respawnParticle(m_Particles[idx], emitterPos, rotationMatrix);
        }

        // stop emitting, coz its oneshot duh
        Emitting = false;
    }
    

    // update existing particles

    for (unsigned int i = 0; i < MaxParticles; ++i)
    {
        Particle &p = m_Particles[i];
        if (p.LifeTime > 0.0f)
        {
            p.LifeTime -= deltaTime;
            // if still alive
            if (p.LifeTime > 0.0f)
            {
                p.Velocity += Gravity * deltaTime;
                p.Position += p.Velocity * deltaTime;

                float lifePercent = 1.0f - (p.LifeTime / p.InitialLifeTime);
                // Scale over time
                if (ScaleOverTime && lifePercent >= BeginScaling)
                {
                    float t = (lifePercent - BeginScaling) / (1.0f - BeginScaling);
                    p.Size = glm::mix(p.Size, 0.0f, t); // Scale to zero size
                }

                // Fade over time
                if (FadeOverTime && lifePercent >= BeginFading)
                {
                    float t = (lifePercent - BeginFading) / (1.0f - BeginFading);
                    p.Color.a = glm::mix(p.Color.a, 0.0f, t); // Fade to transparent
                }
            } 
            else
            {
                // is dead - make it transparent
                p.Color.a = 0.0f;
            }
        }
    }

}

void ParticleSystem::Render() 
{

    std::vector<std::pair<float, int>> sorted;
    sorted.reserve(MaxParticles); 

    for (int i = 0; i < MaxParticles; ++i) {
        Particle &p = m_Particles[i];
        if (p.LifeTime > 0.0f) {
            float dist = glm::length(p.Position - CameraPosition);
            sorted.push_back({dist, i});
        }
    }

    GLsizei liveCount = static_cast<GLsizei>(sorted.size());

    if (DrawEmissionShape)
        drawDebug();

    if (liveCount == 0)
        return;

    std::sort(sorted.begin(), sorted.end(), [](auto &a, auto &b) { return a.first > b.first; });

    // temp array for living particles
    std::vector<ParticleInstanceData> instances;
    instances.reserve(liveCount);

    for (unsigned int i = 0; i < liveCount; ++i)
    {
        Particle &p = m_Particles[sorted[i].second];
        ParticleInstanceData instance;
        if (UseLocalSpace) {
            instance.Offset = gameObject->transform.GetGlobalPosition() + p.Position;
        } else {
            instance.Offset = p.Position;
        }
        instance.Size = p.Size;
        instance.Color = p.Color;
        instances.push_back(instance);
        
    }

    

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, BlendingAdditive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);


    // send data to gpu
    glBindBuffer(GL_ARRAY_BUFFER, m_InstanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, liveCount * sizeof(ParticleInstanceData), instances.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (m_Texture && m_Texture->id != 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_Texture->id);
        m_Shader->Use();
        m_Shader->SetInt("particleTexture", 0);
        m_Shader->SetFloat("emission", EmissionMultiplier);
        m_Shader->SetBool("u_BillboardY", BillboardY);

        // draw quads
        glBindVertexArray(m_VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, liveCount);
    }

    // unbind
    glBindVertexArray(0);

    // unbind shader
    glUseProgram(0);

    glDisable(GL_BLEND);
    
}

void ParticleSystem::SetMaxParticles(int maxParticles) 
{ 
    MaxParticles = maxParticles; 
    cleanup();
    m_Particles.clear();
    m_Particles.resize(MaxParticles);
}

void ParticleSystem::SetSpawnRate(float spawnRate) { SpawnRate = spawnRate; }

void ParticleSystem::SetTexture(std::shared_ptr<Texture> texture) 
{ 
    //glDeleteTextures(1, &m_Texture);

    m_Texture = texture; 
}

void ParticleSystem::SetShader(std::shared_ptr<Shader> shaderProgram) { m_Shader = shaderProgram; }

void ParticleSystem::Init() { 
	initParticles();
    initBuffers();
    m_Initialized = true;
}

bool ParticleSystem::IsInitialized() { return m_Initialized; }

void ParticleSystem::Emit() { Emitting = true; }

void ParticleSystem::Stop() { Emitting = false; }

bool ParticleSystem::IsEmitting() const { return Emitting; }

std::shared_ptr<Texture> ParticleSystem::GetTexture() const { return m_Texture; }

void ParticleSystem::cleanup() 
{
    if (m_InstanceVBO)
        glDeleteBuffers(1, &m_InstanceVBO);

    if (m_QuadVBO)
        glDeleteBuffers(1, &m_QuadVBO);

    if (m_VAO)
        glDeleteVertexArrays(1, &m_VAO);

    m_Initialized = false;
}

void ParticleSystem::initParticles() {

	for (unsigned int i = 0; i < MaxParticles; i++) {
        m_Particles[i].LifeTime = 0.0f;
    }

}

void ParticleSystem::initBuffers() 
{
    cleanup();

    // gen VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // gen quad VBO
    glGenBuffers(1, &m_QuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);

    float quadVertices[] = {
            // Positions         // UVs
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 
            0.5f,  0.5f, 0.0f, 1.0f, 1.0f,

            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 
            0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 
            -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);

    // aTexCoords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

    // gen instance VBO
    glGenBuffers(1, &m_InstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_InstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * sizeof(ParticleInstanceData), nullptr, GL_STREAM_DRAW);

    // instance offset
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void *) offsetof(ParticleInstanceData, Offset));
    glVertexAttribDivisor(2, 1);

    // instance size
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void *) offsetof(ParticleInstanceData, Size));
    glVertexAttribDivisor(3, 1);

    // instance color
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void *) offsetof(ParticleInstanceData, Color));
    glVertexAttribDivisor(4, 1);

    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

int ParticleSystem::firstUnusedParticle() 
{ 

    for (int i = m_LastUsedParticle; i < MaxParticles; ++i) {
        if (m_Particles[i].LifeTime <= 0.0f) {
            m_LastUsedParticle = i;
            return i;
        }
    }
    for (int i = 0; i < m_LastUsedParticle; ++i) {
        if (m_Particles[i].LifeTime <= 0.0f) {
            m_LastUsedParticle = i;
            return i;
        }
    }
    m_LastUsedParticle = 0;
    return 0; 
}

void ParticleSystem::respawnParticle(Particle &particle, const glm::vec3 &emitterPos, const glm::mat4& rotationMatrix)
{
    glm::vec3 offset;

    switch (EmissionShape)
    {
        case EmissionShapeType::Point:
            offset = glm::vec3(0.0f);
            break;

        case EmissionShapeType::Cube:
            offset = glm::vec3(
                Random::Float(-CubeSize.x / 2, CubeSize.x / 2),
                Random::Float(-CubeSize.y / 2, CubeSize.y / 2),
                Random::Float(-CubeSize.z / 2, CubeSize.z / 2)
            );
            break;
        case EmissionShapeType::Sphere:
            glm::vec3 dir = Random::InUnitSphere();
            if (SphereSurfaceOnly) 
            {
                dir = glm::normalize(dir);
            }
            offset = dir * SphereRadius;
            break;
    }

    glm::vec3 rotatedOffset = glm::vec3(rotationMatrix * glm::vec4(offset, 0.0f));

    if (UseLocalSpace)
        particle.Position =  rotatedOffset;
    else
        particle.Position = emitterPos + rotatedOffset;

    particle.LifeTime = Random::Float(MinLifeTime, MaxLifeTime);
    float vx = Random::Float(MinInitialVelocity.x, MaxInitialVelocity.x);
    float vy = Random::Float(MinInitialVelocity.y, MaxInitialVelocity.y);
    float vz = Random::Float(MinInitialVelocity.z, MaxInitialVelocity.z);
    particle.Velocity = glm::vec3(vx, vy, vz);
    particle.Color = Albedo;

    particle.Size = Random::Float(MinSize, MaxSize);

    particle.InitialLifeTime = particle.LifeTime;

}

void ParticleSystem::correctRanges() 
{
    // lifetime check
    if (MinLifeTime > MaxLifeTime) 
    {
        MaxLifeTime = MinLifeTime;
    }

    if (MaxLifeTime < MinLifeTime)
    {
        MinLifeTime = MaxLifeTime;
    }

    // size check
    if (MinSize > MaxSize) 
    {
        MaxSize = MinSize;
    }

    if (MaxSize < MinSize) 
    {
        MinSize = MaxSize;
    }
}

void ParticleSystem::drawDebug() 
{
    glm::vec3 pos = gameObject->transform.GetGlobalPosition();

    switch (EmissionShape)
    { 
        case EmissionShapeType::Point:
            DebugDraw::GetInstance().Line(pos - glm::vec3(0.1f), pos + glm::vec3(0.1f), glm::vec4(1, 1, 0, 1));
            break;

        case EmissionShapeType::Cube:
            glm::mat4 parentRot = glm::mat4(1);
            if (gameObject->parent)
                parentRot = glm::mat4_cast(gameObject->parent->transform.GetQuatRotation());

            glm::mat4 rot = parentRot * glm::mat4_cast(gameObject->transform.GetQuatRotation());
            DebugDraw::GetInstance().Cube(pos, CubeSize, rot, glm::vec4(0, 1, 0, 1));
            break;

        case EmissionShapeType::Sphere:
            DebugDraw::GetInstance().Sphere(pos, SphereRadius, glm::vec4(0, 0.7f, 1, 1));
            break;
    }

}

