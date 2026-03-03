#version 410 core

in vec2 vUv;

uniform vec3 uSkyBottom;
uniform vec3 uSkyTop;

out vec4 FragColor;

void main() {
  float t = clamp(vUv.y, 0.0, 1.0);
  vec3 sky = mix(uSkyBottom, uSkyTop, t);
  FragColor = vec4(sky, 1.0);
}
