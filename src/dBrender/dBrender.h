//
// Created by Hubert Klonowski on 17/03/2025.
//

#ifndef INSTANCEMANAGER_H
#define INSTANCEMANAGER_H
#include <map>

#include "Shader/Shader.h"
#include <vector>

#include "Scene/Scene.h"
#include "Framebuffer/Framebuffer.h"
#include <GLFW/glfw3.h>
#include "ShadowMapFramebuffer/ShadowMapBuffer.h"
#include "ShadowMapFramebuffer/ShadowCubeMapBuffer/ShadowCubeMapBuffer.h"
#include "FramebufferMultisample/FramebufferMultisample.h"
#include <Components/WorldEnvironment/WorldEnvironment.h>
#include <Components/Camera/Camera.h>
#include <Material/Material.h>
#include "PostProcessing/Bloom/BloomRenderer.h"
#include "Components/Camera/Camera.h"
#include "Components/Control/Control.h"

class Mesh;
class GameObject;

class MeshInstance;

struct InstanceData {
    glm::mat4 modelMatrix;
    int visible;
};

//TODO: Optimize loading time and enable adding just modelMatrices without assigned gameobjects
class dBrender {

    std::string uuid;
    Shader* shader;
    std::shared_ptr<Shader> m_TextShader;
    std::unique_ptr<Shader> depthShader;
    std::unique_ptr<Shader> depthCubeMapShader;
    std::unique_ptr<Shader> debugDepthShader;
    std::unique_ptr<Shader> debugDepthCubeMapShader;
    
    std::unique_ptr<Shader> m_LightShader;
    std::unique_ptr<Shader> refractShader;  
    std::unique_ptr<Shader> reflectShader;
    std::unique_ptr<Shader> volumetricShader;
    std::unique_ptr<Shader> irradianceConvoShader;
    Shader* wireframeShader;

    std::unique_ptr<Shader> m_GlitchShader;
    float m_glitchTime = 0.0f;

    Camera *m_activeCamera = nullptr;
    std::unique_ptr<Shader> m_AnimShader;

    std::shared_ptr<Shader> m_ParticlesShader;
    std::shared_ptr<Shader> m_DebugDrawShader;
    
    std::shared_ptr<Shader> m_ButtonShader;

    std::unique_ptr<Shader> m_ScreenShader;

    std::shared_ptr<Shader> m_SpriteShader;

    std::shared_ptr<Shader> m_HUDShader;

    std::shared_ptr<Shader> m_TransitionShader;

    bool firstStart = true;

    bool preloadModelsFlag = true;

    void setupShaders();

    void renderFramebufferToQuad();

    void renderScene(Shader* shader, bool shadowPass, std::vector<UI::Control*>& controls, const glm::vec3& lightPos = glm::vec3(0.0f), float shadowDistance = 50.0f);
    void renderLights();

    void renderDirectionalLight(GameObject* pDirLightObj, DirectionalLight* pDirLight);
    void renderPointLight(GameObject* pPointLightObj, PointLight* pPointLight, int i);
    void renderSpotLight(GameObject* pSpotLightObj, SpotLight* pSpotLight, int i);
    
    //shadow maps
    GLuint lightSpaceUBO;
    std::vector<std::unique_ptr<ShadowMapBuffer>> shadowMaps;
    std::vector<std::unique_ptr<ShadowCubeMapBuffer>> shadowCubeMaps;
    ShadowCubeMapBuffer shadowCubeMapArrayBuffer;
    ShadowMapBuffer shadowMapArrayBuffer;
    std::vector<glm::mat4> lightSpaceMatrices;
    std::vector<glm::vec4> lightPositions;

    void generateShadowMaps();


    void draw(MeshInstance& mesh, glm::mat4& model);
    void drawInstanced(MeshInstance& mesh, int instanceCount);

    Frustum m_cameraFrustum;

    GLuint m_BonesUBO;

    bool m_ForceRenderLights = true;

public:
    /// <summary>
    /// Shader for rendering HDRI skybox.
    /// </summary>
    std::unique_ptr<Shader> m_hdriSkyboxShader;

    /// <summary>
    /// Shader for processing HDRI images.
    /// </summary>
    std::unique_ptr<Shader> m_hdriProcessshader;

    /// <summary>
    /// Shader for rendering the physical sky.
    /// </summary>
    std::unique_ptr<Shader> m_physicalSkyShader;
    std::unique_ptr<Shader> m_volumetricShader;
    std::unique_ptr<Shader> m_irradianceConvoShader;

    /// <summary>
    /// Shader for rendering the scene in HDR.
    /// </summary>
    std::unique_ptr<Shader>hdrShader;

    FrameBuffer framebuffer;
    FrameBuffer renderFrameBuffer;
    FramebufferMultisample framebufferMultisample;
    FrameBuffer UIFramebuffer;

    /// <summary>
    /// Renderer responsible for bloom post-processing effect.
    /// </summary>
    BloomRenderer bloomRenderer;
    BloomRenderer HUDBloomRenderer;

    ShadowMapBuffer shadowMapBuffer;

    /// <summary>
    /// Pointer to current GLFW window.
    /// </summary>
    GLFWwindow* window = nullptr;
    
    std::vector<GameObject*> m_gameObjects2D;
    std::vector<GameObject *> m_flatGameObjects;

    /// <summary>
    /// Pointer to the currently active scene.
    /// </summary>
    Scene* m_activeScene = nullptr;

    dBrender();
    dBrender(const dBrender&) = delete;
    dBrender& operator=(const dBrender&) = delete;

    /// <summary>
    /// Returns the singleton instance of the dBrender class.
    /// </summary>
    /// <returns>Singleton (dBrender)</returns>
    static dBrender& GetInstance();

    Shader *GetShader();
    /// <summary>
    /// Marked for deletion
    /// </summary>
    /// <param name="shader"></param>
    void SetShader(Shader* shader);

    /// <summary>
    /// Creates all shader programs used in the renderer.
    /// </summary>
    void CreateShaderPrograms();

    /// <summary>
    /// Initializes the renderer, setting up necessary resources and configurations.
    /// </summary>
    void Init();

    /// <summary>
    /// Ask zMagu
    /// </summary>
    void PrepareBuffersAfterLoad();

    /// <summary>
    /// Enables the use of Multisample Anti-Aliasing (MSAA) for rendering.
    /// </summary>
    void EnableMSAA();

    /// <summary>
    /// Disables Multisample Anti-Aliasing (MSAA) for rendering.
    /// </summary>
    void DisableMSAA();

    // idk
    void renderRefractObjects(Camera *camera);
    void renderReflectObjects(Camera *camera);
    void renderVolumetricObjects( GameObject* gameObject);

    /// <summary>
    /// Renders the scene using the specified camera.
    /// </summary>
    /// <param name="camera">Pointer to the camera</param>
    void Render(Camera* camera);

    void RenderCollisionShapes(Camera* camera);

    void SetActiveCamera(Camera *camera) { m_activeCamera = camera; }

    Camera *GetActiveCamera() { return m_activeCamera; }

    void prepForSceneLoad();
    void preFlattenHierarchy();
    void flattenHierarchy(GameObject *gameObject);

    void removeObjectFromFlat(GameObject* uuid);

    std::shared_ptr<Shader> GetParticlesShader();

    glm::vec3 cameraPosition;

    FramebufferMultisample& GetFrameBufferMultiSample()
    {
        return framebufferMultisample;
    }

    FrameBuffer& GetFramebuffer()
    {
        return framebuffer;
    }

    FrameBuffer& GetRenderFramebuffer()
    {
        return renderFrameBuffer;
    }

    void RenderNavigation(Camera* camera);

    void renderGlitchEffect();
};



#endif //INSTANCEMANAGER_H