//
// Created by ivo on 26/11/2024.
//

#include <stdio.h>
#include <stdlib.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d : %s\n", error, description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

long getFileSize(const char *filename) {
    FILE *file = fopen(filename, "rb"); // Open the file in binary mode
    if (file == NULL) {
        perror("Error opening file");
        return -1; // Return an error code if the file can't be opened
    }

    fseek(file, 0, SEEK_END); // Move the file pointer to the end of the file
    long fileSize = ftell(file); // Get the position of the file pointer (size of the file)
    fclose(file); // Close the file

    return fileSize;
}

GLFWwindow* OpenWindow(const char * title, int width, int height, bool is_fullscreen, bool has_vertical_sync)
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    // essai avec gles
    //glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API); // Specify OpenGL ES
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    // Get the primary monitor
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor) {
        fprintf(stderr, "Failed to get primary monitor\n");
        glfwTerminate();
        return nullptr;
    }

    // Get the video mode of the primary monitor
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    if (!videoMode) {
        fprintf(stderr,"Failed to get video mode\n");
        glfwTerminate();
        return nullptr;
    }

    // Print monitor resolution
    fprintf(stderr, "Monitor resolution: %d x %d\n", videoMode->width, videoMode->height);

    window = glfwCreateWindow(width, height, title,
                              is_fullscreen ? glfwGetPrimaryMonitor(): nullptr, nullptr
    );
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);

    int version = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    if(version == 0) {
        fprintf(stderr, "Failed to initialize OpenGL context\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    } else {
        printf("Loaded OpenGL %d \n", version);//, GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
        printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    }

    glfwSwapInterval(has_vertical_sync ? 1 : 0);

    glEnable(GL_PROGRAM_POINT_SIZE);

    return window;
}

void LoadShaders(GLuint * program, GLint * vpos_location, GLint * vcol_location) {

    GLuint vertex_shader, fragment_shader;

    {
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        long vert_size = getFileSize("shaders/pixel_screen.vert");
        char *vertex_shader_text = malloc(vert_size);
        FILE *f = fopen("shaders/pixel_screen.vert", "rb");
        fread(vertex_shader_text, vert_size, 1, f);
        fclose(f);
        const char *sourceStrings[] = {vertex_shader_text};
        const int lengthStrings[] = {(GLint) vert_size};

        glShaderSource(vertex_shader, 1, sourceStrings, lengthStrings);
        glCompileShader(vertex_shader);

        free(vertex_shader_text);

        GLint compiled;
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
        if(!compiled) {
            char infoLog[512];
            glGetShaderInfoLog(vertex_shader, 512, nullptr, infoLog);
            fprintf(stderr, "ERROR: Vertex Shader compilation failed\n%s\n", infoLog);
        } else {
            fputs("Vertex Shader compilation succeed\n", stderr);
        }
    }

    {
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

        long frag_size = getFileSize("shaders/simple.frag");
        char * fragment_shader_text = malloc(frag_size);
        FILE * f = fopen("shaders/simple.frag", "rb");
        fread(fragment_shader_text, frag_size, 1, f);
        fclose(f);
        const char* sourceStrings[] = { fragment_shader_text };
        const int lengthStrings[] = { (GLint)frag_size };

        glShaderSource(fragment_shader, 1, sourceStrings, lengthStrings);
        glCompileShader(fragment_shader);

        free(fragment_shader_text);

        GLint compiled;
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
        if(!compiled) {
            char infoLog[512];
            glGetShaderInfoLog(fragment_shader, 512, nullptr, infoLog);
            fprintf(stderr, "ERROR: Fragment Shader compilation failed\n%s\n", infoLog);

        } else {
            fputs("Fragment Shader compilation succeed\n", stderr);

        }
    }

    *program = glCreateProgram();
    glAttachShader(*program, vertex_shader);
    glAttachShader(*program, fragment_shader);
    glLinkProgram(*program);

    int success;
    char infoLog[512];
    glGetProgramiv(*program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(*program, 512, nullptr, infoLog);
        fprintf(stderr, "ERROR: Shader linking failed\n%s\n", infoLog);
    } else {
        fputs("Shader linking succeed\n", stderr);
    }

    //mvp_location = glGetUniformLocation(program, "MVP");
    *vpos_location = glGetAttribLocation(*program, "vPos");
    *vcol_location = glGetAttribLocation(*program, "vCol");

    //fprintf(stderr, "pos %d\ncolor %d\n", vpos_location, vcol_location);

    GLenum err = glGetError();

    fprintf(stderr, "erreur %d\n", err);

}


// Define your pixels as points (positions in normalized coordinates, colors)
int GeneratePixelData(int height, int width, float (*pixelColorData)[height*width * 3],
                      int pos_index, int col_index, GLuint *VAO, GLuint * VBO) {

    float (*pixelPosData)[height*width * 2] = malloc(sizeof (float[height*width*2]));

    int j = 0;
    int k = 0;
    // Example: Add points with different colors and positions
    for (int i = 0; i < height*width; ++i) {
        float x = (float)(i % width) / (float)width * 2.0f - 1.0f;
        float y = (float)(int)(i / width) / (float)height * 2.0f - 1.0f;

        (*pixelPosData)[j++] =x;
        (*pixelPosData)[j++] =-y;
    }

    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);

    glBindVertexArray(*VAO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    //glBufferData(GL_ARRAY_BUFFER, size* sizeof(float), pixelData, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, (long)(sizeof *pixelPosData + sizeof *pixelColorData), nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (long)(sizeof *pixelPosData), pixelPosData);
    glBufferSubData(GL_ARRAY_BUFFER, (long)(sizeof *pixelPosData), (long)(sizeof *pixelColorData), pixelColorData);

    // Position attribute
    glVertexAttribPointer(pos_index, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(pos_index);

    // Color attribute
    glVertexAttribPointer(col_index, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof *pixelPosData));
    glEnableVertexAttribArray(col_index);

    free(pixelPosData);

    return sizeof *pixelPosData;
}

void RenderPixels(int size, GLuint shaderProgram, GLuint VAO,
                  int height, int width, long offset, float (*pixelColorData)[height*width*3]) {

    // Render points
    glBufferSubData(GL_ARRAY_BUFFER, offset, (long)(sizeof *pixelColorData), pixelColorData);

    glUseProgram(shaderProgram);
    glDrawArrays(GL_POINTS, 0, size);

    // Cleanup

    // glBindVertexArray(0);
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
}
