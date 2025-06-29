#ifndef FRAMEBUFFER_MULTISAMPLE_H
#define FRAMEBUFFER_MULTISAMPLE_H

#include "../Framebuffer/Framebuffer.h"
#include "Helpers/UUID/UUID.h"

class FramebufferMultisample {
    std::string uuid;

	public:
    float m_width = 0;
    float m_height = 0;
    int samples = 4;
        unsigned int fbo;
        unsigned int texture;
        unsigned int colorBuffer;
        unsigned int rbo;
        
		void Init();
        void Bind();
        void Unbind();

        void RescaleFrameBuffer(float width, float height);
        void SetMSAASamples(int samples);

};


#endif // !FRAMEBUFFER_MULTISAMPLE_H
