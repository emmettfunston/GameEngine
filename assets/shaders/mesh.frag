#version 410 core

in VS_OUT {
  vec3 worldPos;
  vec3 worldNormal;
} fs_in;

uniform vec3 uViewPos;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAlbedo;
uniform vec3 uFogColor;
uniform float uFogNear;
uniform float uFogFar;
uniform int uEnableGroundGrid;

out vec4 FragColor;

float gridMask(vec2 worldXZ, float scale, float width) {
  vec2 coord = worldXZ / scale;
  vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
  float line = min(grid.x, grid.y);
  return 1.0 - min(line, width) / width;
}

void main() {
  vec3 N = normalize(fs_in.worldNormal);
  vec3 V = normalize(uViewPos - fs_in.worldPos);
  vec3 L = normalize(-uLightDir);

  float NdotL = max(dot(N, L), 0.0);

  vec3 baseColor = uAlbedo;
  if (uEnableGroundGrid == 1) {
    float minor = gridMask(fs_in.worldPos.xz, 1.0, 1.25);
    float major = gridMask(fs_in.worldPos.xz, 5.0, 1.5);
    float axisX = smoothstep(0.05, 0.0, abs(fs_in.worldPos.z));
    float axisZ = smoothstep(0.05, 0.0, abs(fs_in.worldPos.x));

    vec3 gridTint = mix(vec3(0.0), vec3(0.08, 0.11, 0.14), minor * 0.55 + major * 0.8);
    vec3 axisTint = vec3(0.92, 0.22, 0.16) * axisX + vec3(0.16, 0.58, 0.94) * axisZ;
    baseColor = baseColor + gridTint + axisTint;
  }

  vec3 ambient = 0.10 * baseColor;
  vec3 diffuse = NdotL * baseColor * uLightColor;

  vec3 H = normalize(L + V);
  float spec = pow(max(dot(N, H), 0.0), 48.0);
  vec3 specular = spec * uLightColor * 0.30;

  vec3 lit = ambient + diffuse + specular;

  float dist = length(uViewPos - fs_in.worldPos);
  float fogFactor = clamp((uFogFar - dist) / (uFogFar - uFogNear), 0.0, 1.0);
  vec3 color = mix(uFogColor, lit, fogFactor);

  FragColor = vec4(color, 1.0);
}
