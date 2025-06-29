#ifndef WORLD_ENVIRONMENT_H
#define WORLD_ENVIRONMENT_H

#include <Component/Component.h>
#include <glad/glad.h>
#include <Shader/Shader.h>
#include <Components/Lights/DirectionalLight/DirectionalLight.h>

class Camera;

enum Sky { HDRI, PHYSICAL };

class WorldEnvironment : public Component {

    

    GLuint m_envCubemap;
    GLuint m_captureFBO;
    GLuint m_captureRBO;

    GLuint m_hdrTexture;


    Shader* m_hdriSkyboxShader;
    Shader* m_hdriProcessshader;
    Shader* m_physicalSkyShader;
    Shader* m_volumetricShader;
    Shader *m_irradianceConvoShader;

    Sky m_skyType = HDRI;

    void setupQuad();

    bool m_HDRILoaded = false;

public:
    float irradianceStrength = 1.0f;
    float sampleDelta = 0.1f;

    WorldEnvironment(const std::string& skyboxPath);
    std::string m_skyboxPath;
    ~WorldEnvironment() = default; 

    GLuint GetSkyboxTexture();
    GLuint m_irradianceMap;
    void RenderCube();
    void RenderQuad();
    void RenderSkybox(Camera *camera, DirectionalLight *dirLight = nullptr);

    int GetSkyType();

    void SetSkyType(int skyType);
    void SetShaders(Shader* hdriProcessShader, Shader* hdriSkyboxShader,
                    Shader* physicalSkyShader, Shader* volumetricShader, Shader* irradianceConvoShader);

    void LoadHDRI();

    void setHDRILoaded(bool isHDRILoaded);
    bool IsHDRILoaded() const;

    void SetSkyboxPath(const std::string &path);

    
 

};



#endif // !WORLD_ENVIRONMENT_H

