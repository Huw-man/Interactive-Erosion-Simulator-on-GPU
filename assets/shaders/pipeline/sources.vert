#version 330 core

layout(location = 0) in vec2 vertexPosition_modelspace;
layout(location = 1) in vec3 uv_mask; // also intensity

out vec2 UV;
out float intensity;

void main() {
    UV = uv_mask.xy;
    intensity = uv_mask.z;
    gl_Position = vec4(vertexPosition_modelspace*2-1,0,1);
}

