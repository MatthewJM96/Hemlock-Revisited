#version 330 core

in vec4 f_colour;
in vec2 f_texture_coord;

// Ouput data
out vec4 final_colour;

uniform sampler2D tex;

void main() {
    final_colour = texture(tex, f_texture_coord) * f_colour;
}
