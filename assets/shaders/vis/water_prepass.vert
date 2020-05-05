#version 330 core

layout(location = 0) in vec2 vertexUV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform sampler2D T1_bds;
uniform vec2 texture_size;

out vec4 v_position;
out vec4 v_normal;
out vec2 v_uv;

void main() {
  vec4 self_bds = texture(T1_bds, vertexUV);
  vec4 v_model = vec4(vertexUV.x, self_bds.x+self_bds.y, vertexUV.y, 1);
  v_position = MVP * v_model;
  v_normal = vec4(0,1,0,1);
  v_uv = vertexUV;
  gl_Position = v_position;
}
