#version 420
in vec4 worldPos; 
in vec2 vUV

out vec2 fUV;

uniform mat4 MVP;
uniform mat4 MV;
void main()
{
    vec4 screen_pos = MVP * worldPos;
    gl_Position = screen_pos / screen_pos.w;
    fUV = vUV;
}