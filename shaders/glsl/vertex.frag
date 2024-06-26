#version 450

layout(set = 1, binding = 0) uniform sampler texSampler;
layout(set = 2, binding = 0) uniform texture2D tex2D;

layout(location = 0) in vec4 fragNormal;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(sampler2D(tex2D, texSampler), fragTexCoord);
}