#version 450 core

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec2 inTexcoord;
layout (location = 2) in vec4 inColor;

layout (binding = 0) uniform UBO {
	mat4 projection;
} ubo;

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec4 outColor;


void main()
{
	gl_Position = ubo.projection * inPosition;
	outColor = inColor;
	outTexcoord = inTexcoord;
}