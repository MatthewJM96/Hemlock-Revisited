#version 330

// Uniforms - things that are the same for all vertices.
uniform mat4 WorldProjection;
uniform mat4 ViewProjection;

// Data about this specific vertex (corresponds to our SpriteVertex class).
in vec4 vPosition;
in vec2 vRelativePosition;
in vec4 vUVDimensions;
in vec4 vColour;

// Data we want to send to be used for calculating colour of each pixel.
     out vec2 fRelativePosition;
flat out vec4 fUVDimensions;
     out vec4 fColour;

void main() {
    // Send data we aren't transforming straight to the fragment shader.
    fRelativePosition = vRelativePosition;
    fUVDimensions     = vUVDimensions;
    fColour           = vColour;

    // Calculate the position of this vertex on the screen.
    vec4 worldPosition = WorldProjection * vPosition;
    gl_Position = ViewProjection * worldPosition;
}
