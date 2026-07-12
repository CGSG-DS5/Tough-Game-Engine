/**/
#version 460

layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) out vec3 VPosition;

layout(push_constant) uniform Constants {
  float time;
};

vec2 rotate(vec2 v) {
  return vec2(v.x * cos(time) - v.y * sin(time), v.x * sin(time) + v.y * cos(time));
}

vec2 rotate(float x, float y) {
  return rotate(vec2(x, y));
}

void main() {
  gl_Position = vec4(rotate(0, -0.5), 0, 1);
  VPosition = vec3(1, 0, 0);
  EmitVertex();
  gl_Position = vec4(rotate(-0.5, 0.5), 0, 1);
  VPosition = vec3(0, 1, 0);
  EmitVertex();
  gl_Position = vec4(rotate(0.5, 0.5), 0, 1);
  VPosition = vec3(0, 0, 1);
  EmitVertex();
  EndPrimitive();
}
