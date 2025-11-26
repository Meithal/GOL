#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "util_glfw.h"

static int WIDTH = 800;
static int HEIGHT = 600;
static float TIME_STEP = 0.1f;

// Gaussian kernel for convolution
float kernel[128][128]; // Example size; can be adjusted

// Function to initialize the kernel
void InitKernel(int size) {
    float sigma = size / 4.0f;
    float sum = 0.0f;

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int dx = i - size / 2;
            int dy = j - size / 2;
            kernel[i][j] = expf(-(dx * dx + dy * dy) / (2 * sigma * sigma));
            sum += kernel[i][j];
        }
    }

    // Normalize the kernel
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            kernel[i][j] /= sum;
        }
    }
}

// Convolution computation
void Convolve(
        int height, int width,
        float (*input)[height][width],
        float (*output)[height][width]
) {
    int kernel_size = sizeof(kernel) / sizeof(kernel[0]);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float sum = 0.0f;
            for (int ky = 0; ky < kernel_size; ++ky) {
                for (int kx = 0; kx < kernel_size; ++kx) {
                    int ny = (y + ky - kernel_size / 2 + height) % height;
                    int nx = (x + kx - kernel_size / 2 + width) % width;
                    sum += (*input)[ny][nx] * kernel[ky][kx];
                }
            }
            (*output)[y][x] = sum;
        }
    }
}

// Lenia's growth function
void ApplyGrowth(
        int height, int width,
        float (*input)[height][width],
        float (*output)[height][width]
) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float u = (*input)[y][x];
            float growth = expf(-powf((u - 0.5f) / 0.15f, 2));
            (*output)[y][x] = (*input)[y][x] + TIME_STEP * (growth - (*input)[y][x]);
        }
    }
}

// Launch function for Lenia
void LaunchLenia(unsigned char seed) {
    float (*pixelColors)[HEIGHT * WIDTH * 3] = malloc(sizeof(float[HEIGHT * WIDTH * 3]));
    float (*pixelData)[HEIGHT][WIDTH] = malloc(sizeof(float[HEIGHT][WIDTH]));
    float (*newPixelData)[HEIGHT][WIDTH] = malloc(sizeof(float[HEIGHT][WIDTH]));
    float (*convolvedData)[HEIGHT][WIDTH] = malloc(sizeof(float[HEIGHT][WIDTH]));

    if (pixelColors == NULL || pixelData == NULL || newPixelData == NULL || convolvedData == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    srand(seed);
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            (*pixelData)[y][x] = (float)(rand() % 100) / 100.0f; // Random initial state
        }
    }

    InitKernel(128); // Initialize the Gaussian kernel

    GLFWwindow *window = OpenWindow("Lenia", WIDTH, HEIGHT, false, true);
    GLuint program, VAO, VBO;
    GLint vpos_location, vcol_location;

    LoadShaders(&program, &vpos_location, &vcol_location);

    long offset = GeneratePixelData(HEIGHT, WIDTH, pixelColors, vpos_location, vcol_location, &VAO, &VBO);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        // Apply Lenia dynamics
        Convolve(HEIGHT, WIDTH, pixelData, convolvedData);
        ApplyGrowth(HEIGHT, WIDTH, convolvedData, newPixelData);

        int k = 0;
        for (int i = 0; i < HEIGHT * WIDTH; ++i) {
            float intensity = (*newPixelData)[i / WIDTH][i % WIDTH];
            (*pixelColors)[k++] = intensity; // R
            (*pixelColors)[k++] = intensity; // G
            (*pixelColors)[k++] = intensity; // B
        }

        memcpy(pixelData, newPixelData, sizeof(float[HEIGHT][WIDTH]));

        RenderPixels(WIDTH * HEIGHT, program, VAO, HEIGHT, WIDTH, offset, pixelColors);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(pixelColors);
    free(pixelData);
    free(newPixelData);
    free(convolvedData);

    glfwDestroyWindow(window);
    glfwTerminate();
}
