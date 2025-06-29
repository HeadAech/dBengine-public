#ifndef BLOOM_RENDERER_H
#define BLOOM_RENDERER_H

#include <glm/glm.hpp>
#include <Shader/Shader.h>
#include <memory>
#include <vector>

struct BloomMip {
    glm::vec2 size;
    glm::ivec2 intSize;
    unsigned int texture;
};

class BloomFramebuffer {
public:

    BloomFramebuffer();
    ~BloomFramebuffer() = default;

    bool Init(unsigned int width, unsigned int height, unsigned int mipChainLength);
    void Destroy();
    void BindForWriting();
    const std::vector<BloomMip> &MipChain() const;
    unsigned int m_FBO;

private:

    bool m_Initialized = false;
    std::vector<BloomMip> m_MipChain;
};

class BloomRenderer {
public:
    BloomRenderer();
    ~BloomRenderer() = default;

    bool Init(unsigned int width, unsigned int height);
    void Destroy();
    void RenderBloomTexture(unsigned int srcTexture, float filterRadius);

    unsigned int BloomTexture();
    unsigned int BloomMip_i(int index);

    void Rescale(unsigned int newWidth, unsigned int newHeight);

private:
    void renderDownsamples(unsigned int srcTexture);
    void renderUpsamples(float filterRadius);

    bool m_Initialized = false;
    BloomFramebuffer m_bloomFBO;
    glm::ivec2 m_SrcViewportSize;
    glm::vec2 m_SrcViewportSizeFloat;

    Shader* m_DownSampleShader;
    Shader* m_UpSampleShader;

    bool m_KarisAverageOnDownsample = true;

    void renderQuad();
};

#endif // !BLOOM_RENDERER_H
