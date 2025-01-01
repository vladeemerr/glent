#include <GLFW/glfw3.h>

#define GLAD_GLES2_IMPLEMENTATION
#include <glad/gles2.h>

int main() {
    glfwInit();
    gladLoadGLES2(glfwGetProcAddress);
    glfwTerminate();
}
