#include "ShadowCubeMapBuffer.h"

const unsigned int ShadowCubeMapBuffer::SHADOW_HEIGHT = 1024;
const unsigned int ShadowCubeMapBuffer::SHADOW_WIDTH = 1024;

void ShadowCubeMapBuffer::Init() {
	
	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, depthMap);

	/*for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}*/

    glTexImage3D(
        GL_TEXTURE_CUBE_MAP_ARRAY,
        0,
        GL_DEPTH_COMPONENT32F,
        ShadowCubeMapBuffer::SHADOW_WIDTH,
                 ShadowCubeMapBuffer::SHADOW_HEIGHT,
        6 * 32,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        NULL
    );

	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer not complete!\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowCubeMapBuffer::Bind() { glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); }

void ShadowCubeMapBuffer::Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

unsigned int ShadowCubeMapBuffer::GetDepthMap() { return depthMap; }
