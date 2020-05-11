#version 330 core

layout(location = 0) in vec2 vertexUV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform sampler2D T1_bds;
uniform vec2 texture_size;
uniform mat4 MV;
uniform mat4 P;

out vec4 v_position;
out vec4 v_normal;
out vec2 v_uv;
out vec4 m_position;

vec3 getVPos(vec2 uv) {
  vec4 bds = texture(T1_bds, uv);
  return vec3(uv.x, bds.x / 100.0, uv.y);
}

void main() {
  vec3 o = getVPos(vertexUV);
  vec3 a = getVPos(vertexUV + vec2( 1.0 / texture_size.x, 0                   ));
  vec3 b = getVPos(vertexUV + vec2(0                    , 1.0 / texture_size.y));
  vec3 c = getVPos(vertexUV + vec2(-1.0 / texture_size.x, 0                   ));
  vec3 d = getVPos(vertexUV + vec2(0                    ,-1.0 / texture_size.y));

  vec4 norm = vec4(normalize(cross(b-o,a-o) + cross(c-o,b-o) + cross(d-o,c-o) + cross(a-o,d-o)), 0);
  norm = normalize(MV * norm);

  vec4 p_model = vec4(o, 1);

  m_position = p_model;

  vec4 v_pos = MV * p_model;
  v_position = v_pos;
  v_normal = norm;
  v_uv = vertexUV;
  
  vec4 glpos = MVP * p_model;
  gl_Position = glpos;
}
