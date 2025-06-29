#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>

class FrameBuffer {
public:
    FrameBuffer() = default;
    ~FrameBuffer();
    unsigned int getFrameTexture();
    void RescaleFrameBuffer(float width, float height);
    virtual void Bind() const;
    virtual void Unbind() const;
    virtual void Init(float width, float height);

    static FrameBuffer &GetInstance();

    float m_width = 0;
    float m_height = 0;
    unsigned int colorBuffer;
    unsigned int fbo;
    unsigned int texture;
    unsigned int rbo;

private:
    
    
};
