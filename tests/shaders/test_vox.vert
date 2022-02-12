#version 330 core

uniform mat4 model;
uniform mat4 view_proj;

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_colour;
layout(location = 2) in vec2 v_texture_coord;

out vec4 f_colour;
out vec2 f_texture_coord;

void main() {
    gl_Position = view_proj * model * vec4(v_position, 1.0f);

    f_colour = vec4(v_colour, 1.0f);
    f_texture_coord = v_texture_coord;
}
