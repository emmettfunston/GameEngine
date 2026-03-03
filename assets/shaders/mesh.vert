#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out VS_OUT {
  vec3 worldPos;
  vec3 worldNormal;
} vs_out;

void main() {
  vec4 wpos = uModel * vec4(aPos, 1.0);
  vs_out.worldPos = wpos.xyz;
  vs_out.worldNormal = mat3(transpose(inverse(uModel))) * aNormal;

  gl_Position = uProj * uView * wpos;
}
