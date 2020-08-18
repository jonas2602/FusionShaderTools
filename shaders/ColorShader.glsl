// Space for Comments

#type vertex
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std140, set = 0, binding = 0) uniform CameraBuffer {
    mat4 View;
    mat4 Projection;
	mat4 ViewProjection;
} u_Camera;

layout(std140, set = 0, binding = 1) uniform ModelBuffer {
    mat4 Transform;
} u_Model;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;

layout(location = 0) out vec3 v_FragColor;

void main() {
    // gl_Position = vec4(inPosition, 0.0, 1.0);
    gl_Position = u_Camera.ViewProjection * u_Model.Transform * vec4(a_Position, 1.0);
    v_FragColor = a_Color;
}


#type fragment
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 v_FragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(v_FragColor, 1.0);
}