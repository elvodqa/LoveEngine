#version 450

layout(location = 0) in vec2 fragUV;      // Interpolated UV from vertex shader
layout(location = 1) in vec2 fragBaseNormal;

layout(location = 0) out vec4 outColor;   // Output fragment color

layout ()

void main() {
    // Output the interpolated UV as color (normalized to [0, 1] in RGB space)
    outColor = vec4(fragUV, 0.0, 1.0);
}
