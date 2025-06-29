#include "WorldEnvironment.h"
#include <stb_image.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <GLFW/glfw3.h>
#include "Components/Camera/Camera.h"

WorldEnvironment::WorldEnvironment(const std::string &skyboxPath) { 
	name = "WorldEnvironment";
    icon = ICON_FA_GLOBE;

	SetSkyboxPath(skyboxPath);
    
}


void WorldEnvironment::RenderCube() {
    static GLuint cubeVAO = 0;
    static GLuint cubeVBO = 0;

    if (cubeVAO == 0) {
        float vertices[] = {// positions
                             // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f};

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);

        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    }

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

GLuint WorldEnvironment::GetSkyboxTexture() { return m_envCubemap;  }

void WorldEnvironment::RenderSkybox(Camera* camera, DirectionalLight* dirLight) {
    if (!m_HDRILoaded)
    {
        LoadHDRI();
    }

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    if (m_skyType == PHYSICAL) {

        m_physicalSkyShader->Use();
        m_physicalSkyShader->SetMat4("projection", camera->GetProjectionMatrix());
        m_physicalSkyShader->SetMat4("view", glm::mat4(glm::mat3(camera->GetViewMatrix()))); // remove translation
        m_physicalSkyShader->SetVec3("sunDirection", -dirLight->GetDirection());
        m_physicalSkyShader->SetVec3("sunColor", glm::vec3(1.0f)); // optional tint
        m_physicalSkyShader->SetFloat("timeOfDay", 3);
    } else {

        m_hdriSkyboxShader->Use();
        m_hdriSkyboxShader->SetMat4("projection", camera->GetProjectionMatrix());
        m_hdriSkyboxShader->SetMat4("view", glm::mat4(glm::mat3(camera->GetViewMatrix()))); // remove translation
        m_hdriSkyboxShader->SetInt("skybox", 0);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, GetSkyboxTexture());


    }
    /*glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);*/
    RenderCube();
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    //glDepthFunc(GL_LESS); // Reset depth function

    // // Disable writing to depth buffer and disable depth test
    //glDepthMask(GL_FALSE);
    //glDisable(GL_DEPTH_TEST);

    //m_volumetricShader->Use();

    //// Pass uniforms for lighting and camera
    //float time = glfwGetTime();
    //m_volumetricShader->SetVec3("sunDirection", -dirLight->GetDirection());
    //m_volumetricShader->SetFloat("time", time);
    //m_volumetricShader->SetVec3("cameraPos", camera->gameObject->transform.GetGlobalPosition());

    //// Pass the camera's inverse projection and view matrices
    //glm::mat4 projection = camera->GetProjectionMatrix();
    //glm::mat4 view = camera->GetViewMatrix();
    //glm::mat4 invProjection = glm::inverse(projection);
    //glm::mat4 invView = glm::inverse(view);
    //m_volumetricShader->SetMat4("invProjection", invProjection);
    //m_volumetricShader->SetMat4("invView", invView);

    //// Setup cloud appearance parameters (adjust as needed)
    //m_volumetricShader->SetFloat("cloudLayerHeight", 100.0f); // World-space y coordinate for cloud start
    //m_volumetricShader->SetFloat("cloudThickness", 50.0f); // Cloud vertical extent
    //m_volumetricShader->SetFloat("cloudScale", 0.001f); // Noise scale factor
    //m_volumetricShader->SetInt("numSteps", 64); // Number of raymarching steps
    //m_volumetricShader->SetFloat("stepSize", 1.0f); // Distance between steps

    //RenderQuad();

    //glDepthMask(GL_TRUE);
    //glEnable(GL_DEPTH_TEST);

}

void WorldEnvironment::SetSkyType(int skyType) { 
    this->m_skyType = (Sky) skyType; 
    //if (m_skyType == PHYSICAL) {
    //    // rn nothing.
    //} else if (m_skyType == HDRI) {
    //    
    //}
}

void WorldEnvironment::SetShaders(Shader* hdriProcessShader, Shader* hdriSkyboxShader,
                                  Shader* physicalSkyShader, Shader* volumetricShader, Shader* irradianceConvoShader) 
{
    m_hdriSkyboxShader = hdriSkyboxShader;
    m_physicalSkyShader = physicalSkyShader;
    m_volumetricShader = volumetricShader;
    m_hdriProcessshader = hdriProcessShader;
    m_irradianceConvoShader = irradianceConvoShader;
}


void WorldEnvironment::LoadHDRI() {

    if (m_hdrTexture != 0) {
        glDeleteTextures(1, &m_hdrTexture);
        m_hdrTexture = 0;
    }
    if (m_envCubemap != 0) {
        glDeleteTextures(1, &m_envCubemap);
        m_envCubemap = 0;
    }
    if (m_irradianceMap != 0) {
        glDeleteTextures(1, &m_irradianceMap);
        m_irradianceMap = 0;
    }
    
    glUseProgram(0);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    float *data = stbi_loadf(m_skyboxPath.c_str(), &width, &height, &nrComponents, 0);
    

    // Load HDR texture
    glGenTextures(1, &m_hdrTexture);
    glBindTexture(GL_TEXTURE_2D, m_hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);

    // Setup cubemap to render
    glGenTextures(1, &m_envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Setup PBR framebuffer
    glGenFramebuffers(1, &m_captureFBO);
    glGenRenderbuffers(1, &m_captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO);

    // Projection & view matrices for capturing data to cubemap directions
    glm::mat4 captureViews[] = {glm::lookAt(glm::vec3(0.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
                                glm::lookAt(glm::vec3(0.0), glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
                                glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
                                glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)),
                                glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)),
                                glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0))};

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

    m_hdriProcessshader->Use();
    m_hdriProcessshader->SetInt("equirectangularMap", 0);
    m_hdriProcessshader->SetMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_hdrTexture);

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    for (unsigned int i = 0; i < 6; ++i) {
        m_hdriProcessshader->SetMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_envCubemap,
                               0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderCube(); // a cube that covers all directions
    }

    // Setup irradiance cubemap
    glGenTextures(1, &m_irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Resizing buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
    
    // Convolution to create an irradiance cubemap
    m_irradianceConvoShader->Use();
    m_irradianceConvoShader->SetFloat("irradianceStrength", irradianceStrength);
    m_irradianceConvoShader->SetFloat("sampleDelta", sampleDelta);
    m_irradianceConvoShader->SetInt("skybox", 0);
    m_irradianceConvoShader->SetMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    for (unsigned int i = 0; i < 6; ++i) {
        m_irradianceConvoShader->SetMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               m_irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube();
    }    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0); 
    m_HDRILoaded = true;
}

void WorldEnvironment::setHDRILoaded(bool isHDRILoaded) {
    this->m_HDRILoaded = isHDRILoaded;
}
bool WorldEnvironment::IsHDRILoaded() const { return m_HDRILoaded; }

void WorldEnvironment::SetSkyboxPath(const std::string &path) { 
    m_skyboxPath = path; 
    m_HDRILoaded = false;
}

int WorldEnvironment::GetSkyType() { return m_skyType; }

void WorldEnvironment::RenderQuad() {
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
