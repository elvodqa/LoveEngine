#version 450
#extension GL_EXT_nonuniform_qualifier: enable
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUV;      // Interpolated UV from vertex shader
layout(location = 2) in vec3 fragNormal;      // Interpolated UV from vertex shader
layout(location = 3) in vec3 fragTangent;      // Interpolated UV from vertex shader

layout(location = 0) out vec4 outColor;   // Output fragment color

layout(set=0, binding = 0) uniform sampler samplers[10];
layout(set=0, binding = 1) uniform texture2D textures[];
layout(push_constant) uniform P {layout(offset = 128) int imageID;} push_constants;
vec3 light=vec3(1.f,0.f,2.f);
void main() {
    // Output the interpolated UV as color (normalized to [0, 1] in RGB space)
//    outColor = vec4(fragNormal,1.f);
//    outColor = vec4(fragUV*pow((0.1+max(0.f,dot(fragNormal,normalize(vec3(0,1,0.2))))),0.5f), 0.0, 1.0);

    vec4 color = texture(sampler2D(textures[push_constants.imageID],samplers[0]),fragUV);
    vec3 delta = light-fragPos;
    float L = 10;
    L *= max(0.,dot(fragNormal,normalize(delta)))/(dot(delta, delta));
    outColor = vec4(color.rbg*L,color.w);
}
