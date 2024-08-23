#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec2 TexCoord;

void main()
{
	TexCoord = aTexCoord;
	gl_Position = Projection * View * Model * vec4(aPosition,1.0);
}