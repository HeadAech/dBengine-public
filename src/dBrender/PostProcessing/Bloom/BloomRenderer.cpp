#include "BloomRenderer.h"

BloomFramebuffer::BloomFramebuffer() : m_Initialized(false) {}


bool BloomFramebuffer::Init(unsigned int width, unsigned int height, unsigned int mipChainLength) {
    if (m_Initialized)
        return true;

    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glm::vec2 mipSize((float) width, (float) height);
    glm::ivec2 mipIntSize((int) width, (int) height);

    //Safety check
    if (width > (unsigned int) INT_MAX || height > (unsigned int) INT_MAX) {
        return false;
    }

    for (GLuint i = 0; i < mipChainLength; i++) {
        BloomMip bloomMip;

        mipSize *= 0.5f;
        mipIntSize /= 2;
        bloomMip.size = mipSize;
        bloomMip.intSize = mipIntSize;

        glGenTextures(1, &bloomMip.texture);
        glBindTexture(GL_TEXTURE_2D, bloomMip.texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, (int) mipSize.x, (int) mipSize.y, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        m_MipChain.emplace_back(bloomMip);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_MipChain[0].texture, 0);

    //setup attachments
    unsigned int attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    // check completion status
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Bloom framebuffer not complete: " << status << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_Initialized = true;
    return true;
}

void BloomFramebuffer::Destroy() {
    for (int i = 0; i < (int) m_MipChain.size(); i++) {
        glDeleteTextures(1, &m_MipChain[i].texture);
        m_MipChain[i].texture = 0;
    }
    m_MipChain.clear();
    glDeleteFramebuffers(1, &m_FBO);
    m_FBO = 0;
    m_Initialized = false;
}

void BloomFramebuffer::BindForWriting() { glBindFramebuffer(GL_FRAMEBUFFER, m_FBO); }
const std::vector<BloomMip> &BloomFramebuffer::MipChain() const { return m_MipChain; }

BloomRenderer::BloomRenderer() : m_Initialized(false) {}

bool BloomRenderer::Init(unsigned int width, unsigned int height) {
    if (m_Initialized)
        return true;

    m_SrcViewportSize = glm::ivec2(width, height);
    m_SrcViewportSizeFloat = glm::vec2((float)width,(float) height);

    // FBO
    const unsigned int numBloomNips = 6; 
    bool status = m_bloomFBO.Init(width, height, numBloomNips);
    if (!status) {
        std::cout << "Bloom framebuffer not initialized" << std::endl;
        return false;
    }

    // Shaders
    m_DownSampleShader =
            new Shader("res/shaders/bloom/downsample.vert", "res/shaders/bloom/downsample.frag");
    m_UpSampleShader =
            new Shader("res/shaders/bloom/upsample.vert", "res/shaders/bloom/upsample.frag");

    // downsample
    m_DownSampleShader->Use();
    m_DownSampleShader->SetInt("srcTexture", 0);
    glUseProgram(0);

    m_UpSampleShader->Use();
    m_UpSampleShader->SetInt("srcTexture", 0);
    glUseProgram(0);

    //m_Initialized = true;
    return true;
}

void BloomRenderer::Destroy() {
    m_bloomFBO.Destroy();
    m_Initialized = false;
}

void BloomRenderer::renderDownsamples(unsigned int srcTexture) {
    const std::vector<BloomMip> &mipChain = m_bloomFBO.MipChain();

    m_DownSampleShader->Use();
    m_DownSampleShader->SetVec2("srcResolution", m_SrcViewportSizeFloat);
    if (m_KarisAverageOnDownsample) {
        m_DownSampleShader->SetInt("mipLevel", 0);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);

    for (int i = 0; i < (int) mipChain.size(); i++) {
        const BloomMip &mip = mipChain[i];
        glViewport(0, 0, mip.size.x, mip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

        renderQuad();

        m_DownSampleShader->SetVec2("srcResolution", mip.size);
        glBindTexture(GL_TEXTURE_2D, mip.texture);
        if (i == 0) {
            m_DownSampleShader->SetInt("mipLevel", 1);
        }

    }

    glUseProgram(0);
}

void BloomRenderer::renderUpsamples(float filterRadius) {
    const std::vector<BloomMip> &mipChain = m_bloomFBO.MipChain();

    m_UpSampleShader->Use();
    m_UpSampleShader->SetFloat("filterRadius", filterRadius);

    //enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    for (int i = (int) mipChain.size() - 1; i > 0; i--) {
        const BloomMip &mip = mipChain[i];
        const BloomMip &nextMip = mipChain[i - 1];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        glViewport(0, 0, nextMip.size.x, nextMip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextMip.texture, 0);

        renderQuad();
    }
    glDisable(GL_BLEND);
    glUseProgram(0);
}

void BloomRenderer::RenderBloomTexture(unsigned int srcTexture, float filterRadius)
{ 
    m_bloomFBO.BindForWriting();

    this->renderDownsamples(srcTexture);

    this->renderUpsamples(filterRadius);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //restore viewport
    glViewport(0, 0, m_SrcViewportSize.x, m_SrcViewportSize.y);
}

GLuint BloomRenderer::BloomTexture() { return m_bloomFBO.MipChain()[0].texture; }

GLuint BloomRenderer::BloomMip_i(int index) {
    const std::vector<BloomMip> &mipChain = m_bloomFBO.MipChain();
    int size = (int) mipChain.size();
    return mipChain[(index > size - 1) ? size - 1 : (index < 0) ? 0 : index].texture;
}

void BloomRenderer::Rescale(unsigned int newWidth, unsigned int newHeight) {
    m_bloomFBO.Destroy();

    // Update the viewport size
    m_SrcViewportSize = glm::ivec2(newWidth, newHeight);
    m_SrcViewportSizeFloat = glm::vec2((float) newWidth, (float) newHeight);

    // Reinitialize the framebuffer with the new dimensions
    const unsigned int numBloomMips = 6; // Keep the same number of mip levels
    bool status = m_bloomFBO.Init(newWidth, newHeight, numBloomMips);
    if (!status) {
        std::cerr << "Failed to rescale BloomRenderer framebuffer!" << std::endl;
    }
}

void BloomRenderer::renderQuad() {
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
