#version 450

layout(location = 0) in vec3 inPosition;  // Position attribute
layout(location = 1) in vec2 inUV;        // UV attribute

layout(push_constant) uniform PushConstants {
    mat4 viewProj;  // View-projection matrix
    mat4 model;     // Model matrix
} pushConstants;

layout(location = 0) out vec2 fragUV;    // Pass UV to fragment shader

void main() {
    // Transform the position using the model and view-projection matrices
    gl_Position = (pushConstants.viewProj * pushConstants.model) * vec4(inPosition, 1.f);

    // Pass the UV coordinates to the fragment shader
    fragUV = inUV;
}
