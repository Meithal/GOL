#version 330 core

//uniform mat4 MVP;
uniform float u_yDecal;

in vec3 vCol;
in vec2 vPos;

out vec3 fragColor;

void main()
{
    gl_Position = /*MVP * */vec4(vPos, 0.0, 1.0);

    gl_PointSize = 1.0;
    fragColor = vCol;
}
