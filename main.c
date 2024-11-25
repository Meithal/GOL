#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static int WIDTH = 640;
static int HEIGHT = 320;


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

static GLFWwindow* OpenWindow(const char * title, bool is_fullscreen, bool has_vertical_sync)
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


    window = glfwCreateWindow(WIDTH, HEIGHT, title,
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
}


// Define your pixels as points (positions in normalized coordinates, colors)
void GeneratePixelData(int height, int width, int (*pixelStateData)[height][width],
                       int pos_index, int col_index, GLuint *VAO, GLuint * VBO) {

    float (*pixelPosData)[height*width * 2] = malloc(sizeof (float[height*width*2]));
    float (*pixelColorData)[height*width * 3] = malloc(sizeof (float[height*width*3]));

    int j = 0;
    int k = 0;
    // Example: Add points with different colors and positions
    for (int i = 0; i < height*width; ++i) {
        bool is_on = (*pixelStateData)[i / width][i % width];
        float x = (float)(i % width) / (float)width * 2.0f - 1.0f;
        float y = (float)(int)(i / width) / (float)height * 2.0f - 1.0f;
        float r = is_on ? 1.f : 0x18/255.f;
        float g = is_on ? 1.f : 0x18/255.f;
        float b = is_on ? 1.f : 0x18/255.f;

        (*pixelPosData)[j++] =x;
        (*pixelPosData)[j++] =-y;
        (*pixelColorData)[k++] =r;
        (*pixelColorData)[k++] =g;
        (*pixelColorData)[k++] =b;
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
}

void RenderPixels(int size, GLuint shaderProgram, GLuint VAO,
                  int height, int width, int (*pixelStateData)[height][width]) {

    float (*pixelPosData)[height*width * 2] = malloc(sizeof (float[height*width*2]));
    float (*pixelColorData)[height*width * 3] = malloc(sizeof (float[height*width*3]));

    int k = 0;
    for (int i = 0; i < height*width; ++i) {
        bool is_on = (*pixelStateData)[i / width][i % width];
        float r = is_on ? 1.f : 0x18/255.f;
        float g = is_on ? 1.f : 0x18/255.f;
        float b = is_on ? 1.f : 0x18/255.f;

        (*pixelColorData)[k++] =r;
        (*pixelColorData)[k++] =g;
        (*pixelColorData)[k++] =b;
    }

    // Render points
    glBufferSubData(GL_ARRAY_BUFFER, (long)(sizeof *pixelPosData), (long)(sizeof *pixelColorData), pixelColorData);


    glUseProgram(shaderProgram);
    //glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, size);

    // Cleanup

    free(pixelPosData);
    free(pixelColorData);

    // glBindVertexArray(0);
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
}

void IterateWolf(int height, int width, int (*pixelColors)[height][width], signed char pattern, int line)
{
    for (int x = 0; x < width; x++) {
        int previous = ((*pixelColors)[(height + line-1) % height][(width + x-1) % width] << 2)
                       + ((*pixelColors)[(height + line-1) % height][x] << 1) + ((*pixelColors)[(height + line-1) % height][(x+1) % width] << 0);
        assert(previous >= 0 && previous <= 7);

        int new_value = (_Bool )(pattern & ((1 << previous)));
        (*pixelColors)[line][x] = new_value;
    }
}

void InitWolf(int height, int width, int (*pixelColors)[height][width], signed char pattern) {
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

void LaunchWolfram(signed char seed) {
    GLuint program;
    GLint vpos_location, vcol_location;

    char title[200] = { 0 };
    sprintf(title, "Seed %d (%c%c%c%c%c%c%c%C)", seed
    , "01"[(_Bool)(seed & (1 << 7))]
    , "01"[(_Bool)(seed & (1 << 6))]
    , "01"[(_Bool)(seed & (1 << 5))]
    , "01"[(_Bool)(seed & (1 << 4))]
    , "01"[(_Bool)(seed & (1 << 3))]
    , "01"[(_Bool)(seed & (1 << 2))]
    , "01"[(_Bool)(seed & (1 << 1))]
    , "01"[(_Bool)(seed & (1 << 0))]
    );
    GLFWwindow* window = OpenWindow(title, false, true);

    glEnable(GL_PROGRAM_POINT_SIZE);
    //glEnable(GL_FRAMEBUFFER_SRGB);

    // NOTE: OpenGL error checks have been omitted for brevity

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

    GLuint VAO, VBO;

    int (*pixelColors)[HEIGHT][WIDTH] = malloc(sizeof (int[HEIGHT][WIDTH]));
    if(pixelColors == NULL) {
        fprintf(stderr, "fail to generate color buffer on CPU\n");
        exit(EXIT_FAILURE);
    }

    InitWolf(HEIGHT, WIDTH, pixelColors, seed);

    GeneratePixelData(HEIGHT, WIDTH, pixelColors, vpos_location, vcol_location, &VAO, &VBO);

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

        IterateWolf(HEIGHT, WIDTH, pixelColors, seed, line++);
        line %= HEIGHT;
        RenderPixels(WIDTH*HEIGHT, program, VAO, HEIGHT, WIDTH, pixelColors);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(pixelColors);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void LaunchConway(signed char seed)
{
    int (*pixelColors)[HEIGHT][WIDTH] = malloc(sizeof (int[HEIGHT][WIDTH]));
    if(pixelColors == NULL) {
        fprintf(stderr, "fail to generate color buffer on CPU\n");
        exit(EXIT_FAILURE);
    }

    //InitConway(HEIGHT, WIDTH, pixelColors, seed);


    GLFWwindow* window = OpenWindow("GOL", false, true);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);


        //IterateConway(HEIGHT, WIDTH, pixelColors, seed, line++);
        //RenderPixels(WIDTH*HEIGHT, program, VAO, HEIGHT, WIDTH, pixelColors);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(pixelColors);

    glfwDestroyWindow(window);

    glfwTerminate();
}

int main(void)
{
    //LaunchConway(90);
    LaunchWolfram(90);

    exit(EXIT_SUCCESS);
}
