//
// Created by ivo on 26/11/2024.
//

#ifndef GOL_UTIL_GLFW_H
#define GOL_UTIL_GLFW_H

char *
GetShaderSource_freeme(const char * filename, long * length);

GLFWwindow*
OpenWindow(const char * title, int width, int height, bool is_fullscreen, bool has_vertical_sync);

void
LoadShaders(GLuint * program, GLint * vpos_location, GLint * vcol_location);

int
GeneratePixelData(int height, int width, float (*pixelColorData)[height*width*3],
                      int pos_index, int col_index, GLuint *VAO, GLuint * VBO);
void
RenderPixels(int size, GLuint shaderProgram, GLuint VAO,
                  int height, int width, long offset, float (*pixelColorData)[height*width*3]);

#endif //GOL_UTIL_GLFW_H
