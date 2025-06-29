#version 330 core

in vec2 TexCoords;

out vec4 FragColor;
out vec4 BrightColor;

uniform vec4 u_Color;

uniform sampler2D u_Texture;
uniform bool u_HasTexture = false;

uniform float u_Emission = 10;

uniform float u_ClipLeft = 0.0;   // 0.0 to 1.0
uniform float u_ClipRight = 0.0;  // 0.0 to 1.0
uniform float u_ClipTop = 0.0;    // 0.0 to 1.0
uniform float u_ClipBottom = 0.0; // 0.0 to 1.0

void main() {

	// Clipping logic
    if (TexCoords.x < u_ClipLeft || TexCoords.x > 1.0 - u_ClipRight ||
        TexCoords.y < u_ClipBottom || TexCoords.y > 1.0 - u_ClipTop) 
    {
        discard;
        return;
    }
	
	if (u_HasTexture)
	{
		vec4 baseTex = texture(u_Texture, TexCoords);

		if (baseTex.a < 0.2)
		{
			discard;
			return;
		}

		vec3 colorAndEmission = vec3(u_Emission);
		if (u_Color.rgb != vec3(0))
		{
			colorAndEmission *= u_Color.rgb;
		}

		FragColor = vec4(baseTex.rgb * colorAndEmission, baseTex.a * u_Color.a);
		BrightColor = vec4(baseTex.rgb * u_Emission, 1.0);
	}
	else 
	{
		FragColor = vec4(u_Color.rgb * u_Emission, u_Color.a);
	}

	
}