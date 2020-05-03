#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform sampler2D T1_bds;

out vec4 v_position;
out vec4 v_normal;
out vec2 v_uv;

void main() {
  vec4 self_bds = texture(T1_bds, vertexUV);

  v_position = vec4(vertexPosition_modelspace, 1);
  v_position.y = self_bds.x + self_bds.y; //adjusting the height to be the terrain+water level
  v_normal = vec4(vertexNormal, 1);
  v_uv = vertexUV;
  gl_Position = MVP * v_position;
}
