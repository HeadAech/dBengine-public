//
// Created by Hubert Klonowski on 25/03/2025.
//

#include "EngineSettings.h"
#include "Signal/Signals.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "dBengine/EngineDebug/EngineDebug.h"

const std::string EngineSettings::SETTINGS_FILE = RES_DIR + "/editor_settings.txt";

bool EngineSettings::EditorEnabled = true; // Default value for editor enabled state

EngineSettings::EngineSettings() {
    
}

EngineSettings::~EngineSettings() {
    // Destructor
    SaveToFile(SETTINGS_FILE);
}

void EngineSettings::SetVSync(bool vsyncEnabled) {
    m_vsyncEnabled = vsyncEnabled;
}

EngineSettings &EngineSettings::GetEngineSettings() {
    static EngineSettings engineSettings;
    return engineSettings;
}

void EngineSettings::SetSelectedTheme(int i) { 
    selectedTheme = i; 
    Signals::Editor_SetTheme.emit(i);
}

void EngineSettings::SaveToFile(const std::string &filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open settings file for writing");
    }

    // Write settings as plain text
    file << "inPlayMode=" << m_inPlayMode << "\n";
    file << "vsyncEnabled=" << m_vsyncEnabled << "\n";
    file << "showDirectionalLightDepthMap=" << m_showDirectionalLightDepthMap << "\n";
    file << "selectedTheme=" << selectedTheme << "\n";
    file << "enableMSAA=" << m_enableMSAA << "\n";
    file << "MSAAsamples=" << m_MSAAsamples << "\n";
    file << "windowInFullscreen=" << windowInFullscreen << "\n";
    file << "shadowCutoffStart=" << shadowCutoffStart << "\n";
    file << "enableGlitchShader=" << enableGlitchShader << "\n";
    file << "renderNavigationMesh=" << renderNavigationMesh << "\n";
    file << "renderAgentPaths=" << renderAgentPaths << "\n"; 

    file.close();
}

EngineSettings EngineSettings::LoadFromFile(const std::string &filepath) {
    EngineSettings settings;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open settings file for reading");
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '=')) {
            std::string value;
            if (std::getline(iss, value)) {
                // Parse the value based on the key
                if (key == "inPlayMode") {
                    settings.m_inPlayMode = (value == "1");
                } else if (key == "vsyncEnabled") {
                    settings.m_vsyncEnabled = (value == "1");
                } else if (key == "showDirectionalLightDepthMap") {
                    settings.m_showDirectionalLightDepthMap = (value == "1");
                } else if (key == "selectedTheme") {
                    settings.selectedTheme = std::stoi(value);
                } else if (key == "enableMSAA") {
                    settings.m_enableMSAA = (value == "1");
                } else if (key == "MSAAsamples") {
                    settings.m_enableMSAA = std::stoi(value);
                } else if (key == "windowInFullscreen") {
                    settings.windowInFullscreen = (value == "1");
                } else if (key == "shadowCutoffStart") {
                    settings.shadowCutoffStart = std::stof(value);
                } else if (key == "enableGlitchShader") {
                    settings.enableGlitchShader = (value == "1");
                } else if (key == "renderNavigationMesh") {
                    settings.renderNavigationMesh = (value == "1");
                } else if (key == "renderAgentPaths") {
                    settings.renderAgentPaths = (value == "1");
                } 
                else {
                    std::cerr << "Unknown setting: " << key << std::endl;
                }
            }
        }
    }

    file.close();
    return settings;
}
