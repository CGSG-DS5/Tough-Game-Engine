/**/
#version 460

layout(location = 0) in vec3 VPosition;

layout(location = 0) out vec4 FinalColor;

void main() {
  FinalColor = vec4(VPosition, 1);
}