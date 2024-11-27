#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "util_glfw.h"
#include "smoothlife.h"
#include "lenia.h"

static int WIDTH = 1920;
static int HEIGHT = 1080;


void IterateWolf(int height, int width, int (*pixelColors)[height][width], unsigned char pattern, int line)
{
    for (int x = 0; x < width; x++) {
        int previous = ((*pixelColors)[(height + line-1) % height][(width + x-1) % width] << 2)
                       + ((*pixelColors)[(height + line-1) % height][x] << 1) + ((*pixelColors)[(height + line-1) % height][(x+1) % width] << 0);
        assert(previous >= 0 && previous <= 7);

        int new_value = (_Bool )(pattern & ((1 << previous)));
        (*pixelColors)[line][x] = new_value;
    }
}

void InitWolf(int height, int width, int (*pixelColors)[height][width], unsigned char pattern) {
    srand(42);
    for (int i = 0; i < width; i++) {
        //(*pixelColors)[0][i] = rand() & 1;
        (*pixelColors)[0][i] = 0;
    }
    (*pixelColors)[0][width/2] = 1;

    for (int y = 1; y < height; ++y) {
        IterateWolf(height, width, pixelColors, pattern, y);
    }
}

void LaunchWolfram(unsigned char seed) {
    GLuint program;
    GLint vpos_location, vcol_location;

    char title[200] = { 0 };
    sprintf(title, "Seed %u (%c%c%c%c%c%c%c%C)", seed
    , "01"[(_Bool)(seed & (1 << 7))]
    , "01"[(_Bool)(seed & (1 << 6))]
    , "01"[(_Bool)(seed & (1 << 5))]
    , "01"[(_Bool)(seed & (1 << 4))]
    , "01"[(_Bool)(seed & (1 << 3))]
    , "01"[(_Bool)(seed & (1 << 2))]
    , "01"[(_Bool)(seed & (1 << 1))]
    , "01"[(_Bool)(seed & (1 << 0))]
    );
    GLFWwindow* window = OpenWindow(title, WIDTH, HEIGHT, true, false);

    //glEnable(GL_FRAMEBUFFER_SRGB);

    // NOTE: OpenGL error checks have been omitted for brevity

    GLfloat maxPointSize[2];
    glGetFloatv(GL_POINT_SIZE_RANGE, maxPointSize);
    printf("Maximum point size: %f, %f\n", maxPointSize[0], maxPointSize[1]);

    LoadShaders(&program, &vpos_location, &vcol_location);

    GLuint VAO, VBO;

    int (*pixelData)[HEIGHT][WIDTH] = malloc(sizeof (int[HEIGHT][WIDTH]));
    float (*pixelColors)[HEIGHT*WIDTH*3] = malloc(sizeof (float[HEIGHT*WIDTH*3]));
    if(pixelData == NULL || pixelColors == NULL) {
        fprintf(stderr, "fail to generate color buffer on CPU\n");
        exit(EXIT_FAILURE);
    }

    InitWolf(HEIGHT, WIDTH, pixelData, seed);

    int k = 0;
    for (int i = 0; i < HEIGHT*WIDTH; ++i) {
        bool is_on = (*pixelData)[i / WIDTH][i % WIDTH];
        float r = is_on ? 1.f : 0x18/255.f;
        float g = is_on ? 1.f : 0x18/255.f;
        float b = is_on ? 1.f : 0x18/255.f;

        (*pixelColors)[k++] =r;
        (*pixelColors)[k++] =g;
        (*pixelColors)[k++] =b;
    }
    int offset = GeneratePixelData(HEIGHT, WIDTH, pixelColors, vpos_location, vcol_location, &VAO, &VBO);

    glClearColor(0x18/255.f, 0x18/255.f,0x18/255.f, 1);

    int line = 0;
    while (!glfwWindowShouldClose(window))
    {
        //float ratio;
        int width, height;
        // mat4x4 m, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        //ratio = (float) width / (float) height;

        // resize if window is resized
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        /*
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);
         */

        //glUseProgram(program);
        //glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        //glDrawArrays(GL_TRIANGLES, 0, vertix_count);

        IterateWolf(HEIGHT, WIDTH, pixelData, seed, line++);
        line %= HEIGHT;

        int k = 0;
        for (int i = 0; i < HEIGHT*WIDTH; ++i) {
            bool is_on = (*pixelData)[i / WIDTH][i % WIDTH];
            float r = is_on ? 1.f : 0x18/255.f;
            float g = is_on ? 1.f : 0x18/255.f;
            float b = is_on ? 1.f : 0x18/255.f;

            (*pixelColors)[k++] =r;
            (*pixelColors)[k++] =g;
            (*pixelColors)[k++] =b;
        }
        RenderPixels(WIDTH*HEIGHT, program, VAO, HEIGHT, WIDTH, offset, pixelColors);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(pixelData);
    free(pixelColors);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void InitConway(int height, int width, int (*pixelData)[height][width], signed char pattern) {
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

void IterateConway(
        int height, int width, int (*pixelColors)[height][width],
        int (*newPixelColors)[height][width], int (*rule)(int, int)) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; x++) {
            int moore = (*pixelColors)[(y + height -1)%height][(x+width-1)%width]
                    + (*pixelColors)[(y + height -1)%height][x]
                    + (*pixelColors)[(y + height -1)%height][(x+1)%width]

                    + (*pixelColors)[y][(x+width-1)%width] + (*pixelColors)[y][(x+1)%width]

                    + (*pixelColors)[(y+1)%height][(x+width-1)%width]
                    + (*pixelColors)[(y+1)%height][x]
                    + (*pixelColors)[(y+1)%height][(x+1)%width];
            (*newPixelColors)[y][x] = rule((*pixelColors)[y][x], moore);
        }
    }

    memcpy(pixelColors, newPixelColors, sizeof *newPixelColors);
}

static int conway_rule(int current_state, int moore_number) {
    if (current_state && (moore_number == 2 || moore_number == 3))
        return 1;
    if (!current_state && (moore_number == 3))
        return 1;
    return 0;
}

void LaunchConway(signed char seed)
{
    float (*pixelColors)[HEIGHT*WIDTH*3] = malloc(sizeof (float[HEIGHT*WIDTH*3]));
    int (*pixelData)[HEIGHT][WIDTH] = malloc(sizeof (int[HEIGHT][WIDTH]));
    int (*newPixelData)[HEIGHT][WIDTH] = malloc(sizeof (int[HEIGHT][WIDTH]));

    if(pixelColors == NULL || newPixelData == NULL || pixelData == NULL) {
        fprintf(stderr, "fail to generate color buffer on CPU\n");
        exit(EXIT_FAILURE);
    }

    InitConway(HEIGHT, WIDTH, pixelData, seed);

    GLFWwindow* window = OpenWindow("GOL", WIDTH, HEIGHT, true, true);

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

        IterateConway(HEIGHT, WIDTH, pixelData, newPixelData, conway_rule);
        int k = 0;
        for (int i = 0; i < HEIGHT*WIDTH; ++i) {
            bool is_on = (*pixelData)[i / HEIGHT][i % WIDTH];
            float r = is_on ? 1.f : 0x18/255.f;
            float g = is_on ? 1.f : 0x18/255.f;
            float b = is_on ? 1.f : 0x18/255.f;

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

int main(void)
{

    //LaunchConway(90);
//    for(unsigned char i = 0; i< 255; i++)
//        LaunchConway(i);
    //for(unsigned char i = 0; i< 255; i++)
    //    LaunchWolfram(i);
    //LaunchWolfram(90);

    LaunchSmoothWorld(90);


    LaunchLenia(90);
    exit(EXIT_SUCCESS);
}
