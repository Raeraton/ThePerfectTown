#version 330 core

uniform vec2 middle;
uniform float scale;

layout (location = 0) in vec2 aPosition;
out vec3 Position;

void main()
{
    Position = vec3( (aPosition - middle) * scale, 0.0 ); // vec3( model * vec4(aPosition, 0.0, 1.0) );

    gl_Position = vec4( Position, 1.0 );

}