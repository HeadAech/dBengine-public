#ifndef RANDOM_H
#define RANDOM_H

#include <random>
#include <glm/glm.hpp>

/// <summary>
/// Utility class for generating random numbers.
/// </summary>
class Random
{

    public:
        /// <summary>
        /// Generates a random float between the specified minimum and maximum values.
        /// </summary>
        /// <param name="min">Minimum float</param>
        /// <param name="max">Maximum float</param>
        /// <returns>Random float</returns>
        static inline float Float(float min, float max) {
            static std::random_device rd; // Seed
            static std::mt19937 gen(rd()); // Mersenne Twister generator
            std::uniform_real_distribution<float> dist(min, max);
            return dist(gen);
        }

        /// <summary>
        /// Returns a random vector in a unit sphere.
        /// </summary>
        /// <returns>Random vector</returns>
        static inline glm::vec3 InUnitSphere()
        {
            static std::random_device rd; // Seed
            static std::mt19937 gen(rd()); // Mersenne Twister generator
            std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
            float x, y, z;
            do {
                x = dist(gen);
                y = dist(gen);
                z = dist(gen);
            } while (x * x + y * y + z * z >= 1.0f);
            return glm::vec3(x, y, z);
        }
};

#endif // !RANDOM_H
