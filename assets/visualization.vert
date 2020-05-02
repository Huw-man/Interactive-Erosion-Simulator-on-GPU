#version 330 core

uniform mat4 u_view_projection; // world to view to screen? transform (from proj4 shaders spec)
uniform mat4 u_model; // model to world transform

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s
uniform vec2 texture_size;

in vec4 in_position;
in vec4 in_normal;
in vec2 in_uv;

out vec4 v_position;
out vec4 v_normal;
out vec2 v_uv;

void main() {
  vec4 self_bds = texture(T1_bds, UV);
  float height = self_bds.x + self_bds.y;

  v_position = u_model * in_position;
  v_position.z = height; //adjusting the height to be the terrain+water level
  v_normal = normalize(u_model * in_normal);
  v_uv = in_uv;
  gl_Position = u_view_projection * v_position;
}
