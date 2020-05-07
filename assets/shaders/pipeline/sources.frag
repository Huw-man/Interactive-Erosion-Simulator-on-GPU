#version 330 core

in vec2 UV;
in float intensity;

out vec4 color;

uniform sampler2D mask;

uniform float global_source_intensity;
uniform float delta_t;

void main() {
    // passthrough is covered by blend func
    color = vec4(0);
    color.y = texture(mask, UV).a*intensity*global_source_intensity*delta_t;
}