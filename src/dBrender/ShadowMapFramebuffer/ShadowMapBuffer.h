#ifndef SHADOW_MAP_BUFFER_H
#define SHADOW_MAP_BUFFER_H

#include "../Framebuffer/Framebuffer.h"

class ShadowMapBuffer {

    public:
    static const unsigned int SHADOW_WIDTH ;
        static const  unsigned int SHADOW_HEIGHT;

        unsigned int depthMapFBO;
    unsigned int depthMap;
        virtual void Init();

        virtual void Bind();
        virtual void Unbind();

        unsigned int GetDepthMap();

        void SetLayer(int layer);
};

#endif // !SHADOW_MAP_BUFFER_H