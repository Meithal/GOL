#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D currentState;
uniform vec2 texelSize;

void main() {
    vec4 state = texture(currentState, texCoords);

    // SmoothLife rules: Placeholder - average of neighbors
    vec4 neighbors = texture(currentState, texCoords + texelSize * vec2(-1, -1)) +
    texture(currentState, texCoords + texelSize * vec2(-1,  0)) +
    texture(currentState, texCoords + texelSize * vec2(-1,  1)) +
    texture(currentState, texCoords + texelSize * vec2( 0, -1)) +
    texture(currentState, texCoords + texelSize * vec2( 0,  1)) +
    texture(currentState, texCoords + texelSize * vec2( 1, -1)) +
    texture(currentState, texCoords + texelSize * vec2( 1,  0)) +
    texture(currentState, texCoords + texelSize * vec2( 1,  1));

    vec4 newState = state * 0.5 + neighbors / 8.0; // Simple average for demonstration
    FragColor = newState; // Replace with real SmoothLife logic
}
