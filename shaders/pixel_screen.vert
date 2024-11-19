#version 110

uniform mat4 MVP;
attribute vec3 vCol;
attribute vec2 vPos;
varying vec3 fragColor;

void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);

    gl_PointSize = 5.0;
    fragColor = vCol;
}
