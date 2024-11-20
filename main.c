#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include "GLFW/glfw3.h"

/*
static const struct
{
    float x, y;
} vertices_pos[3] =
        {
                { -0.6f, -0.4f },
                {  0.6f, -0.4f },
                {   0.f,  0.6f }
        };

static const struct {
    float r, g, b;
} vertices_colors[] = {
        { 1.f, 0.f, 0.f },
        { 0.f, 1.f, 0.f },
        { 0.f, 0.f, 1.f }
};

int vertix_count = sizeof vertices_pos / sizeof vertices_pos[0];
*/
static int WIDTH = 640;
static int HEIGHT = 480;

/*
static const char* vertex_shader_text =
        "#version 110\n"
        "uniform mat4 MVP;\n"
        "attribute vec3 vCol;\n"
        "attribute vec2 vPos;\n"
        "varying vec3 color;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
        "    color = vCol;\n"
        "}\n";

static const char* fragment_shader_text =
        "#version 110\n"
        "varying vec3 color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(color, 1.0);\n"
        "}\n";
*/

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

static GLFWwindow* OpenWindow(void)
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    // essai avec gles
    // glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    // glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API); // Specify OpenGL ES
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Simple example", nullptr, nullptr);
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

    glfwSwapInterval(1);

    return window;
}

void LoadShaders(GLuint * program) {

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
            glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
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
            glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
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
        glGetProgramInfoLog(*program, 512, NULL, infoLog);
        fprintf(stderr, "ERROR: Shader linking failed\n%s\n", infoLog);
    } else {
        fputs("Shader linking succeed\n", stderr);
    }
}


// Define your pixels as points (positions in normalized coordinates, colors)
void GeneratePixelData(int count, float pixelColorData[static count], int height, int width, int pos_index, int col_index, GLuint *VAO) {

    float pixelPosData[count * 2];

    int j = 0;
    int k = 0;
    // Example: Add points with different colors and positions
    for (int i = 0; i < count; ++i) {
        float x = (float)(rand() % width) / (float)width * 2.0f - 1.0f;  // Random x in NDC
        float y = (float)(rand() % height) / (float)height * 2.0f - 1.0f; // Random y in NDC
        float r = (float)(rand() % 256) / 255.0f; // Random red color
        float g = (float)(rand() % 256) / 255.0f; // Random green color
        float b = (float)(rand() % 256) / 255.0f; // Random blue color

        pixelPosData[j++] =x;
        pixelPosData[j++] =y;
        pixelColorData[k++] =r;
        pixelColorData[k++] =g;
        pixelColorData[k++] =b;
    }
    GLuint buffers[2];
    glGenVertexArrays(1, VAO);
    glGenBuffers(2, buffers);
    GLuint VBO = buffers[0];

    glBindVertexArray(*VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBufferData(GL_ARRAY_BUFFER, size* sizeof(float), pixelData, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, (long)(sizeof pixelPosData + sizeof (float) * 3), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (long)(sizeof pixelPosData), pixelPosData);
    glBufferSubData(GL_ARRAY_BUFFER, (long)(sizeof pixelPosData), sizeof (float) * 3, pixelColorData);

    // Position attribute
    glVertexAttribPointer(pos_index, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(pos_index);

    // Color attribute
    glVertexAttribPointer(col_index, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof pixelPosData));
    glEnableVertexAttribArray(col_index);
}

void RenderPixels(int size, GLuint shaderProgram, GLuint VAO) {


    // Render points
    glUseProgram(shaderProgram);
    //glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, size);

    // Cleanup

    // glBindVertexArray(0);
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
}

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
             ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
             type, severity, message );
}


int main(void)
{
    // glEnable(GL_DEBUG_OUTPUT);
// During init, enable debug output

    GLuint program;
    GLint vpos_location, vcol_location;

    GLFWwindow* window = OpenWindow();

    //glEnable              ( GL_DEBUG_OUTPUT );
    //glDebugMessageCallback( MessageCallback, 0 );

    // NOTE: OpenGL error checks have been omitted for brevity

    //glGenBuffers(1, &vertex_buffer);
    //glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLfloat maxPointSize[2];
    glGetFloatv(GL_POINT_SIZE_RANGE, maxPointSize);
    printf("Maximum point size: %f, %f\n", maxPointSize[0], maxPointSize[1]);

    LoadShaders(&program);

    //mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    fprintf(stderr, "pos %d\ncolor %d\n", vpos_location, vcol_location);

    GLenum err = glGetError();

    fprintf(stderr, "erreur %d\n", err);

    GLuint VAO;


    /*
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), nullptr);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) (sizeof(float) * 2));
*/
    float (*pixelColors)[WIDTH*HEIGHT*3] = malloc(sizeof (float[WIDTH*HEIGHT*3]));
    if(pixelColors == NULL) {
        fprintf(stderr, "fail to generate color buffer on CPU\n");
        exit(EXIT_FAILURE);
    }
    GeneratePixelData(WIDTH*HEIGHT, pixelColors, HEIGHT, WIDTH, vpos_location, vcol_location, &VAO);

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        // mat4x4 m, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = (float) width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        /*
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);
         */

        glUseProgram(program);
        //glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        //glDrawArrays(GL_TRIANGLES, 0, vertix_count);
        RenderPixels(WIDTH*HEIGHT, program, VAO);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(pixelColors);
    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
