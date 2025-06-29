#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <glm/glm.hpp>

/// <summary>
/// Utility class for clipboard operations.
/// </summary>
class Clipboard
{
	public:
    
        Clipboard() = default;

        /// <summary>
        /// Returns the singleton instance of the Clipboard class.
        /// </summary>
        /// <returns>Singleton (Clipboard)</returns>
        static Clipboard& GetInstance()
        {
            static Clipboard instance;
            return instance;
        }

        Clipboard(const Clipboard &) = delete;
        Clipboard &operator=(const Clipboard &) = delete;

        // values

        /// <summary>
        /// Copied glm::vec3 value.
        /// </summary>
        glm::vec3 CopiedVec3;

        /// <summary>
        /// Copied string.
        /// </summary>
        std::string CopiedString;

        /// <summary>
        /// Copied integer value.
        /// </summary>
        int CopiedInt;

        /// <summary>
        /// Copied floating point value.
        /// </summary>
        float CopiedFloat;

        /// <summary>
        /// Copied ImVec2 value.
        /// </summary>
        ImVec2 CopiedImVec2;

        /// <summary>
        /// Copied glm::vec2 value.
        /// </summary>
        glm::vec2 CopiedVec2;
};

#endif // !CLIPBOARD_H
