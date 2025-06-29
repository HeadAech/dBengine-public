//
// Created by Hubert Klonowski on 08/12/2024.
//

#ifndef UTIL_H
#define UTIL_H
#include <cmath>
#include <dBengine/EngineSettings/EngineSettings.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <iomanip>
#include <sstream>

#include "dBengine/EngineDebug/EngineDebug.h"
#include <iostream>
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include <random>

/// <summary>
/// Utility class providing various helper functions for mathematical operations, formatting, file handling etc.
/// </summary>
class Util {
public:

    /// <summary>
    /// Rounds a double value up to the specified number of decimal places.
    /// </summary>
    /// <param name="value">Value to be round up</param>
    /// <param name="decimal_places">Number of decimal places</param>
    /// <returns>Rounded up number (double)</returns>
    static double round_up(double value, int decimal_places) {
        const double multiplier = std::pow(10.0, decimal_places);
        return std::ceil(value * multiplier) / multiplier;
    }

    /// <summary>
    /// Formats a float value to a string with the specified precision.
    /// </summary>
    /// <param name="value">Value to format</param>
    /// <param name="precision">Precision</param>
    /// <returns>String representation of formatted value</returns>
    static std::string format(float value, int precision) {
        std::stringstream ss;
        ss << std::fixed  << std::setprecision(precision) << value;
        return ss.str();
    }

    /// <summary>
    /// Calculates Euler angles from a direction vector.
    /// </summary>
    /// <param name="direction">Direction vector</param>
    /// <returns>Euler angles (vec3)</returns>
    static glm::vec3 getEulerAnglesFromDirection(glm::vec3 direction) {
        direction = glm::normalize(-direction);
        float yaw = atan2(direction.x, direction.z); // Yaw
        float pitch = atan2(direction.y, sqrt(direction.x * direction.x + direction.z * direction.z));
        float roll = 0.0f;

        return {glm::degrees(pitch), glm::degrees(yaw), glm::degrees(roll)}; // Pitch, Yaw, Roll
    }


    /// <summary>
    /// Calculates the direction vector from Euler angles (pitch, yaw, roll).
    /// </summary>
    /// <param name="pitch"></param>
    /// <param name="yaw"></param>
    /// <param name="roll"></param>
    /// <returns>Direction vector</returns>
    static glm::vec3 GetDirectionFromEulerAngles(float pitch, float yaw, float roll = 0.0f) {
        pitch = glm::radians(pitch);
        yaw = glm::radians(yaw);
        roll = glm::radians(roll);
        glm::mat4 rotationYaw = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotationPitch = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 rotationRoll = glm::rotate(glm::mat4(1.0f), roll, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec4 forward(0.0f, 0.0f, -1.0f, 1.0f);

        // Combine the rotations: yaw * pitch * roll (order matters)
        glm::mat4 rotationMatrix = rotationYaw * rotationPitch * rotationRoll;

        // Apply the combined rotation to the default forward vector
        glm::vec4 rotated = rotationMatrix * forward;

        // Return the normalized direction vector
        return glm::normalize(-glm::vec3(rotated));
    }

    /// <summary>
    /// Linear interpolation between two values.
    /// </summary>
    /// <param name="a">From</param>
    /// <param name="b">To</param>
    /// <param name="t">Time</param>
    /// <returns>Lerp value</returns>
    static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    /// <summary>
    /// Linear interpolation between two vectors.
    /// </summary>
    /// <param name="a">From</param>
    /// <param name="b">To</param>
    /// <param name="t">Time</param>
    /// <returns>Lerp vector</returns>
    static glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t) {
        return a + t * (b - a);
    }

    /// <summary>
    /// Returns current date and time as a formatted string.
    /// </summary>
    /// <returns>String representation of current date and time.</returns>
    static std::string GetCurrentDateTime() {
        // Get current time point
        auto now = std::chrono::system_clock::now();

        // Convert to time_t (C-style time)
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        // Convert to local time
        std::tm localTime = *std::localtime(&now_c);

        // Format the date & time as a string
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);

        return std::string(buffer);
    }

    /// <summary>
    /// Returns current date as a formatted string.
    /// </summary>
    /// <returns>String representation of a current date.</returns>
    static std::string GetCurrentDate() {
        auto now = std::chrono::system_clock::now();

        // Convert to time_t (C-style time)
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        // Convert to local time
        std::tm localTime = *std::localtime(&now_c);

        // Format the date & time as a string
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &localTime);

        return std::string(buffer);
    }

    /// <summary>
    /// Copies a file from source to destination.
    /// </summary>
    /// <param name="sourcePath">Source</param>
    /// <param name="destinationPath">Destination</param>
    /// <returns>Boolean if operation was successful.</returns>
    static bool copyFile(const std::string &sourcePath, const std::string &destinationPath) {
        try {
            std::filesystem::copy(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing);
            return true;
        } catch (const std::exception &e) {
            std::cerr << "Error copying file: " << e.what() << std::endl;
            EngineDebug::GetInstance().PrintError("Error copying file from: " + sourcePath + " to: " + destinationPath);
            return false;
        }
    }

    /// <summary>
    /// Checks if a file exists at the specified path.
    /// </summary>
    /// <param name="destinationPath">Target path</param>
    /// <returns>Boolean</returns>
    static bool fileExists(const std::string &destinationPath) { return std::filesystem::exists(destinationPath); }

    static std::filesystem::file_time_type getFileModificationTime(const std::string &filePath) {
        try {
            return std::filesystem::last_write_time(filePath);
        } catch (const std::filesystem::filesystem_error &e) {
            throw std::runtime_error("Error accessing file: " + std::string(e.what()));
        }
    }

    /// <summary>
    /// Returns a list of script files in the scripts directory.
    /// </summary>
    /// <returns>List of scripts' names</returns>
    static std::vector<std::string> getScripts() {
        std::vector<std::string> fileList;

        try {
            for (const auto &entry: std::filesystem::directory_iterator(RES_DIR + "/scripts")) {
                if (std::filesystem::is_regular_file(entry)) { // Check if it's a file (not a directory)
                    //fileList.push_back(entry.path().string()); // Add file path to the list
                    fileList.push_back(std::filesystem::path(entry.path().stem().string()).string());
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Error reading directory: " << e.what() << std::endl;
        }

        return fileList;
    }

    /// <summary>
    /// Calculates the forward direction vector from a quaternion rotation.
    /// </summary>
    /// <param name="rotation">Quaternion</param>
    /// <returns>Forward vector</returns>
    static glm::vec3 GetForwardDirection(const glm::quat &rotation) { return rotation * glm::vec3(0.0f, 0.0f, -1.0f); }

    /// <summary>
    /// Returns a number from the end of a string.
    /// </summary>
    /// <param name="str">String</param>
    /// <returns>String representation of that number.</returns>
    static std::string GetNumberFromEndOfString(const std::string& str) { 
        std::string out;

        for (int i = str.length() - 1; i >= 0; i--) {
            if (isdigit(str[i])) {
                out += str[i];
            } else {
                break;
            }
        }
        std::reverse(out.begin(), out.end());
        return out;
    }

    /// <summary>
    /// Converts Assimp matrix to GLM matrix format.
    /// </summary>
    /// <param name="from">Assimp matrix</param>
    /// <returns>GLM matrix</returns>
    static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from) {
        glm::mat4 to;
        // the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        to[0][0] = from.a1;
        to[1][0] = from.a2;
        to[2][0] = from.a3;
        to[3][0] = from.a4;
        to[0][1] = from.b1;
        to[1][1] = from.b2;
        to[2][1] = from.b3;
        to[3][1] = from.b4;
        to[0][2] = from.c1;
        to[1][2] = from.c2;
        to[2][2] = from.c3;
        to[3][2] = from.c4;
        to[0][3] = from.d1;
        to[1][3] = from.d2;
        to[2][3] = from.d3;
        to[3][3] = from.d4;
        return to;
    }

    /// <summary>
    /// Converts Assimp vector to GLM vector format.
    /// </summary>
    /// <param name="vec">Assimp vector</param>
    /// <returns>GLM vector</returns>
    static inline glm::vec3 GetGLMVec(const aiVector3D &vec) { return glm::vec3(vec.x, vec.y, vec.z); }

    /// <summary>
    /// Converts Assimp quaternion to GLM quaternion format.
    /// </summary>
    /// <param name="pOrientation">Assimp quaternion</param>
    /// <returns>GLM quaternion</returns>
    static inline glm::quat GetGLMQuat(const aiQuaternion &pOrientation) {
        return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
    }


    /// <summary>
    /// Returns epoch time of a file in seconds [int]
    /// </summary>
    /// <param name="fileModifyTime">time in fs::file_time_type</param>
    /// <returns>time in seconds [int]</returns>
    static unsigned long long int getEpochTimeOfAFile(const std::filesystem::file_time_type& fileModifyTime){
        auto duration = fileModifyTime.time_since_epoch();
        auto timeInEpoch = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        return timeInEpoch;
    }

    /// <summary>
    /// Takes in reference to string with wrong path (example: "res\\models") and fixes it to "res/models".
    /// </summary>
    /// <param name="path">Reference to path</param>
    static void FixPathString(std::string& path)
    {
        std::replace(path.begin(), path.end(), '\\', '/');
    }

    /// <summary>
    /// returns relative filepath fs::path type
    /// </summary>
    /// <param name="path">absolute path</param>
    /// <returns>relative path</returns>
    static std::filesystem::path getRelativePath(std::string path) {
        std::filesystem::path absolutePath = path;
        std::filesystem::path currentPath = std::filesystem::current_path();

        std::filesystem::path relativePath;
        try {
            relativePath = std::filesystem::relative(absolutePath, currentPath); // get rid of system stuff
        } catch (const std::exception &e) {
            EngineDebug::GetInstance().PrintError("FAILED TO GET RELATIVE PATH - IDK, SETTING DEFAULT \"\"");
            relativePath = "";
        }

        std::filesystem::path cleanedPath;

        for (const auto &part: relativePath) { // get rid of .. \\ // stuff
            if (part == "..")
                continue;
            cleanedPath /= part;
        }
        return cleanedPath;
    }

    /// <summary>
    /// Calculates distance between two vec3.
    /// </summary>
    /// <param name="from"></param>
    /// <param name="to"></param>
    /// <returns></returns>
    static inline float GetDistance(const glm::vec3& from, const glm::vec3& to)
    {
        return glm::distance(from, to);
    }

    /// <summary>
    /// Creates empty script from template.
    /// </summary>
    /// <param name="destination">Target path of new script</param>
    static void CreateTemplateScript(const std::string& destination)
    {
        std::string sourcePath = SCRIPT_TEMPLATE_PATH;

        if (!Util::fileExists(destination))
        {
            if (Util::copyFile(sourcePath, destination))
            {
                EngineDebug::GetInstance().PrintDebug("Script created at: " + destination);
            }
        }
    }

    /// <summary>
    /// Moves an object inside a vector up or down one place.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="vec"></param>
    /// <param name="size"></param>
    /// <param name="moveUp"></param>
    template<typename T>
    static void MoveObject(std::vector<T>& vec, size_t index, bool moveUp)
    {
        if (vec.empty() || index >= vec.size())
            return;

        if (moveUp && index > 0)
        {
            std::swap(vec[index], vec[index - 1]);
        }
        else if (!moveUp && index < vec.size() - 1)
        {
            std::swap(vec[index], vec[index + 1]);
        }
    }
};

#endif //UTIL_H
