#version 330 core

in vec3 fragColor;
in float heat;

out vec4 FragColor;

void main() {
    FragColor = vec4(fragColor, 1.0);

    //FragColor = vec4(1,0,0,1);
}
