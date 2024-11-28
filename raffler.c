//
// Created by ivo on 27/11/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define LOG_RES 7
#define FIELD_SIZE (1 << LOG_RES)
#define INNER_RADIUS 7.0
#define OUTER_RADIUS (3 * INNER_RADIUS)
#define B1 0.278
#define B2 0.365
#define D1 0.267
#define D2 0.445
#define ALPHA_N 0.028
#define ALPHA_M 0.147

// Buffers
float fields[2][FIELD_SIZE][FIELD_SIZE] = {0};
float imaginary_field[FIELD_SIZE][FIELD_SIZE] = {0};
float M_re[FIELD_SIZE][FIELD_SIZE], M_im[FIELD_SIZE][FIELD_SIZE];
float N_re[FIELD_SIZE][FIELD_SIZE], N_im[FIELD_SIZE][FIELD_SIZE];
float M_re_buffer[FIELD_SIZE][FIELD_SIZE], M_im_buffer[FIELD_SIZE][FIELD_SIZE];
float N_re_buffer[FIELD_SIZE][FIELD_SIZE], N_im_buffer[FIELD_SIZE][FIELD_SIZE];
int current_field = 0;

// Shader sources
const char* vertex_shader_src = "#version 330 core\n"
                                "layout(location = 0) in vec2 aPos;\n"
                                "void main() {\n"
                                "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
                                "}\n";

const char* fragment_shader_src = "#version 330 core\n"
                                  "out vec4 FragColor;\n"
                                  "uniform sampler2D uField;\n"
                                  "void main() {\n"
                                  "    float value = texture(uField, gl_FragCoord.xy / 128.0).r;\n"
                                  "    FragColor = vec4(value, value, value, 1.0);\n"
                                  "}\n";

// OpenGL utilities
void check_shader_compile(GLuint shader) {
    int success;
    char info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("Shader Compilation Error: %s\n", info_log);
    }
}

void check_program_link(GLuint program) {
    int success;
    char info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        printf("Program Linking Error: %s\n", info_log);
    } else {
        printf("Shader program linking success\n\n");
    }
}

// Simulation functions
float sigma(float x, float a, float alpha) {
    return 1.0f / (1.0f + expf(-4.0f / alpha * (x - a)));
}

float sigma_2(float x, float a, float b) {
    return sigma(x, a, ALPHA_N) * (1.0f - sigma(x, b, ALPHA_N));
}

float lerp(float a, float b, float t) {
    return (1.0f - t) * a + t * b;
}

float S(float n, float m) {
    float alive = sigma(m, 0.5f, ALPHA_M);
    return sigma_2(n, lerp(B1, D1, alive), lerp(B2, D2, alive));
}

// Initialize the field with random patterns
void initialize_field_with_life() {
    float (*cur_field)[FIELD_SIZE] = fields[current_field];

    // Add speckles of life to the field
    for (int i = 0; i < 200; ++i) {  // Number of speckles
        int u = rand() % (FIELD_SIZE - (int)INNER_RADIUS);
        int v = rand() % (FIELD_SIZE - (int)INNER_RADIUS);

        for (int x = 0; x < (int)INNER_RADIUS; ++x) {
            for (int y = 0; y < (int)INNER_RADIUS; ++y) {
                if ((u + x) < FIELD_SIZE && (v + y) < FIELD_SIZE) {
                    cur_field[u + x][v + y] = (float)rand() / RAND_MAX;  // Random intensity
                }
            }
        }
    }
}


void step() {
    float (*cur_field)[FIELD_SIZE] = fields[current_field];
    current_field = 1 - current_field;
    float (*next_field)[FIELD_SIZE] = fields[current_field];

    // Clear imaginary field
    for (int i = 0; i < FIELD_SIZE; ++i) {
        for (int j = 0; j < FIELD_SIZE; ++j) {
            imaginary_field[i][j] = 0.0f;
        }
    }

    // Compute m,n fields (fft2, etc., would be added here)

    // Step function
    for (int i = 0; i < FIELD_SIZE; ++i) {
        for (int j = 0; j < FIELD_SIZE; ++j) {
            next_field[i][j] = S(N_re_buffer[i][j], M_re_buffer[i][j]);
        }
    }
}

// OpenGL rendering
void render(GLFWwindow* window, GLuint shader_program, GLuint field_texture) {
    glClear(GL_COLOR_BUFFER_BIT);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, field_texture);
    glUseProgram(shader_program);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwSwapBuffers(window);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(512, 512, "SmoothLife", nullptr, nullptr);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    // Compile shaders
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_src, nullptr);
    glCompileShader(vertex_shader);
    check_shader_compile(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_src, nullptr);
    glCompileShader(fragment_shader);
    check_shader_compile(fragment_shader);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    check_program_link(shader_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Create texture
    GLuint field_texture;
    glGenTextures(1, &field_texture);
    glBindTexture(GL_TEXTURE_2D, field_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, FIELD_SIZE, FIELD_SIZE, 0, GL_RED, GL_FLOAT, fields[current_field]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    initialize_field_with_life();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        step();
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FIELD_SIZE, FIELD_SIZE, GL_RED, GL_FLOAT, fields[current_field]);
        render(window, shader_program, field_texture);
        glfwPollEvents();
    }

    // Clean up
    glDeleteTextures(1, &field_texture);
    glDeleteProgram(shader_program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
