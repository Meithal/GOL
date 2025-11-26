#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <time.h>

#include "util_glfw.h"
#include "smoothlife.h"
#include "lenia.h"
#include "rafler.h"

//static int WIDTH = 1680;
//static int HEIGHT = 1050;
static int WIDTH = 640;
static int HEIGHT = 480;

struct cellState {
    float value;
    float heat;
};

void ConvertDataToColors(int height, int width, float const (*pixelData)[height][width][2], float (*pixelColors)[HEIGHT*WIDTH*3])
{
    int k = 0;
    for (int i = 0; i < HEIGHT*WIDTH; ++i) {
        int const y = i / WIDTH;
        int const x = i % WIDTH;
        bool const is_on = (*pixelData)[y][x][0] > FLT_EPSILON;
        float const heat = (*pixelData)[y][x][1];
        float const r = is_on ? 1 : 0x18/255.f;
        float const g = is_on ? 0.5f + (1-heat) * 0.5f : 0x18/255.f;
        float const b = is_on ? 0.5f + (1-heat) * 0.5f : 0x18/255.f;

        (*pixelColors)[k++] =r;
        (*pixelColors)[k++] =g;
        (*pixelColors)[k++] =b;
    }
}

void IterateWolf(int const height, int const width, float (*pixelColors)[height][width][2], unsigned char const pattern, int const line)
{
    for (int x = 0; x < width; x++) {
        int const previous = ((int)(*pixelColors)[(height + line-1) % height][(width + x-1) % width][0] << 2)
                       + ((int)(*pixelColors)[(height + line-1) % height][x][0] << 1)
                       + ((int)(*pixelColors)[(height + line-1) % height][(x+1) % width][0] << 0);
        assert(previous >= 0 && previous <= 7);

        (*pixelColors)[line][x][0] = (_Bool )(pattern & (1 << previous));
    }
}

void InitWolf(int height, int width, float (*pixelColors)[height][width][2], unsigned char pattern) {
    srand(42);
    for (int i = 0; i < width; i++) {
        //(*pixelColors)[0][i] = rand() & 1;
        (*pixelColors)[0][i][0] = 0;
        (*pixelColors)[0][i][1] = 0;
    }
    (*pixelColors)[0][width/2][0] = 1;
    (*pixelColors)[0][width/2][1] = 1;

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
    GLFWwindow* window = OpenWindow(title, WIDTH, HEIGHT, false, true);

    //glEnable(GL_FRAMEBUFFER_SRGB);

    // NOTE: OpenGL error checks have been omitted for brevity

    GLfloat maxPointSize[2];
    glGetFloatv(GL_POINT_SIZE_RANGE, maxPointSize);
    printf("Maximum point size: %f, %f\n", maxPointSize[0], maxPointSize[1]);

    LoadShaders(&program, &vpos_location, &vcol_location);

    GLuint VAO, VBO;

    float (*pixelData)[HEIGHT][WIDTH][2] = malloc(sizeof (float[HEIGHT][WIDTH][2]));
    float (*pixelColors)[HEIGHT*WIDTH*3] = malloc(sizeof (float[HEIGHT*WIDTH*3]));
    if(pixelData == NULL || pixelColors == NULL) {
        fprintf(stderr, "fail to generate color buffer on CPU\n");
        exit(EXIT_FAILURE);
    }

    InitWolf(HEIGHT, WIDTH, pixelData, seed);

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

void InitConway(int const height, int const width, float (*pixelData)[height][width][2], signed char const pattern) {
    srand(pattern);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; x++) {
            (*pixelData)[y][x][0] = (float)(rand() & 1);
            (*pixelData)[y][x][1] = 1;
        }
    }
}

void IterateConway(
        int const height, int const width, float (*pixelColors)[height][width][2],
        float (*newPixelColors)[height][width][2], struct cellState(*rule)(float, float, float)) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; x++) {
            float const moore = (*pixelColors)[(y + height -1)%height][(x+width-1)%width][0]
            + (*pixelColors)[(y + height -1)%height][x][0]
            + (*pixelColors)[(y + height -1)%height][(x+1)%width][0]

            + (*pixelColors)[y][(x+width-1)%width][0]
            + (*pixelColors)[y][(x+1)%width][0]

            + (*pixelColors)[(y+1)%height][(x+width-1)%width][0]
            + (*pixelColors)[(y+1)%height][x][0]
            + (*pixelColors)[(y+1)%height][(x+1)%width][0];

            struct cellState const s = rule((*pixelColors)[y][x][0], (*pixelColors)[y][x][1], moore);
            (*newPixelColors)[y][x][0] = s.value;
            (*newPixelColors)[y][x][1] = s.heat;
        }
    }

   // memcpy(pixelColors, newPixelColors, sizeof *newPixelColors);
}

static struct cellState conway_rule(float const current_state, float const heat, float const moore_number) {
    if (current_state && (moore_number == 2 || moore_number == 3))
        return (struct cellState){1, heat * .99f};
    if (!current_state && (moore_number == 3))
        return (struct cellState){1, 1};
    return (struct cellState){0, 0};
}

void LaunchConway(signed char seed)
{
    // passé en ur a openGL, du coup tableau flat
    float (*pixelColors)[HEIGHT*WIDTH*3] = malloc(sizeof *pixelColors);
    float (*pixelData)[HEIGHT][WIDTH][2] = malloc(sizeof *pixelData);
    float (*newPixelData)[HEIGHT][WIDTH][2] = malloc(sizeof *newPixelData);

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

    clock_t start_clock = clock();
    printf("clock start %lu\n", start_clock);

    int iterations = 0;

    while (!glfwWindowShouldClose(window))
    {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);

        IterateConway(HEIGHT, WIDTH, pixelData, newPixelData, conway_rule);
        ConvertDataToColors(HEIGHT, WIDTH, pixelData, pixelColors);
        RenderPixels(WIDTH*HEIGHT, program, VAO, HEIGHT, WIDTH, offset, pixelColors);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // intervertit les buffers
        void*tmp = pixelData;
        pixelData = newPixelData;
        newPixelData = tmp;

        clock_t const current_clock = clock();
        if ((double)(current_clock - start_clock) / CLOCKS_PER_SEC >= 1)
        {
            printf("FPS: %d\n", iterations);
            iterations = 0;
            start_clock = current_clock;
        }
        iterations++;
    }

    free(pixelColors);
    free(pixelData);
    free(newPixelData);

    glfwDestroyWindow(window);

    glfwTerminate();
}

int main(void)
{

    //for(unsigned char i = 0; i< 255; i++)
    //    LaunchWolfram(i);
    //for(unsigned char i = 160; i< 255; i++)
    //    LaunchWolfram(i);
    //LaunchWolfram(30);
    LaunchConway(90);
    //LaunchWolfram(135);
    //LaunchWolfram(169); // croissant

    //LaunchSmoothWorld(90);

    //LaunchRafler();


    //LaunchLenia(90);
    exit(EXIT_SUCCESS);
}
