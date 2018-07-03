// TODO(Suhaib): Render using textures instead of copying again to buffer and rendering that buffer, perf based so late in the game

// TODO(Suhaib): Ingotduce dirty and clean rectangle

#define GLEW_STATIC

#include <glew.h>
#include <glfw3.h>
GLFWwindow* glwindow;


#include "UIFramework.c"

#define WIDTH 640
#define HEIGHT 480

SUIEnvironment* environment;


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    SUISetDimension(environment, width, height);
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
        __SUISubDivide(SUIGetHoverWindow(environment), VERTICAL);
    }
    
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        __SUISubDivide(SUIGetHoverWindow(environment), HORIZONTAL);
    }
    
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        __SUIDelete(environment, SUIGetHoverWindow(environment));
    }
    
}


void BottomSUIRender(SUIElement* element)
{
    uint32* buffer = element->buffer.buffer;
    int32 width = element->buffer.width;
    int32 height = element->buffer.height;
    
    uint32 r = 255;
    uint32 b = 0;
    uint32 g = 0;
    
    for (int y = 0; y < height; y++)
    {
        b = 0;
        for (int x = 0; x < width; x++)
        {
            buffer[x + y * width] = (b | (g << 8) | (r << 16));
            b = (b + 1) % 256;
        }
        g = (g + 1) % 256;
    }
}

void LeftSUIRender(SUIElement* element)
{
    uint32* buffer = element->buffer.buffer;
    int32 width = element->buffer.width;
    int32 height = element->buffer.height;
    
    uint32 r = 0;
    uint32 b = 255;
    uint32 g = 0;
    
    for (int y = 0; y < height; y++)
    {
        r = 0;
        for (int x = 0; x < width; x++)
        {
            buffer[x + y * width] = (b | (g << 8) | (r << 16));
            r = (r + 1) % 256;
        }
        g = (g + 1) % 256;
    }
}

void RightSUIRender(SUIElement* element)
{
    uint32* buffer = element->buffer.buffer;
    int32 width = element->buffer.width;
    int32 height = element->buffer.height;
    
    uint32 r = 0;
    uint32 b = 0;
    uint32 g = 255;
    
    for (int y = 0; y < height; y++)
    {
        r = 0;
        for (int x = 0; x < width; x++)
        {
            buffer[x + y * width] = (b | (g << 8) | (r << 16));
            r = (r + 1) % 256;
        }
        b = (b + 1) % 256;
    }
}

int main()
{
    if (!glfwInit())
        return -1;
    
    /* Create a windowed mode window and its OpenGL context */
    glwindow= glfwCreateWindow(WIDTH, HEIGHT, "Hello World", NULL, NULL);
    if (!glwindow)
    {
        printf("GLFW Problem\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(glwindow);
    
    
    if (glewInit() != GLEW_OK)
    {
        printf("GLEW Problem\n");
        glfwTerminate();
        return -1;
    }
    
    glfwSetFramebufferSizeCallback(glwindow, framebuffer_size_callback);
    glfwSetMouseButtonCallback(glwindow, mouse_button_callback);
    glfwSetKeyCallback(glwindow, key_callback);
    int width, height;
    
    glfwGetWindowSize(glwindow, &width, &height);
    
    environment = SUIInit();
    
    /*bottom element*/
    SUIElement bottomUIElement = *SUIGetValidUIElement();//(SUIElement) {0};
    
    bottomUIElement.SUIRenderCallback = BottomSUIRender;
    
    /*Left Side*/
    SUIElement leftUIElement = *SUIGetValidUIElement();//(SUIElement) {0};
    
    leftUIElement.SUIRenderCallback = LeftSUIRender;
    
    /*Right Side*/
    SUIElement rightUIElement = *SUIGetValidUIElement();//(SUIElement) {0};
    
    rightUIElement.SUIRenderCallback = RightSUIRender;
    
    SUIContainer* completeUI = SUIGetRoot(environment);
    
    SUIContainer* hero;
    SUIContainer* temp;
    
    SUIOpen(environment, &bottomUIElement);
    SUIDock(completeUI, &bottomUIElement, DOCK_BOTTOM, &hero, &temp); 
    
    SUIContainer* rightElement;
    
    SUIOpen(environment, &leftUIElement);
    SUIDock(hero, &leftUIElement, DOCK_LEFT, &temp, &rightElement);
    
    SUIOpen(environment, &rightUIElement);
    SUIDock(rightElement, &rightUIElement, DOCK_COMPLETE, &temp, &temp);
    
    
    SUISetDimension(environment, width, height);
    
    
    while (!glfwWindowShouldClose(glwindow))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        
        
        double xpos, ypos;
        
        glfwGetCursorPos(glwindow, &xpos, &ypos);
        
        SUISetMouseState(environment, (int) xpos, (int) ypos, pressed);
        
        SUIUpdate(environment);
        SUIRender(environment);
        
        
        
        glfwSwapBuffers(glwindow);
        glfwPollEvents();
        
    }
    
    glfwTerminate();
    
    
    return 0;
}