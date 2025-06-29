#ifndef SHADOW_CUBE_MAP_BUFFER_H
#define SHADOW_CUBE_MAP_BUFFER_H

#include "../ShadowMapBuffer.h"

class ShadowCubeMapBuffer {

	public:
    static const unsigned int SHADOW_WIDTH, SHADOW_HEIGHT;

    unsigned int depthMapFBO;
    unsigned int depthMap;
		void Init() ;
        void Bind() ;
        void Unbind() ;
        unsigned int GetDepthMap();
};

#endif // !SHADOW_CUBE_MAP_BUFFER_H
