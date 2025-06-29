//
// Created by Hubert Klonowski on 25/03/2025.
//

#ifndef ENGINESETTINGS_H
#define ENGINESETTINGS_H
#include <cstdint>
#include <string>

#include <glm/glm.hpp>

constexpr int32_t GL_VERSION_MAJOR = 4;
constexpr int32_t GL_VERSION_MINOR = 1;

constexpr int32_t WINDOW_WIDTH  = 1800;
constexpr int32_t WINDOW_HEIGHT = 900;

const std::string RES_DIR = "res";
const std::string SCRIPT_TEMPLATE_PATH = "res/scripts/template.lua";

class EngineSettings {
public:
    static const std::string SETTINGS_FILE;

    static bool EditorEnabled;

    bool m_inPlayMode = true;

    bool m_vsyncEnabled = false;

    bool m_showDirectionalLightDepthMap = false;

    bool m_enableMSAA = true;
    int m_MSAAsamples = 4;

    int selectedTheme = 0;

    bool windowInFullscreen = false;

    float shadowCutoffStart = 50.0f;
    bool enableGlitchShader = false;

    bool renderNavigationMesh = false;
    bool renderAgentPaths = false;

    // shader parameters
    bool enableGlitchEffect = false;
    float glitchEffectIntensity = 0.1f;
    float glitchEffectFrequency = 40.0f;
    float vignetteStrength = 0.4f;

    float fisheyeStrength = 0.05f;
    float chromaticStrength = 0.001f;
    glm::vec3 shadowColor = { 0, 0, 1 };
    float shadowRadius = 0.0001f;
    glm::vec2 shadowOffset = { 0.005f, 0.01f };
    float shadowIntensity = 0.05f;
    float displacementStrength = 0.004f;
    float displacementSpeed = 10.0f;
    float scanlineHeight = 3.0f;
    float scanlineProbability = 0.002f;

    bool distortionEnabled = true;

    EngineSettings();
    ~EngineSettings();

    static EngineSettings& GetEngineSettings();

    void SetVSync(bool vsyncEnabled);

    void SetSelectedTheme(int i);

    void SaveToFile(const std::string &filepath) const;

    // Load settings from a plain text file
    static EngineSettings LoadFromFile(const std::string &filepath);

    private:

};



#endif //ENGINESETTINGS_H
