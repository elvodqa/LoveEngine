#version 450

layout(location = 0) in vec3 inPosition;  // Position attribute
layout(location = 1) in vec2 inUV;        // UV attribute
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(push_constant) uniform PushConstants {
    mat4 viewProj;  // View-projection matrix
    mat4 model;     // Model matrix
} pushConstants;

layout(location = 0) out vec3 fragPos;    // Pass UV to fragment shader
layout(location = 1) out vec2 fragUV;    // Pass UV to fragment shader
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outTangent;

void main() {
    // Transform the position using the model and view-projection matrices
    mat4 mvp = (pushConstants.viewProj * pushConstants.model);
    gl_Position = mvp * vec4(inPosition, 1.f);
    fragPos = ((pushConstants.model) * vec4(inPosition,1.f)).xyz;
    outNormal = ((pushConstants.model) * vec4(inNormal,0.f)).xyz;
    outTangent = (pushConstants.model *vec4(inTangent,0.0f)).xyz;
    // Pass the UV coordinates to the fragment shader
    fragUV = inUV;
}
