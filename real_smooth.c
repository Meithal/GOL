#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <float.h>

#include "util_glfw.h"

#define WIDTH 512
#define HEIGHT 512

// Quad vertices for rendering
float vertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f,
};

// Function to compile shaders
GLuint compileShader(const char *source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader Compilation Error:\n%s\n", infoLog);
    }

    return shader;
}

// Function to link a program
GLuint linkProgram(const char *vertexSource, const char *fragmentSource) {
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "Program Linking Error:\n%s\n", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Initialize a glider-like pattern
void initGlider(float *data, int width, int height) {
    for (int i = 0; i < width * height * 4; i++) data[i] = 0.0f;

    int cx = width / 2, cy = height / 2;
    data[(cy * width + cx) * 4 + 0] = 1.0f; // Example glider pattern
    data[((cy - 1) * width + cx) * 4 + 0] = 1.0f;
    data[(cy * width + cx + 1) * 4 + 0] = 1.0f;
    data[((cy + 1) * width + cx - 1) * 4 + 0] = 1.0f;
    data[((cy + 1) * width + cx) * 4 + 0] = 1.0f;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "SmoothLife", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    // Load shaders
    long length;
    char *vertexSource = GetShaderSource_freeme("shaders/vertex_to_tex.vert", &length); // Add vertex shader source
    char *updateSource = GetShaderSource_freeme("shaders/smoothlife_update.frag", &length); // Add fragment shader for SmoothLife logic
    char *renderSource = GetShaderSource_freeme("shaders/texture_render.frag", &length); // Add render shader source

    GLuint updateProgram = linkProgram(vertexSource, updateSource);
    GLuint renderProgram = linkProgram(vertexSource, renderSource);

    free(vertexSource);
    free(updateSource);
    free(renderSource);

    // Generate textures and framebuffers
    GLuint textures[2];
    glGenTextures(2, textures);

    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    GLuint framebuffers[2];
    glGenFramebuffers(2, framebuffers);

    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);
    }

    // Initialize grid with a glider
    float *initialState = (float *)malloc(WIDTH * HEIGHT * 4 * sizeof(float));
    initGlider(initialState, WIDTH, HEIGHT);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGBA, GL_FLOAT, initialState);
    free(initialState);

    int current = 0;
    while (!glfwWindowShouldClose(window)) {
        // Update step
        glUseProgram(updateProgram);
        glUniform2f(glGetUniformLocation(updateProgram, "texelSize"), 1.0f / WIDTH, 1.0f / HEIGHT);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1 - current]);
        glBindTexture(GL_TEXTURE_2D, textures[current]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Render step
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(renderProgram);
        glBindTexture(GL_TEXTURE_2D, textures[1 - current]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Swap
        glfwSwapBuffers(window);
        glfwPollEvents();
        current = 1 - current;
    }

    glfwTerminate();
    return 0;
}
