#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

static const struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
        {
                { -0.6f, -0.4f, 1.f, 0.f, 0.f },
                {  0.6f, -0.4f, 0.f, 1.f, 0.f },
                {   0.f,  0.6f, 0.f, 0.f, 1.f }
        };

int vertix_count = sizeof vertices / sizeof vertices[0];

static int WIDTH = 640;
static int HEIGHT = 480;


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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

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

        }

    }

    {
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

        long frag_size = getFileSize("shaders/simple.frag");
        char * fragment_shader_text = malloc(frag_size);
        FILE * f = fopen("shaders/pixel_screen.vert", "rb");
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
    }

}


// Define your pixels as points (positions in normalized coordinates, colors)
void GeneratePixelData(int count, float pixelData[count], int height, int width) {

    int j = 0;
    // Example: Add points with different colors and positions
    for (int i = 0; i < count; ++i) {
        float x = (rand() % width) / (float)width * 2.0f - 1.0f;  // Random x in NDC
        float y = (rand() % height) / (float)height * 2.0f - 1.0f; // Random y in NDC
        float r = (rand() % 256) / 255.0f; // Random red color
        float g = (rand() % 256) / 255.0f; // Random green color
        float b = (rand() % 256) / 255.0f; // Random blue color

        pixelData[j++] =x;
        pixelData[j++] =y;
        pixelData[j++] =r;
        pixelData[j++] =g;
        pixelData[j++] =b;
    }
}

void RenderPixels(int size, const float pixelData[size], GLuint shaderProgram) {
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size* sizeof(float), pixelData, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Render points
    glUseProgram(shaderProgram);
    glDrawArrays(GL_POINTS, 0, size / 5);

    // Cleanup
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

int main(void)
{
    GLuint vertex_buffer, program;
    GLint mvp_location, vpos_location, vcol_location;

    GLFWwindow* window = OpenWindow();

    // NOTE: OpenGL error checks have been omitted for brevity

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    LoadShaders(&program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), nullptr);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) (sizeof(float) * 2));

    float (*pixelColors)[WIDTH*HEIGHT*5] = malloc(sizeof (float[WIDTH*HEIGHT*3]));
    GeneratePixelData(WIDTH*HEIGHT, pixelColors, HEIGHT, WIDTH);

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = (float) width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        //glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        //glDrawArrays(GL_TRIANGLES, 0, vertix_count);
        RenderPixels(WIDTH*HEIGHT, pixelColors, program );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
