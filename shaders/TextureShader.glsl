// Space for Comments
  
#type vertex
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std140, set = 0, binding = 0) uniform CameraBuffer {
    mat4 View;
    mat4 Projection;
	mat4 ViewProjection;
} u_Camera;

layout(std140, set = 0, binding = 2) uniform ModelBuffer {
    mat4 Transform;
} u_Model;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_FragColor;
layout(location = 1) out vec2 v_FragTexCoord;

void main() {
    gl_Position = u_Camera.ViewProjection * u_Model.Transform * vec4(a_Position, 1.0);
    v_FragColor = a_Color;
    v_FragTexCoord = a_TexCoord;
}

#type fragment
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 3) uniform sampler2D u_TexSampler;
layout(set = 1, binding = 1) uniform sampler2D u_TexSampler1;
layout(set = 2, binding = 2) uniform sampler2D u_TexSampler2;

layout(location = 0) in vec3 v_FragColor;
layout(location = 1) in vec2 v_FragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(texture(u_TexSampler, v_FragTexCoord).xyz, 1.0);
}