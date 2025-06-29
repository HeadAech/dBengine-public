//
// Created by Hubert Klonowski on 17/03/2025.
//

#include "dBrender.h"

#include <iostream>
#include <ostream>
#include <glad/glad.h>
#include <glm/matrix.hpp>

#include "GameObject/GameObject.h"

#include <GLFW/glfw3.h>
#include "Components/MeshInstance/MeshInstance.h"
#include "dBengine/EngineDebug/EngineDebug.h"

#include <Gizmo/Gizmo.h>
#include <Components/Lights/SpotLight/SpotLight.h>
#include <Components/Lights/PointLight/PointLight.h>
#include <Components/Lights/DirectionalLight/DirectionalLight.h>
#include "Helpers/Util.h"
#include "ShadowMapFramebuffer/ShadowCubeMapBuffer/ShadowCubeMapBuffer.h"
#include <glm/gtc/type_ptr.hpp>
#include <Signal/Signals.h>
#include "Components/CollisionShape/CollisionShape.h"
#include <Components/Animator/Animator.h>
#include <Components/Particles/ParticleSystem.h>
#include "Helpers/DebugDraw.h"
#include <Helpers/TimerHelper/TimerHelper.h>
#include <Components/Tag/Tag.h>
#include <dBphysics/dBphysics.h>
#include <Components/Control/Button/Button.h>
#include <Singletons/Ref/Ref.h>
#include <Components/AISystem/AISystem.h>
#include <Components/Control/Text/Text.h>
#include <Components/Control/Sprite/Sprite.h>
#include "PostProcessing/Shake/CameraShake.h"
#include "PostProcessing/Fade/FadeTransition.h"


dBrender::dBrender() { 
    this->uuid = UUID::generateUUID(); 
    Signals::Render_ResetFlatObjects.connect(this->uuid, [this]() { preFlattenHierarchy(); });
    Signals::Render_RemoveGameObject.connect(this->uuid, [this](GameObject* gameObject) { removeObjectFromFlat(gameObject); });

    Signals::ForceRenderLights.connect(this->uuid, [this]()
        {
            m_ForceRenderLights = true;
        });

    Signals::PostProcessing_ApplyProperties.connect(this->uuid, [this]()
        {
            auto& settings = EngineSettings::GetEngineSettings();
            if (hdrShader) {
                hdrShader->Use();
                hdrShader->SetFloat("u_VignetteStrength", settings.vignetteStrength);
                hdrShader->SetBool("u_EnableGlitchEffect", settings.enableGlitchEffect);
                hdrShader->SetFloat("glitchIntensity", settings.glitchEffectIntensity);
                hdrShader->SetFloat("glitchFrequency", settings.glitchEffectFrequency);
            }

            if (m_HUDShader)
            {
                m_HUDShader->Use();
                m_HUDShader->SetFloat("u_FisheyeStrength", settings.fisheyeStrength);
                m_HUDShader->SetFloat("u_ChromaticStrength", settings.chromaticStrength);
                m_HUDShader->SetFloat("u_ShadowRadius", settings.shadowRadius);
                m_HUDShader->SetVec3("u_ShadowColor", settings.shadowColor);
                m_HUDShader->SetFloat("u_ShadowIntensity", settings.shadowIntensity);
                m_HUDShader->SetVec2("u_ShadowOffset", settings.shadowOffset);
                m_HUDShader->SetBool("u_DistortionEnabled", settings.distortionEnabled);
                m_HUDShader->SetFloat("u_DisplacementStrength", settings.displacementStrength);
                m_HUDShader->SetFloat("u_DisplacementSpeed", settings.displacementSpeed);
                m_HUDShader->SetFloat("u_ScanlineHeight", settings.scanlineHeight);
                m_HUDShader->SetFloat("u_ScanlineProbability", settings.scanlineProbability);
            }
        });
}

dBrender &dBrender::GetInstance() {
    static dBrender instance;
    return instance;
}

Shader *dBrender::GetShader() { 
    return this->shader; 
}

void dBrender::SetShader(Shader *shader) {
    this->shader = shader;
}



void dBrender::RenderCollisionShapes(Camera* camera) {
    if (wireframeShader) {
        wireframeShader->Use();
        wireframeShader->SetMat4("view", camera->GetViewMatrix());
        wireframeShader->SetMat4("projection", camera->GetProjectionMatrix());
        CollisionShape::RenderColliders(wireframeShader);
        if (dBphysics::GetInstance().IsOctreeVisible()) {
            dBphysics::GetInstance().RenderOctree(wireframeShader);
        }
    }
}

void dBrender::setupShaders() { 
    // TODO: do it, im too tired for this shit
    debugDepthShader->Use();
    debugDepthShader->SetInt("depthMap", 0);

    hdrShader->Use();
    hdrShader->SetInt("hdrBuffer", 0);
    hdrShader->SetFloat("exposure", 1.0f);
    hdrShader->SetInt("bloomBlur", 1);

    shader->Use();
    shader->SetInt("material.diffuse", 0);
    shader->SetInt("material.normal", 1);
    shader->SetInt("material.metallic", 2);
    shader->SetInt("material.roughness", 3);
    shader->SetInt("material.ambientOcclusion", 4);
    shader->SetInt("material.specular", 5);
    shader->SetInt("material.height", 6);
    shader->SetInt("material.emissive", 7);
    shader->SetInt("irradianceMap", 8);


    glBindBuffer(GL_UNIFORM_BUFFER, lightSpaceUBO);
    GLuint uniformBlockIndex = glGetUniformBlockIndex(shader->ID, "LightSpaceData");
    glUniformBlockBinding(shader->ID, uniformBlockIndex, 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightSpaceUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    m_ParticlesShader->Use();
    m_ParticlesShader->SetInt("particleTexture", 0);

    DebugDraw::GetInstance().SetShader(m_DebugDrawShader);

    m_ScreenShader->Use();
    m_ScreenShader->SetInt("screenTexture", 0);

    m_GlitchShader->Use();
    m_GlitchShader->SetInt("screenTexture", 0);
}



void dBrender::CreateShaderPrograms() {
    TimerHelper timer("dBrender::CreateShaderPrograms");

       // for world environment
    m_hdriProcessshader = std::make_unique<Shader>("res/shaders/skybox/hdri.vert", "res/shaders/skybox/hdri.frag");
    m_physicalSkyShader = std::make_unique<Shader>("res/shaders/skybox/physical/sky_physical.vert",
                                                   "res/shaders/skybox/physical/sky_physical.frag");
    m_hdriSkyboxShader = std::make_unique<Shader>("res/shaders/skybox/shader.vert", "res/shaders/skybox/shader.frag");
    m_volumetricShader = std::make_unique<Shader>("res/shaders/skybox/clouds/vol_clouds.vert",
                                                  "res/shaders/skybox/clouds/vol_clouds.frag");
    m_irradianceConvoShader = std::make_unique<Shader>("res/shaders/skybox/hdri.vert","res/shaders/skybox/irradiance_convo.frag");

    wireframeShader = new Shader("res/shaders/debug/wireframe.vert", "res/shaders/debug/wireframe.frag");
    m_TextShader = std::make_shared<Shader>("res/shaders/text/shader.vert", "res/shaders/text/shader.frag");
    hdrShader = std::make_unique<Shader>("res/shaders/hdr/shader.vert", "res/shaders/hdr/shader.frag");
    depthShader = std::make_unique<Shader>("res/shaders/depth/shader.vert", "res/shaders/depth/shader.frag");
    depthCubeMapShader =
            std::make_unique<Shader>("res/shaders/depth/cubemap/shader.vert", "res/shaders/depth/cubemap/shader.frag",
                                     "res/shaders/depth/cubemap/shader.geom");
    debugDepthShader =
            std::make_unique<Shader>("res/shaders/depth/debug/shader.vert", "res/shaders/depth/debug/shader.frag");
    debugDepthCubeMapShader = std::make_unique<Shader>("res/shaders/depth/debug/cubemap/shader.vert",
                                                       "res/shaders/depth/debug/cubemap/shader.frag");
    refractShader = std::make_unique<Shader>("res/shaders/refract/shader.vert", "res/shaders/refract/shader.frag");
    reflectShader = std::make_unique<Shader>("res/shaders/reflect/shader.vert", "res/shaders/reflect/shader.frag");
    volumetricShader =
            std::make_unique<Shader>("res/shaders/volumetric/shader.vert", "res/shaders/volumetric/shader.frag");


    m_LightShader = std::make_unique<Shader>("res/shaders/light/light.vert", "res/shaders/light/light.frag");

    m_AnimShader = std::make_unique<Shader>("res/shaders/animation/anim.vert", "res/shaders/animation/anim.frag");

    m_ParticlesShader = std::make_shared<Shader>("res/shaders/particles/particles.vert", "res/shaders/particles/particles.frag");
    m_DebugDrawShader =
            std::make_shared<Shader>("res/shaders/debug_draw/debug_draw.vert", "res/shaders/debug_draw/debug_draw.frag");

    m_ButtonShader = std::make_shared<Shader>("res/shaders/button/button.vert", "res/shaders/button/button.frag");

    //runtime
    m_ScreenShader = std::make_unique<Shader>("res/shaders/screen/screen.vert", "res/shaders/screen/screen.frag");

    // glitch effect
    m_GlitchShader = std::make_unique<Shader>("res/shaders/glitch/glitch.vert", "res/shaders/glitch/glitch.frag");

    // sprite 
    m_SpriteShader = std::make_unique<Shader>("res/shaders/sprite/sprite.vert", "res/shaders/sprite/sprite.frag");

    // hud shader
    m_HUDShader = std::make_unique<Shader>("res/shaders/hud/hud.vert", "res/shaders/hud/hud.frag");

    //transition
    m_TransitionShader = std::make_unique<Shader>("res/shaders/transition/transition.vert",
                                                  "res/shaders/transition/transition.frag");
}

void dBrender::Init() {

    Signals::ShaderReloaded.connect(this->uuid, [this](unsigned int id) { 
        setupShaders();
        m_ForceRenderLights = true;
    });

   
    setupShaders();

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    framebuffer.Init(width, height);
    renderFrameBuffer.Init(width, height);
    shadowMapBuffer.Init();
    framebufferMultisample.m_width = width;
    framebufferMultisample.m_height = height;
    framebufferMultisample.Init();
    bloomRenderer.Init(width, height);
    UIFramebuffer.Init(width, height);
    HUDBloomRenderer.Init(width, height);

   
    TimerHelper bufferCreationTimer("dBrender::Init");

    //prepare light UBO
    GLuint uniformBlockIndex = glGetUniformBlockIndex(shader->ID, "LightSpaceData");
    glUniformBlockBinding(shader->ID, uniformBlockIndex, 1);

    glGenBuffers(1, &lightSpaceUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, lightSpaceUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 32 + sizeof(glm::vec4) * 32,
                 NULL, GL_DYNAMIC_DRAW);
    
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, lightSpaceUBO, 0,
                      sizeof(glm::mat4) * 32 + sizeof(glm::vec4) * 32);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightSpaceUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &m_BonesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_BonesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 100, nullptr, GL_DYNAMIC_DRAW);
    
    GLuint blockIndex = glGetUniformBlockIndex(shader->ID, "Bones");
    glUniformBlockBinding(shader->ID, blockIndex, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_BonesUBO);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //big boi - dirLight and spotlights
    shadowMapArrayBuffer.Init();

    // big boi - pointlights
    shadowCubeMapArrayBuffer.Init();


}



void dBrender::renderFramebufferToQuad() {
    static unsigned int quadVAO = 0;
    static unsigned int quadVBO;
    if (quadVAO == 0) {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void dBrender::Render(Camera* camera) {
    if (auto worldEnv = m_activeScene->GetWorldEnvironment()) {
        if (!worldEnv->IsHDRILoaded() && worldEnv->GetSkyType() == 0) {
            worldEnv->LoadHDRI();
        }
    }

    camera->gameObject->transform.ComputeModelMatrix();
    m_cameraFrustum =
            Frustum::CreateFrustumFromCamera(camera->gameObject->transform.GetGlobalPosition(), camera->Front, camera->Right, camera->Up, camera->aspectRatio,
                                             glm::radians(camera->Zoom), camera->NearPlane, camera->FarPlane);


    m_activeCamera = camera;


    EngineDebug::GetInstance().drawCalls = 0;
    int instancesCount = m_activeScene->sceneRootObject->children.size();
    EngineDebug::GetInstance().instancesCount = instancesCount;

    glCullFace(GL_FRONT);
    generateShadowMaps();
    glCullFace(GL_BACK);
    

    glViewport(0, 0, framebuffer.m_width, framebuffer.m_height);

    shader->Use();

    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 proj = camera->GetProjectionMatrix();
    glm::vec3 viewPos = camera->gameObject->transform.GetGlobalPosition();

    cameraPosition = viewPos;

    shader->SetMat4("view", view);
    shader->SetMat4("projection", proj);
    shader->SetVec3("viewPos", viewPos);
    shader->SetFloat("u_FadeStart", EngineSettings::GetEngineSettings().shadowCutoffStart);
    if (m_activeScene && m_activeScene->GetWorldEnvironment() && m_activeScene->GetWorldEnvironment()->GetSkyType() == HDRI){
        shader->SetBool("useIrradiance", true);
    } else{
        shader->SetBool("useIrradiance", false);
    }

    m_LightShader->Use();
    m_LightShader->SetMat4("view", view);
    m_LightShader->SetMat4("projection", proj);

    m_ParticlesShader->Use();
    m_ParticlesShader->SetMat4("view", view);
    m_ParticlesShader->SetMat4("projection", proj);

    m_DebugDrawShader->Use();
    m_DebugDrawShader->SetMat4("view", view);
    m_DebugDrawShader->SetMat4("projection", proj);

    volumetricShader->Use();
    volumetricShader->SetMat4("view", view);
    volumetricShader->SetMat4("projection", proj);
    volumetricShader->SetVec3("cameraPos", viewPos);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    framebufferMultisample.Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Ref::ScreenResolution = { framebufferMultisample.m_width, framebufferMultisample.m_height };

    if (auto worldEnv = m_activeScene->GetWorldEnvironment()) {
        if (worldEnv->GetSkyType() == PHYSICAL) {
            if (auto dirLight = m_activeScene->GetDirectionalLight())
                worldEnv->RenderSkybox(camera, dirLight->GetComponent<DirectionalLight>());
        } else {
            worldEnv->RenderSkybox(camera, nullptr);
        }
    }
    
    glEnable(GL_DEPTH_TEST);
    

    renderLights();
    shader->Use();


    glBindBuffer(GL_UNIFORM_BUFFER, lightSpaceUBO);
    if (lightSpaceMatrices.size() > 0)
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * 32, lightSpaceMatrices.data());
    if (lightPositions.size() > 0)
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 32, sizeof(glm::vec4) * 32, lightPositions.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0); 

    // int lastIdx = 8;
    if (auto worldEnv = m_activeScene->GetWorldEnvironment()) {
        if (worldEnv->GetSkyType() == 0) {
            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_CUBE_MAP, worldEnv->m_irradianceMap);
        }
    }
    // 0-7 materials, 8 = ibl [nie bijcie, nigdzie nie jest to sprawdzane, jest ok]

    int lastIdx = 9;

    shader->SetFloat("farPlane", 150.0f);

    glActiveTexture(GL_TEXTURE0 + lastIdx);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMapArrayBuffer.GetDepthMap());
    shader->SetInt("shadowMapArray", lastIdx);
    lastIdx++;

    glActiveTexture(GL_TEXTURE0 + lastIdx);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, shadowCubeMapArrayBuffer.GetDepthMap());
    shader->SetInt("shadowCubeMapArray", lastIdx);

    // Array containing all the 2D UI elements
    std::vector<UI::Control*> controls;

    glEnable(GL_DEPTH_TEST);
    renderScene(shader, false, controls);

    framebufferMultisample.Unbind();

    UIFramebuffer.Bind();
    glViewport(0, 0, UIFramebuffer.m_width, UIFramebuffer.m_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Optional: transparent background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render all UI controls as you already do
    
    for (auto& control : controls)
    {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (auto button = dynamic_cast<UI::Button*>(control))
        {
            if (!button->GetShader())
            {
                button->SetShader(m_ButtonShader);
            }
            button->Render();
        }
        else if (auto text = dynamic_cast<UI::Text*>(control))
        {
            if (!text->GetShader())
            {
                text->SetShader(m_TextShader);
            }
            text->Render();
        }
        else if (auto sprite = dynamic_cast<UI::Sprite*>(control))
        {
            if (!sprite->GetShader())
            {
                sprite->SetShader(m_SpriteShader);
            }
            sprite->Render();
        }
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
    
    UIFramebuffer.Unbind();
    
    //framebufferMultisample.Unbind();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferMultisample.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, framebufferMultisample.m_width, framebufferMultisample.m_height, 0, 0, framebuffer.m_width,
                      framebuffer.m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glBlitFramebuffer(0, 0, framebufferMultisample.m_width, framebufferMultisample.m_height, 0, 0, framebuffer.m_width,
                      framebuffer.m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    bloomRenderer.RenderBloomTexture(framebuffer.colorBuffer, 0.005f);
    HUDBloomRenderer.RenderBloomTexture(UIFramebuffer.texture, 0.005f);

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //final framebuffer rendering
    {
        renderFrameBuffer.Bind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Apply glitch effect if enabled, otherwise use HDR shader
        if (EngineSettings::GetEngineSettings().enableGlitchShader) {
            renderGlitchEffect();
        } else {
            hdrShader->Use();
            hdrShader->SetInt("hdrBuffer", 0);
            hdrShader->SetInt("bloomBlur", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, framebuffer.getFrameTexture());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, bloomRenderer.BloomTexture());

            // camera shake
            auto& cameraShake = CameraShake::GetInstance();
            if (cameraShake.Active)
            {
                hdrShader->SetFloat("time", cameraShake.GetTime());
                hdrShader->SetFloat("shakeAmount", cameraShake.GetCurrentShake());
            }

            hdrShader->SetFloat("u_Time", glfwGetTime());

           

            // render to hdr
            renderFramebufferToQuad();
        }


        m_HUDShader->Use();
        m_HUDShader->SetInt("u_HUDTexture", 0);
        m_HUDShader->SetInt("u_BloomBlur", 1);
        m_HUDShader->SetFloat("u_Time", glfwGetTime());
        
       /* m_HUDShader->SetFloat("u_FisheyeStrength", 0.5f);
        m_HUDShader->SetFloat("u_ChromaticOffset", 0.005f);
   */     m_HUDShader->SetVec2("u_Resolution", glm::vec2(UIFramebuffer.m_width, UIFramebuffer.m_height));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, UIFramebuffer.getFrameTexture());

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, HUDBloomRenderer.BloomTexture());

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        renderFramebufferToQuad();
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // transition
        auto& fadeTransition = FadeTransition::GetInstance();
        if (fadeTransition.IsActive())
        {

            m_TransitionShader->Use();
            m_TransitionShader->SetFloat("u_Fade", fadeTransition.fade);
            renderFramebufferToQuad();
        }
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        

        if (EngineSettings::GetEngineSettings().m_showDirectionalLightDepthMap) {

            glDisable(GL_DEPTH_TEST);
            debugDepthShader->Use();
            debugDepthShader->SetInt("depthMap", 0);
            debugDepthShader->SetInt("depthMapArray", 1);
            debugDepthShader->SetInt("testArray", 1);
            debugDepthShader->SetInt("debugLayer", 0);

            /*debugDepthCubeMapShader->Use();
            debugDepthCubeMapShader->SetInt("cubemap", 0);
            debugDepthCubeMapShader->SetInt("faceIndex", 2);
            debugDepthCubeMapShader->SetInt("layer", 0);*/
            glActiveTexture(GL_TEXTURE1);

            //glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, shadowCubeMapArrayBuffer.GetDepthMap());
            //glBindTexture(GL_TEXTURE_2D, bloomRenderer.BloomTexture());
            //shadowMapArrayBuffer.SetLayer(0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMapArrayBuffer.GetDepthMap());

            renderFramebufferToQuad();
        }

        RenderCollisionShapes(camera);
        RenderNavigation(camera);

        

        renderFrameBuffer.Unbind();
    }
    
    if (!EngineSettings::GetEngineSettings().EditorEnabled)
    {
        //TODO what is happening
        glViewport(0, 0, Ref::WindowSize.x, Ref::WindowSize.y);
        m_ScreenShader->Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderFrameBuffer.getFrameTexture());
        renderFramebufferToQuad();
    }
}

void dBrender::renderScene(Shader* shader, bool shadowPass, std::vector<UI::Control*>& controls, const glm::vec3& lightPos, float shadowDistance) {
    controls.clear();

    for (int j = 0; j < m_flatGameObjects.size(); j++) {


        if (!m_flatGameObjects.at(j)->m_enabled) {
            continue;
        }

        auto gameObject = m_flatGameObjects.at(j);
        auto meshInstance = m_flatGameObjects.at(j)->GetComponent<MeshInstance>();

        if (meshInstance && meshInstance->model) {
            if (!meshInstance->model->Ready) {
                continue;
            }
            if (meshInstance->enabled) {
                if (!shadowPass) {
                    if (!meshInstance->model->pAABB->IsOnFrustum(m_cameraFrustum, m_flatGameObjects.at(j)->transform) ||
                        !m_flatGameObjects.at(j)->m_enabled) {

                        continue;
                    }
                }
                else
                {
                    if (Util::GetDistance(lightPos, gameObject->transform.GetGlobalPosition()) > shadowDistance)
                    {
                        continue;
                    }
                }


                Material *material = meshInstance->MaterialOverride.get();
                if (material == nullptr) {
                    material = &meshInstance->model->Material;
                }

                if (shader->ID == this->shader->ID) {
                    material->Bind(shader);
                }

                // rendering light
                if (auto light = m_flatGameObjects.at(j)->GetComponent<PointLight>()) {
                    m_LightShader->Use();
                    m_LightShader->SetVec3("lightColor", light->GetDiffuse());
                    m_LightShader->SetFloat("intensity", light->GetIntensity());
                }

                if (auto anim = m_flatGameObjects.at(j)->GetComponent<Animator>()) {
                    shader->SetBool("isAnimated", true);
                    auto transforms = anim->GetFinalBoneMatrices();
                    glBindBuffer(GL_UNIFORM_BUFFER, m_BonesUBO);
                    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * transforms.size(), transforms.data());
                    glBindBuffer(GL_UNIFORM_BUFFER, 0);
                } else {
                    shader->SetBool("isAnimated", false);
                }

                

                glm::mat4 M = m_flatGameObjects.at(j)->transform.GetModelMatrix();

                if (meshInstance->isUsingVolumetric())
                {
                    renderVolumetricObjects(gameObject);
                    continue;
                }


                if (shadowPass) {
                    glCullFace(GL_FRONT);
                    if (auto tag = m_flatGameObjects.at(j)->GetComponent<Tag>()) {
                        if (tag->Name == "Floor") {
                            glCullFace(GL_BACK);
                        }
                    }
                }
                draw(*meshInstance, M);
                glCullFace(GL_BACK);

                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_CULL_FACE);
            }
        }

        if (!shadowPass) {
            if (auto particleSystem = m_flatGameObjects.at(j)->GetComponent<ParticleSystem>()) {
                particleSystem->CameraPosition = cameraPosition;
                particleSystem->Render();
            }

            // doesnt require mesh
            if (auto button = m_flatGameObjects.at(j)->GetComponent<UI::Button>())
            {
                controls.push_back((UI::Control*) button);
            }

            if (auto text = m_flatGameObjects.at(j)->GetComponent<UI::Text>()) {
                controls.push_back((UI::Control *) text);
            }

            if (auto sprite = m_flatGameObjects.at(j)->GetComponent<UI::Sprite>())
            {
                controls.push_back((UI::Control*)sprite);
            }
        }
    }
}

void dBrender::drawInstanced(MeshInstance &mesh, int instanceCount) {
    for (unsigned int j = 0; j < mesh.model->Meshes.size(); j++) {
        glBindVertexArray(mesh.model->Meshes[j].VAO);
        glEnableVertexAttribArray(7);
        glEnableVertexAttribArray(8);
        glEnableVertexAttribArray(9);
        glEnableVertexAttribArray(10);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(mesh.model->Meshes[j].indices.size()),
                                GL_UNSIGNED_INT,
                                0, instanceCount);
        glBindVertexArray(0);
        EngineDebug::GetInstance().drawCalls++;
    }
}

void dBrender::draw(MeshInstance &mesh, glm::mat4& model) {
    for (unsigned int i = 0; i < mesh.model->Meshes.size(); i++) {
        glBindVertexArray(mesh.model->Meshes[i].VAO);
        glDisableVertexAttribArray(7);
        glDisableVertexAttribArray(8);
        glDisableVertexAttribArray(9);
        glDisableVertexAttribArray(10);
        glVertexAttrib4fv(7, glm::value_ptr(model[0]));
        glVertexAttrib4fv(8, glm::value_ptr(model[1]));
        glVertexAttrib4fv(9, glm::value_ptr(model[2]));
        glVertexAttrib4fv(10, glm::value_ptr(model[3]));
        
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.model->Meshes[i].indices.size()), GL_UNSIGNED_INT,
                       0);
        glBindVertexArray(0);
        EngineDebug::GetInstance().drawCalls++;
    }
}

void dBrender::renderLights() {
    GameObject *directionalLightGameObject = m_activeScene->GetDirectionalLight();
    const std::vector<GameObject *> &pointLightsGameObjects = m_activeScene->GetPointLights();
    const std::vector<GameObject *> &spotLightsGameObjects = m_activeScene->GetSpotLights();

    shader->Use();
    shader->SetInt("numPointLights", pointLightsGameObjects.size());
    shader->SetInt("numSpotLights", spotLightsGameObjects.size());
    shader->SetFloat("material.shininess", 32.0f);

    if (directionalLightGameObject) {

        DirectionalLight *dirLight = directionalLightGameObject->GetComponent<DirectionalLight>();
        if (dirLight != nullptr) {
            renderDirectionalLight(directionalLightGameObject, dirLight);
        }
    }

    for (int i = 0; i < pointLightsGameObjects.size(); i++) {
        GameObject *pointLightGO = pointLightsGameObjects.at(i);

        PointLight *pointLight = pointLightGO->GetComponent<PointLight>();
        if (pointLight != nullptr) {
            renderPointLight(pointLightGO, pointLight, i);
        }
    }

    for (int i = 0; i < spotLightsGameObjects.size(); i++) {
        GameObject *spotLightGO = spotLightsGameObjects.at(i);
        SpotLight *spotLight = spotLightGO->GetComponent<SpotLight>();

        if (spotLight != nullptr) {
            renderSpotLight(spotLightGO, spotLight, i);
        }
    }

    if (m_ForceRenderLights)
    {
        m_ForceRenderLights = false;
    }
}

void dBrender::renderDirectionalLight(GameObject* pDirLightObj, DirectionalLight* pDirLight)
{
    shader->SetBool("dirLight.isOn", pDirLightObj->m_enabled && pDirLight->enabled);

    PropertyWatch& propertyWatch = pDirLight->propertyWatch;
    bool isDirty = propertyWatch.IsDirty();

    if (!isDirty && !m_ForceRenderLights)
    {
        return;
    }

    if (propertyWatch.Direction || m_ForceRenderLights)
    {
        shader->SetVec3("dirLight.direction", pDirLight->GetDirection());
        propertyWatch.Direction = false;
    }

    if (propertyWatch.Ambient || m_ForceRenderLights)
    {
        shader->SetVec3("dirLight.ambient", pDirLight->GetAmbient());
        propertyWatch.Ambient = false;
    }

    if (propertyWatch.Diffuse || m_ForceRenderLights) {
        shader->SetVec3("dirLight.diffuse", pDirLight->GetDiffuse());
        propertyWatch.Diffuse = false;
    }

    if (propertyWatch.Specular || m_ForceRenderLights) {
        shader->SetVec3("dirLight.specular", pDirLight->GetSpecular());
        propertyWatch.Specular = false;
    }

    if (propertyWatch.Intensity || m_ForceRenderLights) {
        shader->SetFloat("dirLight.intensity", pDirLight->GetIntensity());
        propertyWatch.Intensity = false;
    }

    if (propertyWatch.AmbientIntensity || m_ForceRenderLights) {
        shader->SetFloat("dirLight.ambientIntensity", pDirLight->GetAmbientIntensity());
        propertyWatch.AmbientIntensity = false;
    }
}

void dBrender::renderPointLight(GameObject* pPointLightObj, PointLight* pPointLight, int i)
{
    std::string base = "pointLights[" + std::to_string(i) + "]";
    shader->SetBool(base + ".isOn", pPointLightObj->m_enabled && pPointLight->enabled);

    PropertyWatch& propertyWatch = pPointLight->propertyWatch;
    bool isDirty = propertyWatch.IsDirty();

    if (!isDirty && !m_ForceRenderLights)
    {
        return;
    }

    if (propertyWatch.Position || m_ForceRenderLights) 
    {
        shader->SetVec3(base + ".position", pPointLightObj->transform.GetGlobalPosition());
        propertyWatch.Position = false;
    }

    if (propertyWatch.Ambient || m_ForceRenderLights)
    {
        shader->SetVec3(base + ".ambient", pPointLight->GetAmbient());
        propertyWatch.Ambient = false;
    }

    if (propertyWatch.Diffuse || m_ForceRenderLights)
    {
        shader->SetVec3(base + ".diffuse", pPointLight->GetDiffuse());
        propertyWatch.Diffuse = false;
    }

    if (propertyWatch.Specular || m_ForceRenderLights)
    {
        shader->SetVec3(base + ".specular", pPointLight->GetSpecular());
        propertyWatch.Specular = false;
    }

    if (propertyWatch.Constant || m_ForceRenderLights)
    {
        shader->SetFloat(base + ".constant", pPointLight->GetConstant());
        propertyWatch.Constant = false;
    }
    
    if (propertyWatch.Linear || m_ForceRenderLights)
    {
        shader->SetFloat(base + ".linear", pPointLight->GetLinear());
        propertyWatch.Linear = false;
    }

    if (propertyWatch.Quadratic || m_ForceRenderLights)
    {
        shader->SetFloat(base + ".quadratic", pPointLight->GetQuadratic());
        propertyWatch.Quadratic = false;
    }

    if (propertyWatch.Intensity || m_ForceRenderLights)
    {
        shader->SetFloat(base + ".intensity", pPointLight->GetIntensity());
        propertyWatch.Intensity = false;
    }

    if (propertyWatch.AmbientIntensity || m_ForceRenderLights)
    {
        shader->SetFloat(base + ".ambientIntensity", pPointLight->GetAmbientIntensity());
        propertyWatch.AmbientIntensity = false;
    }

}

void dBrender::renderSpotLight(GameObject* pSpotLightObj, SpotLight* pSpotLight, int i)
{
    std::string base = "spotLights[" + std::to_string(i) + "]";

    shader->SetBool(base + ".isOn", pSpotLightObj->m_enabled && pSpotLight->enabled);


    PropertyWatch &propertyWatch = pSpotLight->propertyWatch;
    bool isDirty = propertyWatch.IsDirty();

    if (!isDirty && !m_ForceRenderLights) 
    {
        return;
    }

    if (propertyWatch.Position || m_ForceRenderLights)
    {
        shader->SetVec3(base + ".position", pSpotLightObj->transform.GetGlobalPosition());
        propertyWatch.Position = false;
    }

    if (propertyWatch.Ambient || m_ForceRenderLights)
    {
        shader->SetVec3(base + ".ambient", pSpotLight->GetAmbient());
        propertyWatch.Ambient = false;
    }

    if (propertyWatch.Diffuse || m_ForceRenderLights) {
        shader->SetVec3(base + ".diffuse", pSpotLight->GetDiffuse());
        propertyWatch.Diffuse = false;
    }

    if (propertyWatch.Specular || m_ForceRenderLights) {
        shader->SetVec3(base + ".specular", pSpotLight->GetSpecular());
        propertyWatch.Specular = false;
    }

    if (propertyWatch.Direction || m_ForceRenderLights) {
        shader->SetVec3(base + ".direction", pSpotLight->GetDirection());
        propertyWatch.Direction = false;
    }

    if (propertyWatch.Constant || m_ForceRenderLights) {
        shader->SetFloat(base + ".constant", pSpotLight->GetConstant());
        propertyWatch.Constant = false;
    }

    if (propertyWatch.Linear || m_ForceRenderLights) {
        shader->SetFloat(base + ".linear", pSpotLight->GetLinear());
        propertyWatch.Linear = false;
    }

    if (propertyWatch.Quadratic || m_ForceRenderLights)
    {
        shader->SetFloat(base + ".quadratic", pSpotLight->GetQuadratic());
        propertyWatch.Quadratic = false;
    }

    if (propertyWatch.Intensity || m_ForceRenderLights)
    {
        shader->SetFloat(base + ".intensity", pSpotLight->GetIntensity());
        propertyWatch.Intensity = false;
    }

    if (propertyWatch.AmbientIntensity || m_ForceRenderLights) {
        shader->SetFloat(base + ".ambientIntensity", pSpotLight->GetAmbientIntensity());
        propertyWatch.AmbientIntensity = false;
    }

    if (propertyWatch.InnerCutOff || m_ForceRenderLights)
    {
        shader->SetFloat(base + ".cutOff", pSpotLight->GetInnerCutOff());
        propertyWatch.InnerCutOff = false;
    }

    if (propertyWatch.OuterCutOff || m_ForceRenderLights)
    {
        shader->SetFloat(base + ".outerCutOff", pSpotLight->GetOuterCutOff());
        propertyWatch.OuterCutOff = false;
    }
    
}

void dBrender::generateShadowMaps() {
    GameObject *directionalLightGameObject = m_activeScene->GetDirectionalLight();
    const std::vector<GameObject *> &pointLightsGameObjects = m_activeScene->GetPointLights();
    const std::vector<GameObject *> &spotLightsGameObjects = m_activeScene->GetSpotLights();

    lightSpaceMatrices.clear();
    lightPositions.clear();
    // render scene from lights position
    float farPlane = 150.0f;
    int shadowMapIdx = 0;
    
    // dir light
    if (directionalLightGameObject) {


        glm::vec3 dirLightRot = directionalLightGameObject->transform.GetEulerRotation();
        // glm::vec3 lightDir = Util::GetDirectionFromEulerAngles(dirLightRot.x, dirLightRot.y, dirLightRot.z);
        glm::vec3 lightDir = Util::GetForwardDirection(directionalLightGameObject->transform.GetQuatRotation());
        glm::vec3 lightTarget = glm::vec3(0.0f);
        glm::vec3 lightPos = lightTarget - lightDir * 30.0f;

        glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.1f, farPlane);
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        lightPositions.push_back({lightPos.x, lightPos.y, lightPos.z, 1.0f});
        lightSpaceMatrices.push_back(lightSpaceMatrix);

        depthShader->Use();
        depthShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

        // depthShader->SetInt("layerIndex", shadowMapIdx);

        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, ShadowMapBuffer::SHADOW_WIDTH, ShadowMapBuffer::SHADOW_HEIGHT);

        shadowMapArrayBuffer.Bind();
        shadowMapArrayBuffer.SetLayer(shadowMapIdx);
        glClear(GL_DEPTH_BUFFER_BIT);

        // dummy vector
        std::vector<UI::Control*> controls;

        renderScene(depthShader.get(), true, controls);

        shadowMapArrayBuffer.Unbind();

        
    }
    shadowMapIdx++;
    // point lights
    glViewport(0, 0, ShadowCubeMapBuffer::SHADOW_WIDTH, ShadowCubeMapBuffer::SHADOW_HEIGHT);
    shadowCubeMapArrayBuffer.Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    shadowCubeMapArrayBuffer.Unbind();

    for (int i = 0; i < pointLightsGameObjects.size(); i++) {
        glm::vec3 lightPos = pointLightsGameObjects.at(i)->transform.GetGlobalPosition();
        if (pointLightsGameObjects.at(i) && !pointLightsGameObjects.at(i)->m_enabled || !pointLightsGameObjects.at(i)->GetComponent<PointLight>()->enabled) {
            continue;
        }
        
        // dont generate shadow map if not casting shadow.
        if (!pointLightsGameObjects.at(i)->GetComponent<PointLight>()->IsCastingShadows())
        {
            continue;
        }

        float shadowDistance = pointLightsGameObjects.at(i)->GetComponent<PointLight>()->GetShadowDistance();
        if (m_activeCamera)
        {
            if (Util::GetDistance(lightPos, m_activeCamera->gameObject->transform.GetGlobalPosition()) > shadowDistance)
            {
                continue;
            }
        }
        

        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, farPlane);
        glm::mat4 shadowTransforms[] = {
                shadowProj * 
                        glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                shadowProj *
                        glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                shadowProj *
                        glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                shadowProj *
                        glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                shadowProj *
                        glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                shadowProj *
                        glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),

        };
        depthCubeMapShader->Use();

        depthCubeMapShader->SetMat4("shadowMatrices[0]", shadowTransforms[0]);
        depthCubeMapShader->SetMat4("shadowMatrices[1]", shadowTransforms[1]);
        depthCubeMapShader->SetMat4("shadowMatrices[2]", shadowTransforms[2]);
        depthCubeMapShader->SetMat4("shadowMatrices[3]", shadowTransforms[3]);
        depthCubeMapShader->SetMat4("shadowMatrices[4]", shadowTransforms[4]);
        depthCubeMapShader->SetMat4("shadowMatrices[5]", shadowTransforms[5]);

        depthCubeMapShader->SetVec3("lightPos", lightPos);
        depthCubeMapShader->SetFloat("farPlane", farPlane);
        depthCubeMapShader->SetInt("lightIndex", i);

        depthCubeMapShader->SetVec3("u_CamPos", cameraPosition);
        depthCubeMapShader->SetFloat("u_FadeStart", shadowDistance * 0.9f);
        depthCubeMapShader->SetFloat("u_FadeEnd", shadowDistance * 0.99f);

        
        shadowCubeMapArrayBuffer.Bind();
        
        //glClear(GL_DEPTH_BUFFER_BIT);
        // dummy vector
        std::vector<UI::Control*> controls;
        renderScene(depthCubeMapShader.get(), true, controls, lightPos, shadowDistance);

        shadowCubeMapArrayBuffer.Unbind();

    }

    shadowMapArrayBuffer.Bind();
    // spot lights
    glViewport(0, 0, ShadowMapBuffer::SHADOW_WIDTH, ShadowMapBuffer::SHADOW_HEIGHT);
    for (int i = 0; i < spotLightsGameObjects.size(); i++) {

        if (spotLightsGameObjects.at(i) && !spotLightsGameObjects.at(i)->m_enabled ||
            !spotLightsGameObjects.at(i)->GetComponent<SpotLight>()->enabled) {
            continue;
        }

        // if casting disabled - dont generate the shadow map
        if (!spotLightsGameObjects.at(i)->GetComponent<SpotLight>()->IsCastingShadows())
        {
            continue;
        }

        glm::mat4 perspective = glm::perspective(glm::radians(90.0f), 1.0f, 2.5f, farPlane);
        glm::vec3 spotRot = spotLightsGameObjects.at(i)->transform.GetEulerRotation();
        //lightDir = Util::GetDirectionFromEulerAngles(spotRot.x, spotRot.y, spotRot.z);
        glm::vec3 lightDir = Util::GetForwardDirection(spotLightsGameObjects.at(i)->transform.GetQuatRotation());
        glm::vec3 lightPos = spotLightsGameObjects.at(i)->transform.GetGlobalPosition();
        glm::vec3 lightTarget = lightPos + lightDir;

        glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = perspective * lightView;

        lightPositions.push_back({lightPos.x, lightPos.y, lightPos.z, 1.0f});
        lightSpaceMatrices.push_back(lightSpaceMatrix);

        depthShader->Use();
        depthShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
        depthShader->SetInt("layerIndex", shadowMapIdx);
        shadowMapArrayBuffer.SetLayer(shadowMapIdx);

        glClear(GL_DEPTH_BUFFER_BIT);
        
        // dummy vector
        std::vector<UI::Control*> controls;

        renderScene(depthShader.get(), true, controls);
        
        shadowMapIdx++;
    }
    shadowMapArrayBuffer.Unbind();
}




void dBrender::prepForSceneLoad() {
    preloadModelsFlag = true;
    m_gameObjects2D.clear();
    m_flatGameObjects.clear();
}

std::shared_ptr<Shader> dBrender::GetParticlesShader() { return m_ParticlesShader; }

void dBrender::PrepareBuffersAfterLoad() {

    // clear instances, because we do not know if they are there [later on, we will use/check whether it is here or not]
    // for now, just focus on using meshes from modelPaths

}

void dBrender::EnableMSAA() { glEnable(GL_MULTISAMPLE); }

void dBrender::DisableMSAA() { glDisable(GL_MULTISAMPLE); }

void dBrender::renderRefractObjects(Camera* camera) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_CULL_FACE);

    refractShader->Use();
    refractShader->SetMat4("view", camera->GetViewMatrix());
    refractShader->SetMat4("projection", camera->GetProjectionMatrix());
    refractShader->SetVec3("cameraPos", camera->gameObject->transform.GetGlobalPosition());

    if (auto m_worldEnvironment = m_activeScene->GetWorldEnvironment()) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_worldEnvironment->GetSkyboxTexture());
        refractShader->SetInt("skybox", 0);
    }

    /*for (auto it = m_gameObjects.begin(); it != m_gameObjects.end(); it++) {
        int meshIndex = it->first;
        auto &objects = it->second;

        MeshInstance *meshInstance = getValueOfInstancedMeshes(meshIndex);

        for (auto &obj: objects) {
            if (!obj->m_enabled)
                continue;

            auto mesh = obj->GetComponent<MeshInstance>();
            if (!mesh || !mesh->enabled)
                continue;

            if (mesh->isUsingRefraction()) {
                for (unsigned int j = 0; j < meshInstance->meshes.size(); j++) {
                    glBindVertexArray(meshInstance->meshes[j].VAO);
                    refractShader->SetMat4("model", obj->transform.GetModelMatrix());

                    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(meshInstance->meshes[j].indices.size()),
                                   GL_UNSIGNED_INT, 0);

                    glBindVertexArray(0);
                    EngineDebug::GetInstance().drawCalls++;
                }
            }
        }*/
    //}

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void dBrender::renderReflectObjects(Camera *camera) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_CULL_FACE);

    reflectShader->Use();
    reflectShader->SetMat4("view", camera->GetViewMatrix());
    reflectShader->SetMat4("projection", camera->GetProjectionMatrix());
    reflectShader->SetVec3("cameraPos", camera->gameObject->transform.GetGlobalPosition());
    return;
    /*if (m_worldEnvironment) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_worldEnvironment->GetSkyboxTexture());
        reflectShader->SetInt("skybox", 0);
    }

    for (auto it = m_gameObjects.begin(); it != m_gameObjects.end(); it++) {
        int meshIndex = it->first;
        auto &objects = it->second;

        MeshInstance *meshInstance = getValueOfInstancedMeshes(meshIndex);

        for (auto &obj: objects) {
            if (!obj->m_enabled)
                continue;

            auto mesh = obj->GetComponent<MeshInstance>();
            if (!mesh || !mesh->enabled)
                continue;

            if (mesh->isUsingRefraction()) {
                for (unsigned int j = 0; j < meshInstance->meshes.size(); j++) {
                    glBindVertexArray(meshInstance->meshes[j].VAO);
                    refractShader->SetMat4("model", obj->transform.GetModelMatrix());

                    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(meshInstance->meshes[j].indices.size()),
                                   GL_UNSIGNED_INT, 0);

                    glBindVertexArray(0);
                    EngineDebug::GetInstance().drawCalls++;
                }
            }
        }
    }*/

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void dBrender::renderVolumetricObjects(GameObject *gameObject) {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Get the MeshInstance for this mesh index
    MeshInstance *meshInstance = gameObject->GetComponent<MeshInstance>();

    // Check if the MeshInstance is set for volumetric rendering
    if (meshInstance->isUsingVolumetric()) {
        volumetricShader->Use();
        volumetricShader->SetFloat("density", meshInstance->density);
        volumetricShader->SetInt("samples", meshInstance->samples);
        volumetricShader->SetVec3("fogColor", meshInstance->fogColor);
        volumetricShader->SetFloat("scattering", meshInstance->scattering);
        glm::mat4 model = meshInstance->gameObject->transform.GetModelMatrix();
        volumetricShader->SetMat4("model", model);

        // meshInstance->model->GenerateAABB(); //it might be better to just load the model, but i do not want to do it
        // every time, so i just generate it every time :(
        AABB *aabb = meshInstance->model->pAABB
                             .get(); // since it is only for volumetric meshes it does not affect anything tbh.
        glm::vec3 minLocal = aabb->min;
        glm::vec3 maxLocal = aabb->max;

        // AABB Corners
        std::vector<glm::vec3> corners = {
                glm::vec3(minLocal.x, minLocal.y, minLocal.z), glm::vec3(minLocal.x, minLocal.y, maxLocal.z),
                glm::vec3(minLocal.x, maxLocal.y, minLocal.z), glm::vec3(minLocal.x, maxLocal.y, maxLocal.z),
                glm::vec3(maxLocal.x, minLocal.y, minLocal.z), glm::vec3(maxLocal.x, minLocal.y, maxLocal.z),
                glm::vec3(maxLocal.x, maxLocal.y, minLocal.z), glm::vec3(maxLocal.x, maxLocal.y, maxLocal.z)};


        std::vector<glm::vec3> worldCorners;
        for (const auto &corner: corners) {
            worldCorners.push_back(glm::vec3(model * glm::vec4(corner, 1.0)));
        }

        glm::vec3 aabbMinWorld = worldCorners[0];
        glm::vec3 aabbMaxWorld = worldCorners[0];
        for (const auto &wc: worldCorners) {
            aabbMinWorld = glm::min(aabbMinWorld, wc);
            aabbMaxWorld = glm::max(aabbMaxWorld, wc);
        }

        // Pass the "corrected" world space AABB to the shader
        volumetricShader->SetVec3("aabbMin", aabbMinWorld);
        volumetricShader->SetVec3("aabbMax", aabbMaxWorld);

        // Local is needed for Density stuff.
        volumetricShader->SetVec3("aabbMinLocal", minLocal);
        volumetricShader->SetVec3("aabbMaxLocal", maxLocal);

        for (unsigned int i = 0; i < meshInstance->model->Meshes.size(); i++) {
            glBindVertexArray(meshInstance->model->Meshes[i].VAO);
            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(meshInstance->model->Meshes[i].indices.size()),
                           GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            EngineDebug::GetInstance().drawCalls++;
        }
    }


    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void dBrender::preFlattenHierarchy(){
    m_flatGameObjects.clear();
    flattenHierarchy(m_activeScene->sceneRootObject.get());
}

void dBrender::flattenHierarchy(GameObject* gameObject) {
    m_flatGameObjects.push_back(gameObject);

    for (int i = 0; i < gameObject->children.size(); ++i) {
        flattenHierarchy(gameObject->children.at(i).get());
    }
}

void dBrender::removeObjectFromFlat(GameObject *gameObject) { 
    auto it = std::find_if(m_flatGameObjects.begin(), m_flatGameObjects.end(),
                           [gameObject](GameObject* &obj) { return obj->uuid == gameObject->uuid; });


    if (it != m_flatGameObjects.end()) {
        for (int i = 0; i < gameObject->children.size(); i++){
            removeObjectFromFlat(gameObject->children.at(i).get());
        }
        m_flatGameObjects.erase(it);
    }
}

void dBrender::RenderNavigation(Camera* camera) {
    if (wireframeShader) {
        wireframeShader->Use();
        wireframeShader->SetMat4("view", camera->GetViewMatrix());
        wireframeShader->SetMat4("projection", camera->GetProjectionMatrix());

        if (EngineSettings::GetEngineSettings().renderAgentPaths) {
            //AISystem::GetInstance()->RenderAgentPaths(wireframeShader);
        }
        if (EngineSettings::GetEngineSettings().renderNavigationMesh) {
            //AISystem::GetInstance()->RenderNavigationMesh(wireframeShader);
        }
    }
}

void dBrender::renderGlitchEffect() {
    // Update glitch time
    m_glitchTime += 0.032f;

    m_GlitchShader->Use();

    m_GlitchShader->SetFloat("time", m_glitchTime);
    m_GlitchShader->SetFloat("glitchIntensity", 0.2f);
    m_GlitchShader->SetFloat("digitalNoiseIntensity", 0.1f);
    m_GlitchShader->SetFloat("rgbShiftIntensity", 1.0f);
    m_GlitchShader->SetFloat("blockIntensity", 0.15f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer.getFrameTexture());

    renderFramebufferToQuad();
}
