//
// Created by ivo on 26/11/2024.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "util_glfw.h"

static int WIDTH = 800;
static int HEIGHT = 600;

static float MIN_PERP = 1.5f;
static float MAX_PERP = 2.9999f;
static float MIN_SPAWN = 2.05f;
static float MAX_SPAWN = 6.1f;

void InitSmoothworld(int height, int width, float (*pixelData)[height][width], unsigned char pattern) {
    srand(pattern);

    if(pattern == 0) {
        memset(*pixelData, 0, sizeof *pixelData);
        // glider
        (*pixelData)[0][0] = 1;
        (*pixelData)[1][1] = 1;
        (*pixelData)[1][2] = 1;
        (*pixelData)[2][0] = 1;
        (*pixelData)[2][1] = 1;
    } else {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; x++) {
                (*pixelData)[y][x] = rand() & 1;
            }
        }
    }
}

void IterateSmoothworld(
        int height, int width, float (*pixelColors)[height][width],
        float (*newPixelColors)[height][width], float (*rule)(float, float, float, float, float, float),
        const float min_perp, const float max_perp, const float min_spawn, const float max_spawn
        ) {

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; x++) {
            float moore = (*pixelColors)[(y + height -1)%height][(x+width-1)%width]
                        + (*pixelColors)[(y + height -1)%height][x]
                        + (*pixelColors)[(y + height -1)%height][(x+1)%width]

                        + (*pixelColors)[y][(x+width-1)%width] + (*pixelColors)[y][(x+1)%width]

                        + (*pixelColors)[(y+1)%height][(x+width-1)%width]
                        + (*pixelColors)[(y+1)%height][x]
                        + (*pixelColors)[(y+1)%height][(x+1)%width];
            (*newPixelColors)[y][x] = rule((*pixelColors)[y][x], moore, min_perp, max_perp, min_spawn, max_spawn);
        }
    }

    memcpy(pixelColors, newPixelColors, sizeof *newPixelColors);
}

static float conway_rule(float current_state, float moore_number,
                         const float min_perp, const float max_perp, const float min_spawn, const float max_spawn) {
    if (current_state && (moore_number >= min_perp && moore_number <= max_perp))
        return current_state;
    if (!current_state && (moore_number >= min_spawn && moore_number <= max_spawn))
        return 1;

    return 0;
}

void LaunchSmoothWorld(unsigned char seed)
{
    float (*pixelColors)[HEIGHT*WIDTH*3] = malloc(sizeof (float[HEIGHT*WIDTH*3]));
    float (*pixelData)[HEIGHT][WIDTH] = malloc(sizeof (float[HEIGHT][WIDTH]));
    float (*newPixelData)[HEIGHT][WIDTH] = malloc(sizeof (float[HEIGHT][WIDTH]));

    if(pixelColors == NULL || newPixelData == NULL || pixelData == NULL) {
        fprintf(stderr, "fail to generate color buffer on CPU\n");
        exit(EXIT_FAILURE);
    }

    InitSmoothworld(HEIGHT, WIDTH, pixelData, seed);

    GLFWwindow* window = OpenWindow("GOL", WIDTH, HEIGHT, false, false);
    GLuint program, VAO, VBO;
    GLint vpos_location, vcol_location;

    LoadShaders(&program, &vpos_location, &vcol_location);

    long offset = GeneratePixelData(HEIGHT, WIDTH, pixelColors, vpos_location, vcol_location, &VAO, &VBO);

    glClearColor(0x18/255.f, 0x18/255.f,0x18/255.f, 1);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);

        IterateSmoothworld(HEIGHT, WIDTH, pixelData, newPixelData, conway_rule,
                           MIN_PERP, MAX_PERP, MIN_SPAWN, MAX_SPAWN);
        int k = 0;
        for (int i = 0; i < HEIGHT*WIDTH; ++i) {
            float strength = (*pixelData)[i / HEIGHT][i % WIDTH];
            float min = 0x18/255.f;
            float r = min + strength;
            float g = min;
            float b = min;

            (*pixelColors)[k++] =r;
            (*pixelColors)[k++] =g;
            (*pixelColors)[k++] =b;
        }
        RenderPixels(WIDTH*HEIGHT, program, VAO, HEIGHT, WIDTH, offset, pixelColors);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(pixelColors);
    free(pixelData);
    free(newPixelData);

    glfwDestroyWindow(window);

    glfwTerminate();
}

