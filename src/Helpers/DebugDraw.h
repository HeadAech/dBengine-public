#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <string>
#include <memory>
#include "Shader/Shader.h"
#include <glm/gtc/constants.hpp>

/// <summary>
/// Utility class for debugging purposes, providing methods to draw lines, cubes, and spheres in 3D space.
/// </summary>
class DebugDraw
{

	public:
        
        DebugDraw() = default;
        ~DebugDraw() = default;
        DebugDraw(const DebugDraw &) = delete;
        DebugDraw &operator=(const DebugDraw &) = delete;
        DebugDraw(DebugDraw &&) = delete;
        
        /// <summary>
        /// Returns the singleton instance of DebugDraw.
        /// </summary>
        /// <returns>Singleton (DebugDraw)</returns>
        static DebugDraw &GetInstance() {
            static DebugDraw instance;
            return instance;
        }

        /// <summary>
        /// Sets the shader to be used for rendering debug shapes.
        /// </summary>
        /// <param name="shader">Pointer to the shader</param>
        void SetShader(std::shared_ptr<Shader> shader) { m_Shader = shader; }

        /// <summary>
        /// Draws a line between two points in 3D space with the specified color.
        /// </summary>
        /// <param name="start">Starting point (vec3)</param>
        /// <param name="end">End point (vec3)</param>
        /// <param name="color">Color (vec4)</param>
        void Line(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &color) 
        {
            static GLuint vao = 0, vbo = 0;
            if (vao == 0) {
                glGenVertexArrays(1, &vao);
                glGenBuffers(1, &vbo);
            }

            glm::vec3 points[2] = {start, end};

            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW);

            m_Shader->Use();
            m_Shader->SetVec4("u_Color", color);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *) 0);

            glDrawArrays(GL_LINES, 0, 2);

            glDisableVertexAttribArray(0);
            glBindVertexArray(0);
        };

        /// <summary>
        /// Draws a cube centered at a specified point with a given size and color.
        /// </summary>
        /// <param name="center">Center point (vec3)</param>
        /// <param name="size">Size (vec3)</param>
        /// <param name="color">Color (vec4)</param>
        void Cube(const glm::vec3 &center, const glm::vec3 &size, const glm::quat& rotation, const glm::vec4 &color) {
            glm::vec3 h = size * 0.5f;

            glm::vec3 local[8] = {
                {-h.x, -h.y, -h.z}, {h.x, -h.y, -h.z},
                {h.x,  h.y, -h.z}, {-h.x,  h.y, -h.z},
                {-h.x, -h.y,  h.z}, {h.x, -h.y,  h.z},
                {h.x,  h.y,  h.z}, {-h.x,  h.y,  h.z}
            };

            glm::vec3 c[8];
            for (int i = 0; i < 8; ++i)
                c[i] = center + rotation * local[i];  // rotate then translate

            int edges[12][2] = {
                {0, 1}, {1, 2}, {2, 3}, {3, 0},
                {4, 5}, {5, 6}, {6, 7}, {7, 4},
                {0, 4}, {1, 5}, {2, 6}, {3, 7}
            };

            for (int i = 0; i < 12; ++i)
                Line(c[edges[i][0]], c[edges[i][1]], color);
        }

        /// <summary>
        /// Draws a sphere centered at a specified point with a given radius and color.
        /// </summary>
        /// <param name="center">Center point (vec3)</param>
        /// <param name="radius">Radius (float)</param>
        /// <param name="color">Color (vec4)</param>
        void Sphere(const glm::vec3 &center, float radius, const glm::vec4 &color) {
            const int segments = 32;

            for (int i = 0; i < segments; ++i) {
                float t0 = glm::two_pi<float>() * i / segments;
                float t1 = glm::two_pi<float>() * (i + 1) / segments;

                // XY
                Line(center + glm::vec3(cos(t0), sin(t0), 0) * radius,
                                center + glm::vec3(cos(t1), sin(t1), 0) * radius, color);

                // XZ
                Line(center + glm::vec3(cos(t0), 0, sin(t0)) * radius,
                                center + glm::vec3(cos(t1), 0, sin(t1)) * radius, color);

                // YZ
                Line(center + glm::vec3(0, cos(t0), sin(t0)) * radius,
                                center + glm::vec3(0, cos(t1), sin(t1)) * radius, color);
            }
        }


        private:
            std::shared_ptr<Shader> m_Shader;
};

#endif // !DEBUG_DRAW_H
