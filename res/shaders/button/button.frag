#version 330 core

in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 u_Color;

uniform sampler2D u_Texture;
uniform bool u_HasTexture = false;

uniform float u_Emission = 10;

void main() {
	
	if (u_HasTexture)
	{
		vec4 baseTex = texture(u_Texture, TexCoords);

		if (baseTex.a < 0.05)
		{
			discard;
		}

		FragColor = vec4(baseTex.rgb + u_Color * u_Emission, baseTex.a);
	}
	else 
	{
		FragColor = vec4(u_Color * u_Emission, 1.0);
	}

	
}