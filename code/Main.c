#include <glfw3.h>
#include "UIFramework.c"

#define WIDTH 640
#define HEIGHT 480



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    UISetDimension(width, height);
    //glViewport(0, 0, width, -height);
}

int pressed = 0;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        pressed = 1;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        pressed = 0;
    }
    
    
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        SubDivide(UIGetHoverWindow(), VERTICAL);
    }
    
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        SubDivide(UIGetHoverWindow(), HORIZONTAL);
    }
    
}

int main()
{
    
    //void* buff = malloc(100);
    
    //free(buff);
    
    //free(buff);
    
    
    GLFWwindow* glwindow;
    
    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /* Create a windowed mode window and its OpenGL context */
    glwindow= glfwCreateWindow(WIDTH, HEIGHT, "Hello World", NULL, NULL);
    if (!glwindow)
    {
        glfwTerminate();
        return -1;
    }
    
    
    glfwSetFramebufferSizeCallback(glwindow, framebuffer_size_callback);
    glfwSetMouseButtonCallback(glwindow, mouse_button_callback);
    glfwSetKeyCallback(glwindow, key_callback);
    int width, height;
    
    glfwGetWindowSize(glwindow, &width, &height);
    
    UISetDimension(width, height);
    UIInit();
    
    
    
    
    glfwMakeContextCurrent(glwindow);
    
    
    while (!glfwWindowShouldClose(glwindow))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        
        
        double xpos, ypos;
        
        glfwGetCursorPos(glwindow, &xpos, &ypos);
        
        UISetMouseState((int) xpos, (int) ypos, pressed);
        
        UIUpdate();
        UIRender();
        
        
        
        glfwSwapBuffers(glwindow);
        glfwPollEvents();
    }
    
    glfwTerminate();
    
    return 0;
}