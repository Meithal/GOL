//
// Created by ivo on 27/11/2024.
//


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

// Shader source code
const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
out vec2 texCoord;
void main() {
    texCoord = aPos * 0.5 + 0.5; // Map [-1, 1] to [0, 1]
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 330 core
#extension GL_ARB_gpu_shader_fp64 : enable

in vec2 texCoord;
out vec4 FragColor;

uniform float zoom;
uniform vec2 offset;
uniform int maxIter;

void main() {
    // Map texture coordinates to the complex plane
    vec2 c = (texCoord - 0.5) * zoom - offset;
    vec2 z = vec2(0.0);
    int iter;
    for (iter = 0; iter < maxIter; iter++) {
        float x = z.x * z.x - z.y * z.y + c.x;
        float y = 2.0 * z.x * z.y + c.y;
        if (x * x + y * y > 4.0) break;
        z = vec2(x, y);
    }
    float color = float(iter) / float(maxIter);
    FragColor = vec4(color, color * 0.5, color * 0.25, 1.0); // Gradient coloring
}
)";

// Function to compile shaders
unsigned int compileShader(const char *source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
    }
    return shader;
}

// Initialize OpenGL and render the Mandelbrot set
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    // Set GLFW context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 800, "Mandelbrot Set", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    // Compile shaders
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    // Create shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Shader program linking error: %s\n", infoLog);
    } else {
        printf("Shader program linking success\n\n");
    }

    // Cleanup shaders (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Quad vertex data
    float vertices[] = {
            -1.0f, -1.0f, // Bottom-left
            1.0f, -1.0f, // Bottom-right
            -1.0f,  1.0f, // Top-left
            1.0f,  1.0f  // Top-right
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Render loop
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    float zoom = 2.0f;
    float offsetX = 0.0f, offsetY = 0.0f;
    int maxIter = 100;

    while (!glfwWindowShouldClose(window)) {
        // Handle user input
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) offsetY -= 0.05f * zoom;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) offsetY += 0.05f * zoom;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) offsetX += 0.05f * zoom;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) offsetX -= 0.05f * zoom;
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) zoom *= 0.9f;
        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) zoom /= 0.9f;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) maxIter -= 10;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) maxIter += 10;

        // Pass uniforms to the shader
        glUniform1f(glGetUniformLocation(shaderProgram, "zoom"), zoom);
        glUniform2f(glGetUniformLocation(shaderProgram, "offset"), offsetX, offsetY);
        glUniform1i(glGetUniformLocation(shaderProgram, "maxIter"), maxIter);

        // Render
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
