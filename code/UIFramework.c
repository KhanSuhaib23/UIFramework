#include <stdio.h>
#include <malloc.h>


#include "StandardDefinitions.h"

#define LINE_WIDTH 5

#define F2P(ratio, dim) ((ratio) * (dim))
#define P2F(pixel, dim) ((float32) (pixel) / (float32) (dim))

typedef struct SUIElement SUIElement;

typedef enum SUIAlignment
{
    NONE,
    VERTICAL,
    HORIZONTAL
    
} SUIAlignment;

typedef struct SUIDividingLine
{
    float32 start, end;
    float32 otherCood;
    float32 width;
    
} SUIDividingLine;

typedef enum SUIChildDirection
{
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
    
} SUIChildDirection;

typedef struct SUIBuffer
{
    
    int32 width;
    int32 height;
    void* buffer;
    
} SUIBuffer;

typedef struct SUIContainer SUIContainer;

typedef struct SUIElementVertex
{
    float x, y;
    float u, v;
    uint32 texID;
    
} SUIElementVertex;

typedef struct SUILineVertex
{
    float x, y;
    float r, g, b, a;
    
} SUILineVertex;

struct SUIContainer
{
    float32 x, y;
    float32 width;
    float32 height;
    float32 ratioOfParent;
    SUIAlignment split;
    SUIDividingLine line;
    SUIElement* element;
    SUIContainer* first;
    SUIContainer* second;
    SUIContainer* parent;
    
    
};

typedef void (SUIRenderCallbackFunction(SUIElement*));


// TODO(Suhaib): Rename this 
struct SUIElement
{
    SUIBuffer buffer;
    SUIRenderCallbackFunction* SUIRenderCallback;
    uint32 texture;
    uint32 texID;
    int32 onDisplay;
    int32 dirty;
};


typedef struct SUIEnvironment
{
    int32 xPos, yPos, clicked;
    int32 pXPos, pYPos, pClicked;
    SUIContainer* root;
    SUIDividingLine phantomLine;
    SUIBuffer fullUISpace;
    SUIContainer* currentContainerHeld;
    GLFWcursor* cursor;
    int32 filledTextureSlot[32];
    GLuint elementShader; // TODO(Suhaib): see if uint32 works instead of GLuint
    GLuint lineShader;
    uint32 elementVAO;
    uint32 elementVBO;
    uint32 lineVAO;
    uint32 lineVBO;
    SUIElementVertex elementVertices[100];
    SUILineVertex lineVertices[100];
    int32 elementArraySize;
    int32 lineArraySize;
    
} SUIEnvironment;

internal float32 S2GL_W(float32 cood)
{
    if (cood <= 0.5f)
    {
        return (-1.0f + 2.0f * cood); 
    }
    else
    {
        return (2.0f * (cood - 0.5f));
    }
}

internal float32 S2GL_H(float32 cood)
{
    if (cood <= 0.5f)
    {
        return (1.0f - 2.0f * cood); 
    }
    else
    {
        return (2.0f * (0.5f - cood));
    }
}


internal void SUIRenderCallbackStub(SUIElement* element)
{
    persist int32 colorIndex = 0;
    
    int32 width = element->buffer.width;
    int32 height = element->buffer.height;
    uint32 * buffer = element->buffer.buffer;
    
    for (int32 i = 0; i < width * height; i++)
    {
        buffer[i] = 0xff000000;
    }
    
    
}

SUIElement* SUIGetValidUIElement()
{
    SUIElement* element = malloc(sizeof(SUIElement));
    
    *element = (SUIElement) {0};
    
    glGenTextures(1, &element->texture);
    
    glBindTexture(GL_TEXTURE_2D, element->texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    element->onDisplay = 0;
    element->dirty = 1;
    
    return element;
}

SUIEnvironment* SUIInit()
{
    SUIEnvironment* environment = malloc(sizeof(SUIEnvironment));
    *environment = (SUIEnvironment) {0};
    
    
    GLint result;
    
    GLuint program = glCreateProgram();
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    const GLchar* vertexSource = 
        "#version 330 core\n\
\n\
layout (location = 0) in vec2 position;\n\
layout (location = 1) in vec2 texCood;\n\
layout (location = 2) in float texID;\n\
\n\
out DATA\n\
{\n\
vec2 texCood;\n\
float texID;\n\
} vs_out;\n\
\n\
void main()\n\
{\n\
gl_Position = vec4(position, 1.0f, 1.0f);\n\
vs_out.texCood = texCood;\n\
vs_out.texID = texID;\n\
}";
    
    glShaderSource(vertexShaderID, 1, &vertexSource, NULL);
    glCompileShader(vertexShaderID);
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    
    if (!result)
    {
        puts(vertexSource);
        printf("Cannot Compile Vertex Shader\n");
    }
    
    const GLchar* fragmentSource =
        "#version 330 core\n\
\n\
out vec4 color;\n\
\n\
in DATA\n\
{\n\
vec2 texCood;\n\
float texID;\n\
} fs_in;\n\
\n\
uniform sampler2D textures[32];\n\
\n\
void main()\n\
{\n\
int tid = int (fs_in.texID);\n\
color = texture(textures[tid], fs_in.texCood);\n\
}";
    
    glShaderSource(fragmentShaderID, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShaderID);
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    
    if (!result)
    {
        puts(fragmentSource);
        printf("Cannot Compile Fragment Shader\n");
    }
    
    glAttachShader(program, vertexShaderID);
    glAttachShader(program, fragmentShaderID);
    
    glLinkProgram(program);
    glValidateProgram(program);
    
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    
    environment->elementShader = program;
    
    glUseProgram(environment->elementShader);
    
    int shaderTexArr[32] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
        30, 31 };
    
    glUniform1iv(glGetUniformLocation(environment->elementShader, "textures"), 32, shaderTexArr);
    
    glUseProgram(0);
    
    
    
    program = glCreateProgram();
    vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    vertexSource = 
        "#version 330 core\n\
\n\
layout (location = 0) in vec2 position;\n\
layout (location = 1) in vec4 color;\n\
\n\
out vec4 vert_color;\n\
\n\
void main()\n\
{\n\
gl_Position = vec4(position, 1.0f, 1.0f);\n\
vert_color = color;\n\
}";
    
    glShaderSource(vertexShaderID, 1, &vertexSource, NULL);
    glCompileShader(vertexShaderID);
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    
    if (!result)
    {
        puts(vertexSource);
        printf("Cannot Compile Line Vertex Shader\n");
    }
    
    fragmentSource =
        "#version 330 core\n\
\n\
out vec4 color;\n\
\n\
in vec4 vert_color;\n\
\n\
void main()\n\
{\n\
color = vert_color;\n\
}";
    
    glShaderSource(fragmentShaderID, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShaderID);
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    
    if (!result)
    {
        puts(fragmentSource);
        printf("Cannot Compile Line Fragment Shader\n");
    }
    
    glAttachShader(program, vertexShaderID);
    glAttachShader(program, fragmentShaderID);
    
    glLinkProgram(program);
    glValidateProgram(program);
    
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    
    environment->lineShader = program;
    
    
    glGenVertexArrays(1, &environment->elementVAO);
    glGenBuffers(1, &environment->elementVBO);
    
    
    glBindVertexArray(environment->elementVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, environment->elementVBO);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float) + 1 * sizeof(uint32), (void*) 0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float) + 1 * sizeof(uint32), (void*) (2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE, 4 * sizeof(float) + 1 * sizeof(uint32), (void*) (4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    
    glGenVertexArrays(1, &environment->lineVAO);
    glGenBuffers(1, &environment->lineVBO);
    
    
    glBindVertexArray(environment->lineVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, environment->lineVBO);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    
    environment->root = malloc(sizeof(SUIContainer));
    
    environment->root->x = 0.0f;
    environment->root->y = 0.0f;
    
    environment->root->width = 1.0f;
    environment->root->height = 1.0f;
    
    environment->root->split = NONE;
    
    environment->root->line.start = 0.0f;
    environment->root->line.end = 0.0f;
    environment->root->line.width = 0.0f;
    
    environment->root->element = malloc(sizeof(SUIElement));
    
    environment->root->element->buffer.buffer = NULL;
    
    environment->root->element->SUIRenderCallback = SUIRenderCallbackStub;
    
    environment->root->first = NULL;
    environment->root->second = NULL;
    
    environment->root->ratioOfParent = 1.0f;
    
    environment->root->parent = NULL;
    
    return environment;
}

internal void __SUISubDivide(SUIContainer* container, SUIAlignment align)
{
    
    if (container == NULL) return;
    
    container->split = align;
    
    
    if (container->element != NULL)
    {
        free(container->element->buffer.buffer);
        container->element->buffer.buffer = NULL;
        
        container->element = NULL;
    }
    
    if (container->split == VERTICAL)
    {
        SUIDividingLine line;
        line.start = container->y;
        line.end = container->y + container->height;
        
        line.otherCood = container->x + container->width / 2;
        
        line.width = LINE_WIDTH;
        
        SUIContainer* first = malloc(sizeof(SUIContainer));
        SUIContainer* second = malloc(sizeof(SUIContainer));
        
        container->line = line;
        
        first->x = container->x;
        first->y = container->y;
        
        first->width = container->width / 2;
        first->height = container->height;
        
        first->split = NONE;
        first->first = NULL;
        first->second = NULL;
        
        
        
        first->split = NONE;
        
        first->ratioOfParent = 0.5f;
        
        first->parent = container;
        
        
        second->x = container->x + container->width / 2;
        second->y = container->y;
        
        second->width = container->width / 2;
        second->height = container->height;
        
        second->split = NONE;
        second->first = NULL;
        second->second = NULL;
        
        second->split = NONE;
        
        second->ratioOfParent = 0.5f;
        
        second->parent = container;
        
        
        container->first = first;
        container->second = second;
        
    }
    else if (container->split == HORIZONTAL)
    {
        SUIDividingLine line;
        line.start = container->x;
        line.end = container->x + container->width;
        
        line.otherCood = container->y + container->height / 2;
        
        line.width = LINE_WIDTH;
        
        SUIContainer* first = malloc(sizeof(SUIContainer));
        SUIContainer* second = malloc(sizeof(SUIContainer));
        
        container->line = line;
        
        first->x = container->x;
        first->y = container->y;
        
        first->width = container->width;
        first->height = container->height / 2;
        
        first->split = NONE;
        first->first = NULL;
        first->second = NULL;
        
        first->split = NONE;
        
        first->ratioOfParent = 0.5f;
        
        first->parent = container;
        
        second->x = container->x;
        second->y = container->y + container->height / 2;
        
        second->width = container->width;
        second->height = container->height / 2;
        
        second->split = NONE;
        second->first = NULL;
        second->second = NULL;
        
        second->split = NONE;
        
        second->ratioOfParent = 0.5f;
        
        second->parent = container;
        
        
        container->first = first;
        container->second = second;
    }
    
}


typedef enum SUIDockType
{
    DOCK_COMPLETE,
    DOCK_LEFT,
    DOCK_RIGHT,
    DOCK_TOP,
    DOCK_BOTTOM
    
} SUIDockType;

SUIContainer* SUIGetRoot(SUIEnvironment* environment)
{
    return environment->root;
}


void SUIDock(SUIEnvironment* environment, SUIContainer* container, SUIElement* element, SUIDockType docType, SUIContainer** first, SUIContainer** second)
{
    // TODO(Suhaib): Add check for problems like trying to dock to a non leaf
    
    if (docType == DOCK_COMPLETE)
    {
        element->buffer.width = (int32) (environment->fullUISpace.width * container->width);
        element->buffer.height = (int32) (environment->fullUISpace.height * container->height);
        element->onDisplay = 1;
        
        container->element = element;
        
        *first = NULL;
        *second = NULL;
    }
    else if (docType == DOCK_LEFT)
    {
        __SUISubDivide(container, VERTICAL);
        
        *first = container->first;
        *second = container->second;
        
        element->buffer.width = (int32) (environment->fullUISpace.width * container->first->width);
        element->buffer.height = (int32) (environment->fullUISpace.height * container->first->height);
        element->onDisplay = 1;
        
        container->first->element = element;
        
    }
    else if (docType == DOCK_RIGHT)
    {
        __SUISubDivide(container, VERTICAL);
        
        *first = container->first;
        *second = container->second;
        
        element->buffer.width = (int32) (environment->fullUISpace.width * container->second->width);
        element->buffer.height = (int32) (environment->fullUISpace.height * container->second->height);
        element->onDisplay = 1;
        
        container->second->element = element;
    }
    else if (docType == DOCK_TOP)
    {
        __SUISubDivide(container, HORIZONTAL);
        
        *first = container->first;
        *second = container->second;
        
        element->buffer.width = (int32) (environment->fullUISpace.width * container->first->width);
        element->buffer.height = (int32) (environment->fullUISpace.height * container->first->height);
        element->onDisplay = 1;
        
        container->first->element = element;
    }
    else if (docType == DOCK_BOTTOM)
    {
        __SUISubDivide(container, HORIZONTAL);
        
        *first = container->first;
        *second = container->second;
        
        element->buffer.width = (int32) (environment->fullUISpace.width * container->second->width);
        element->buffer.height = (int32) (environment->fullUISpace.height * container->second->height);
        element->onDisplay = 1;
        
        container->second->element = element;
    }
}

// TODO(Suhaib): Complete this
void SUIUndock()
{
    
}


void SUIOpen(SUIEnvironment* environment, SUIElement* element)
{
    for (int i = 0; i < 32; i++)
    {
        if (environment->filledTextureSlot[i] == 0)
        {
            element->texID = i;
            environment->filledTextureSlot[i] = 1;
            return;
        }
    }
}

void SUIClose(SUIEnvironment* environment, SUIElement* element)
{
    // TODO(Suhaib): SUIUndock is incomplete
    SUIUndock();
    environment->filledTextureSlot[element->texID] = 0;
}


void SUISetMouseState(SUIEnvironment* environment, int32 xpos, int32 ypos, int32 clicked)
{
    environment->pXPos = environment->xPos;
    environment->pYPos = environment->yPos;
    environment->pClicked = environment->clicked;
    
    environment->xPos = xpos;
    environment->yPos = ypos;
    environment->clicked = clicked;
}


void __SUIDrawSeperatingLines(SUIEnvironment* environment, SUIContainer* container)
{
    
    if (container->split == NONE) return;
    if (container == NULL) return;
    
    SUIDividingLine line = container->line;
    
    //glLineWidth(line.width); 
    //glColor3f(1.0, 1.0, 1.0);
    //glBegin(GL_LINES);
    if (container->split == VERTICAL)
    {
        float actualWidth = line.width / environment->fullUISpace.width;
        
        float x1 = S2GL_W(line.otherCood);
        float x2 = S2GL_W(line.otherCood + actualWidth);
        
        float y1 = S2GL_H(line.start);
        float y2 = S2GL_H(line.end);
        
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y1, 1.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y1, 1.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y2, 1.0f, 0.0f, 0.0f, 1.0f};
        
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y1, 1.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y2, 1.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y2, 1.0f, 0.0f, 0.0f, 1.0f};
        
    }
    else if (container->split == HORIZONTAL)
    {
        
        float actualWidth = line.width / environment->fullUISpace.height;
        
        float y1 = S2GL_H(line.otherCood);
        float y2 = S2GL_H(line.otherCood + actualWidth);
        
        float x1 = S2GL_W(line.start);
        float x2 = S2GL_W(line.end);
        
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y1, 1.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y1, 1.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y2, 1.0f, 0.0f, 0.0f, 1.0f};
        
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y1, 1.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y2, 1.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y2, 1.0f, 0.0f, 0.0f, 1.0f};
        
    }
    
    __SUIDrawSeperatingLines(environment, container->first);
    __SUIDrawSeperatingLines(environment, container->second);
    
}

void __SUIResizeWindow(SUIEnvironment* environment, SUIContainer* container, SUIChildDirection direction)
{
    switch (direction)
    {
        case LEFT:
        {
            SUIContainer* parent = container->parent;
            
            container->x = parent->x;
            container->y = parent->y;
            
            float32 widthChange = 0.0f;
            float32 heightChange = 0.0f;
            
            if (parent->height != container->height)
            {
                heightChange = parent->height - container->height;
                
                container->height += heightChange;
            }
            
            if (parent->width * container->ratioOfParent != container->width)
            {
                widthChange = parent->width * container->ratioOfParent - container->width;
                
                container->width += widthChange;
            }
            
            container->ratioOfParent = container->width / parent->width;
            
            
            if (container->split == HORIZONTAL)
            {
                container->line.start = container->x;
                container->line.end = container->x + container->width;
                container->line.otherCood = container->y + container->height * container->first->ratioOfParent;
                
                __SUIResizeWindow(environment, container->first, TOP);
                __SUIResizeWindow(environment, container->second, BOTTOM);
            }
            else if (container->split == VERTICAL)
            {
                container->line.start = container->y;
                container->line.end = container->y + container->height;
                
                container->line.otherCood = container->x + container->width * container->first->ratioOfParent;
                
                __SUIResizeWindow(environment, container->first, LEFT);
                __SUIResizeWindow(environment, container->second, RIGHT);
            }
        }
        break;
        case RIGHT:
        {
            SUIContainer* parent = container->parent;
            
            container->x = parent->x + parent->width * (1 - container->ratioOfParent);
            container->y = parent->y;
            
            float32 widthChange = 0.0f;
            float32 heightChange = 0.0f;
            
            if (parent->height != container->height)
            {
                heightChange = parent->height - container->height;
                
                container->height += heightChange;
            }
            
            if (parent->width * container->ratioOfParent != container->width)
            {
                widthChange = parent->width * container->ratioOfParent - container->width;
                
                container->width += widthChange;
            }
            
            container->ratioOfParent = container->width / parent->width;
            
            if (container->split == HORIZONTAL)
            {
                container->line.start = container->x;
                container->line.end = container->x + container->width;
                container->line.otherCood = container->y + container->height * container->first->ratioOfParent;
                
                __SUIResizeWindow(environment, container->first, TOP);
                __SUIResizeWindow(environment, container->second, BOTTOM);
            }
            else if (container->split == VERTICAL)
            {
                container->line.start = container->y;
                container->line.end = container->y + container->height;
                
                container->line.otherCood = container->x + container->width * container->first->ratioOfParent;
                
                __SUIResizeWindow(environment, container->first, LEFT);
                __SUIResizeWindow(environment, container->second, RIGHT);
            }
        }
        break;
        case TOP:
        {
            SUIContainer* parent = container->parent;
            
            container->x = parent->x;
            container->y = parent->y;
            
            float32 widthChange = 0.0f;
            float32 heightChange = 0.0f;
            
            if (parent->width != container->width)
            {
                widthChange= parent->width - container->width;
                
                container->width += widthChange;
            }
            
            if (parent->height * container->ratioOfParent != container->height)
            {
                heightChange = parent->height * container->ratioOfParent - container->height;
                
                container->height += heightChange;
            }
            
            container->ratioOfParent = container->height / parent->height;
            
            if (container->split == HORIZONTAL)
            {
                container->line.start = container->x;
                container->line.end = container->x + container->width;
                container->line.otherCood = container->y + container->height * container->first->ratioOfParent;
                
                __SUIResizeWindow(environment, container->first, TOP);
                __SUIResizeWindow(environment, container->second, BOTTOM);
            }
            else if (container->split == VERTICAL)
            {
                container->line.start = container->y;
                container->line.end = container->y + container->height;
                
                container->line.otherCood = container->x + container->width * container->first->ratioOfParent;
                
                __SUIResizeWindow(environment, container->first, LEFT);
                __SUIResizeWindow(environment, container->second, RIGHT);
            }
        }
        break;
        case BOTTOM:
        {
            SUIContainer* parent = container->parent;
            
            container->x = parent->x;
            container->y = parent->y + parent->height * (1 - container->ratioOfParent);
            
            float32 widthChange = 0.0f;
            float32 heightChange = 0.0f;
            
            if (parent->width != container->width)
            {
                widthChange= parent->width - container->width;
                
                container->width += widthChange;
            }
            
            if (parent->height * container->ratioOfParent != container->height)
            {
                heightChange = parent->height * container->ratioOfParent - container->height;
                
                container->height += heightChange;
            }
            
            container->ratioOfParent = container->height / parent->height;
            
            if (container->split == HORIZONTAL)
            {
                container->line.start = container->x;
                container->line.end = container->x + container->width;
                container->line.otherCood = container->y + container->height * container->first->ratioOfParent;
                
                __SUIResizeWindow(environment, container->first, TOP);
                __SUIResizeWindow(environment, container->second, BOTTOM);
            }
            else if (container->split == VERTICAL)
            {
                container->line.start = container->y;
                container->line.end = container->y + container->height;
                
                container->line.otherCood = container->x + container->width * container->first->ratioOfParent;
                
                __SUIResizeWindow(environment, container->first, LEFT);
                __SUIResizeWindow(environment, container->second, RIGHT);
            }
        }
        break;
    }
    
    
    if (container->element != NULL)
    {
        int32 oldWidth = container->element->buffer.width;
        int32 oldHeight = container->element->buffer.height;
        
        int32 newWidth = (int32) (container->width * environment->fullUISpace.width);
        int32 newHeight = (int32) (container->height * environment->fullUISpace.height);
        
        container->element->buffer.width = newWidth;
        container->element->buffer.height = newHeight;
        
        if (newWidth * newHeight != oldWidth * oldHeight)
        {
            free(container->element->buffer.buffer);
            container->element->buffer.buffer = NULL;
            
            container->element->buffer.buffer = malloc(newHeight * newWidth * 4);
            
            container->element->dirty = 1;
            
        }
        
    }
    
    
    
}

void __SUIMoveSeperatingLine(SUIEnvironment* environment, SUIContainer* container, int32 pixels)
{
    if (container->split == VERTICAL)
    {
        float32 toMove = P2F(pixels, environment->fullUISpace.width);
        
        SUIContainer* first = container->first;
        SUIContainer* second = container->second;
        
        
        if (toMove < 0.0f && first->width + toMove <= 0.1f)
        {
            return;
        }
        if (toMove > 0.0f && second->width - toMove <= 0.1f)
        {
            return;
        }
        
        first->width += toMove;
        
        second->x += toMove;
        second->width -= toMove;
        
        container->line.otherCood += toMove;
        
        first->ratioOfParent = first->width / container->width;
        second->ratioOfParent = second->width / container->width;
        
        if (first->split == VERTICAL)
        {
            first->line.otherCood += toMove * first->first->ratioOfParent;
            
            __SUIResizeWindow(environment, first->first, LEFT);
            __SUIResizeWindow(environment, first->second, RIGHT);
        }
        else if (first->split == HORIZONTAL)
        {
            first->line.end += toMove;
            __SUIResizeWindow(environment, first->first, TOP);
            __SUIResizeWindow(environment, first->second, BOTTOM);
        }
        
        if (second->split == VERTICAL)
        {
            second->line.otherCood += toMove * second->second->ratioOfParent;
            __SUIResizeWindow(environment, second->first, LEFT);
            __SUIResizeWindow(environment, second->second, RIGHT);
        }
        else if (second->split == HORIZONTAL)
        {
            second->line.start += toMove;
            __SUIResizeWindow(environment, second->first, TOP);
            __SUIResizeWindow(environment, second->second, BOTTOM);
        }
        
        
    }
    else if (container->split == HORIZONTAL)
    {
        float32 toMove = P2F(pixels, environment->fullUISpace.height);
        
        SUIContainer* first = container->first;
        SUIContainer* second = container->second;
        
        if (toMove < 0.0f && first->height + toMove <= 0.1f)
        {
            return;
        }
        if (toMove > 0.0f && second->height - toMove <= 0.1f)
        {
            return;
        }
        
        first->height += toMove;
        
        second->y += toMove;
        second->height -= toMove;
        
        container->line.otherCood += toMove;
        
        first->ratioOfParent = first->height / container->height;
        second->ratioOfParent = second->height / container->height;
        
        if (first->split == VERTICAL)
        {
            first->line.end += toMove;
            
            __SUIResizeWindow(environment, first->first, LEFT);
            __SUIResizeWindow(environment, first->second, RIGHT);
        }
        else if (first->split == HORIZONTAL)
        {
            first->line.otherCood += toMove * first->first->ratioOfParent;
            
            __SUIResizeWindow(environment, first->first, TOP);
            __SUIResizeWindow(environment, first->second, BOTTOM);
        }
        
        if (second->split == VERTICAL)
        {
            second->line.start += toMove;
            
            __SUIResizeWindow(environment, second->first, LEFT);
            __SUIResizeWindow(environment, second->second, RIGHT);
        }
        else if (second->split == HORIZONTAL)
        {
            second->line.otherCood += toMove * second->second->ratioOfParent;
            
            __SUIResizeWindow(environment, second->first, TOP);
            __SUIResizeWindow(environment, second->second, BOTTOM);
        }
        
    }
    
    SUIContainer* first = container->first;
    SUIContainer* second = container->second;
    
    if (first->element != NULL)
    {
        int32 oldWidth = first->element->buffer.width;
        int32 oldHeight = first->element->buffer.height;
        
        int32 newWidth = (int32) (first->width * environment->fullUISpace.width);
        int32 newHeight = (int32) (first->height * environment->fullUISpace.height);
        
        first->element->buffer.width = newWidth;
        first->element->buffer.height = newHeight;
        
        if (newWidth * newHeight != oldWidth * oldHeight)
        {
            free(first->element->buffer.buffer);
            first->element->buffer.buffer = NULL;
            
            first->element->buffer.buffer = malloc(newHeight * newWidth * 4);
            
            first->element->buffer.width = newWidth;
            first->element->buffer.height = newHeight;
            
            first->element->dirty = 1;
            
        }
        
    }
    
    if (second->element != NULL)
    {
        int32 oldWidth = second->element->buffer.width;
        int32 oldHeight = second->element->buffer.height;
        
        int32 newWidth = (int32) (second->width * environment->fullUISpace.width);
        int32 newHeight = (int32) (second->height * environment->fullUISpace.height);
        
        second->element->buffer.width = newWidth;
        second->element->buffer.height = newHeight;
        
        if (newWidth * newHeight != oldWidth * oldHeight)
        {
            free(second->element->buffer.buffer);
            second->element->buffer.buffer = NULL;
            
            second->element->buffer.buffer = malloc(newHeight * newWidth * 4);
            
            second->element->buffer.width = newWidth;
            second->element->buffer.height = newHeight;
            
            second->element->dirty = 1;
        }
        
    }
    
}

void __SUIModifyAllBuffer(SUIEnvironment* environment, SUIContainer* container)
{
    if (container == NULL) 
    {
        return;
    }
    
    if (container->element != NULL)
    {
        free(container->element->buffer.buffer);
        container->element->buffer.buffer = NULL;
        
        int newWidth = (int32) (environment->fullUISpace.width * container->width);
        int newHeight = (int32) (environment->fullUISpace.height * container->height);
        
        container->element->buffer.buffer = malloc(newWidth * newHeight * 4);
        
        container->element->buffer.width = newWidth;
        container->element->buffer.height = newHeight;
        
        container->element->dirty = 1;
    }
    
    __SUIModifyAllBuffer(environment, container->first);
    __SUIModifyAllBuffer(environment, container->second);
}

void SUISetDimension(SUIEnvironment* environment, int32 width, int32 height)
{
    
    int oldWidth = environment->fullUISpace.width;
    int oldHeight = environment->fullUISpace.height;
    
    int newWidth = environment->fullUISpace.width = width;
    int newHeight = environment->fullUISpace.height = height;
    
    if (newWidth * newHeight == oldWidth * oldHeight)
    {
        free(environment->fullUISpace.buffer);
        environment->fullUISpace.buffer = NULL;
        environment->fullUISpace.buffer = malloc(newHeight * newWidth * 4);
    }
    
    __SUIModifyAllBuffer(environment, environment->root);
    
}

SUIContainer* __SUIGetSeperatingLineHover(SUIEnvironment* environment, SUIContainer* container)
{
    if (container == NULL) return NULL;
    
    SUIDividingLine line = container->line;
    
    int32 width = environment->fullUISpace.width;
    int32 height = environment->fullUISpace.height;
    
    int32 x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    
    if (container->split == VERTICAL)
    {
        // (start, otherCood - width) to (end, otherCood + width)
        
        x1 = (int32) (line.otherCood * width - line.width);
        x2 = (int32) (line.otherCood * width + line.width);
        y1 = (int32) (line.start * width);
        y2 = (int32) (line.end * width);
        
    }
    else if (container->split == HORIZONTAL)
    {
        // (start, otherCood - width) to (end, otherCood + width)
        
        y1 = (int32) (line.otherCood * height - line.width);
        y2 = (int32) (line.otherCood * height + line.width);
        x1 = (int32) (line.start * width);
        x2 = (int32) (line.end * width);
        
    }
    
    if (environment->xPos >= x1 && environment->xPos <= x2 && environment->yPos >= y1 && environment->yPos <= y2)
    {
        return container;
    }
    
    
    
    SUIContainer* returnContainer = __SUIGetSeperatingLineHover(environment, container->first);
    
    if (returnContainer == NULL)
    {
        returnContainer = __SUIGetSeperatingLineHover(environment, container->second);
    }
    
    return returnContainer;
    
}


void __SUIDelete(SUIEnvironment* environment, SUIContainer* container)
{
    
    if (container == NULL) return;
    // TODO(Suhaib): Terrible code again
    
    SUIContainer* parent = container->parent;
    
    if (parent == NULL) return;
    
    SUIContainer* gParent = parent->parent;
    
    SUIContainer* changedContainer = NULL;
    
    if (gParent == NULL)
    {
        if (parent->first == container)
        {
            changedContainer = parent->second;
            
            parent->second->x = environment->root->x;
            parent->second->y = environment->root->y;
            parent->second->width = environment->root->width;
            parent->second->height = environment->root->height;
            parent->second->ratioOfParent = environment->root->ratioOfParent;
            parent->second->parent = environment->root->parent;
            
            environment->root = parent->second;
        }
        else if (parent->second == container)
        {
            changedContainer = parent->first;
            
            parent->first->x = environment->root->x;
            parent->first->y = environment->root->y;
            parent->first->width = environment->root->width;
            parent->first->height = environment->root->height;
            parent->first->ratioOfParent = environment->root->ratioOfParent;
            parent->first->parent = environment->root->parent;
            
            environment->root = parent->first;
        }
        
        if (environment->root->split == HORIZONTAL)
        {
            environment->root->line.start = environment->root->x;
            environment->root->line.end = environment->root->x + environment->root->width;
            environment->root->line.otherCood = environment->root->y + environment->root->height * environment->root->first->ratioOfParent;
            
            __SUIResizeWindow(environment, environment->root->first, TOP);
            __SUIResizeWindow(environment, environment->root->second, BOTTOM);
        }
        else if (environment->root->split == VERTICAL)
        {
            environment->root->line.start = environment->root->y;
            environment->root->line.end = environment->root->y + environment->root->height;
            environment->root->line.otherCood = environment->root->x + environment->root->width * environment->root->first->ratioOfParent;
            
            __SUIResizeWindow(environment, environment->root->first, LEFT);
            __SUIResizeWindow(environment, environment->root->second, RIGHT);
        }
        
    }
    
    else if (gParent->first == parent)
    {
        
        
        if (parent->first == container)
        {
            changedContainer = parent->second;
            
            parent->second->x = gParent->first->x;
            parent->second->y = gParent->first->y;
            parent->second->width = gParent->first->width;
            parent->second->height = gParent->first->height;
            parent->second->ratioOfParent = gParent->first->ratioOfParent;
            parent->second->parent = gParent;
            
            gParent->first = parent->second;
        }
        else if (parent->second == container)
        {
            changedContainer = parent->first;
            
            parent->first->x = gParent->first->x;
            parent->first->y = gParent->first->y;
            parent->first->width = gParent->first->width;
            parent->first->height = gParent->first->height;
            parent->first->ratioOfParent = gParent->first->ratioOfParent;
            parent->first->parent = gParent;
            
            gParent->first = parent->first;
        }
        
        if (gParent->first->split == HORIZONTAL)
        {
            gParent->first->line.start = gParent->first->x;
            gParent->first->line.end = gParent->first->x + gParent->first->width;
            gParent->first->line.otherCood = gParent->first->y + gParent->first->height * gParent->first->first->ratioOfParent;
            
            __SUIResizeWindow(environment, gParent->first->first, TOP);
            __SUIResizeWindow(environment, gParent->first->second, BOTTOM);
        }
        else if (gParent->first->split == VERTICAL)
        {
            gParent->first->line.start = gParent->first->y;
            gParent->first->line.end = gParent->first->y + gParent->first->height;
            gParent->first->line.otherCood = gParent->first->x + gParent->first->width * gParent->first->first->ratioOfParent;
            
            __SUIResizeWindow(environment, gParent->first->first, LEFT);
            __SUIResizeWindow(environment, gParent->first->second, RIGHT);
        }
        
        
    }
    else if (gParent->second == parent)
    {
        
        if (parent->first == container)
        {
            changedContainer = parent->second;
            
            parent->second->x = gParent->second->x;
            parent->second->y = gParent->second->y;
            parent->second->width = gParent->second->width;
            parent->second->height = gParent->second->height;
            parent->second->ratioOfParent = gParent->second->ratioOfParent;
            parent->second->parent = gParent;
            
            gParent->second = parent->second;
        }
        else if (parent->second == container)
        {
            changedContainer = parent->first;
            
            parent->first->x = gParent->second->x;
            parent->first->y = gParent->second->y;
            parent->first->width = gParent->second->width;
            parent->first->height = gParent->second->height;
            parent->first->ratioOfParent = gParent->second->ratioOfParent;
            parent->first->parent = gParent;
            
            gParent->second = parent->first;
        }
        
        if (gParent->second->split == HORIZONTAL)
        {
            gParent->second->line.start = gParent->second->x;
            gParent->second->line.end = gParent->second->x + gParent->second->width;
            gParent->second->line.otherCood = gParent->second->y + gParent->second->height * gParent->second->first->ratioOfParent;
            
            __SUIResizeWindow(environment, gParent->second->first, TOP);
            __SUIResizeWindow(environment, gParent->second->second, BOTTOM);
        }
        else if (gParent->second->split == VERTICAL)
        {
            gParent->second->line.start = gParent->second->y;
            gParent->second->line.end = gParent->second->y + gParent->second->height;
            gParent->second->line.otherCood = gParent->second->x + gParent->second->width * gParent->second->first->ratioOfParent;
            
            __SUIResizeWindow(environment, gParent->second->first, LEFT);
            __SUIResizeWindow(environment, gParent->second->second, RIGHT);
        }
        
    }
    
    if (changedContainer != NULL)
    {
        if (changedContainer->element != NULL)
        {
            free(changedContainer->element->buffer.buffer);
            
            changedContainer->element->buffer.buffer = NULL;
            
            int newWidth = (int32) (changedContainer->width * environment->fullUISpace.width);
            int newHeight = (int32) (changedContainer->height * environment->fullUISpace.height);
            
            changedContainer->element->buffer.buffer = malloc(newHeight * newWidth * 4);
            changedContainer->element->buffer.width = newWidth;
            changedContainer->element->buffer.height = newHeight;
            
            changedContainer->element->dirty = 1;
        }
    }
    
}



SUIContainer* __SUIGetHoverWindow(SUIEnvironment* environment, SUIContainer* container)
{
    int width = environment->fullUISpace.width;
    int height = environment->fullUISpace.height;
    
    if (container->split == NONE)
    {
        int32 x1 = (int32) (container->x * width);
        int32 x2 = (int32) (container->x * width + container->width * width);
        
        int32 y1 = (int32) (container->y * height);
        int32 y2 = (int32) (container->y * height + container->height * height);
        
        
        
        if (environment->xPos >= x1 && environment->xPos <= x2 && environment->yPos >= y1 && environment->yPos <= y2)
        {
            return container;
        }
        else
        {
            return NULL;
        }
    }
    
    SUIContainer* subWindow = __SUIGetHoverWindow(environment, container->first);
    
    if (subWindow == NULL)
    {
        subWindow = __SUIGetHoverWindow(environment, container->second);
    }
    
    return subWindow;
    
}


global SUIContainer* currentContainerHover;

void SUIUpdate(SUIEnvironment* environment)
{
    
    // NOTE(Suhaib): Currently phantom line has no check, results in no known bugs but looks wierd
    // TODO(Suhaib): Add bound check to phantom line
    
    SUIContainer* container = __SUIGetSeperatingLineHover(environment, environment->root);
    
    SUIContainer* currentContainerHeld = environment->currentContainerHeld;
    
    int clicked = environment->clicked;
    int pClicked = environment->pClicked;
    
    int width = environment->fullUISpace.width;
    
    int height = environment->fullUISpace.height;
    
    currentContainerHover = NULL;
    
    if (clicked == 1 && pClicked == 0) // just clicked
    {
        if (container != NULL)
        {
            environment->currentContainerHeld = container;
            environment->phantomLine = container->line;
        }
    }
    else if (clicked == 0 && pClicked == 1) // just released
    {
        if (currentContainerHeld != NULL)
        {
            if (currentContainerHeld->split == VERTICAL)
            {
                __SUIMoveSeperatingLine(environment, currentContainerHeld, (int32) ((environment->phantomLine.otherCood - currentContainerHeld->line.otherCood) * width));
            }
            else if (currentContainerHeld->split == HORIZONTAL)
            {
                __SUIMoveSeperatingLine(environment, currentContainerHeld, (int32) ((environment->phantomLine.otherCood - currentContainerHeld->line.otherCood) * height));
            }
            environment->currentContainerHeld = NULL;
        }
    }
    else if (clicked == 1) // has held  the button
    {
        if (currentContainerHeld != NULL)
        {
            if (currentContainerHeld->split == VERTICAL)
            {
                environment->phantomLine.otherCood = (float) environment->xPos / (float) width;
            }
            else if (currentContainerHeld->split == HORIZONTAL)
            {
                environment->phantomLine.otherCood = (float) environment->yPos / (float) height;
            }
        }
    }
    else if (clicked == 0) // has no held the button
    {
        
    }
    
    environment->cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    
    if (container != NULL)
    {
        if (container->split == VERTICAL)
        {
            environment->cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        }
        else if (container->split == HORIZONTAL)
        {
            environment->cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        }
    }
    else if (currentContainerHeld != NULL)
    {
        if (currentContainerHeld->split == VERTICAL)
        {
            environment->cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        }
        else if (currentContainerHeld->split == HORIZONTAL)
        {
            environment->cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        }
    }
    else 
    {
        currentContainerHover = __SUIGetHoverWindow(environment, environment->root);
    }
    
    glfwSetCursor(glwindow, environment->cursor);
    
}


void __SUIFillBuffer(SUIEnvironment* environment, SUIContainer* container)
{
    int width = environment->fullUISpace.width;
    int height = environment->fullUISpace.height;
    
    if (container->split == NONE && container->element != NULL)
    {
        
        int32 actualWidth = (int32) (container->width * width);
        int32 actualHeight = (int32) (container->height * height);
        
        
        float32 x1 = S2GL_W(container->x);
        float32 y1 = S2GL_H(container->y);
        
        float32 x2 = S2GL_W(container->x + container->width);
        float32 y2 = S2GL_H(container->y + container->height);
        
        container->element->SUIRenderCallback(container->element);
        
        uint32 tid = container->element->texID; 
        
        environment->elementVertices[environment->elementArraySize++] = (SUIElementVertex) {x1, y1, 0, 0, tid};
        environment->elementVertices[environment->elementArraySize++] = (SUIElementVertex) {x2, y1, 1, 0, tid};
        environment->elementVertices[environment->elementArraySize++] = (SUIElementVertex) {x2, y2, 1, 1, tid};
        
        environment->elementVertices[environment->elementArraySize++] = (SUIElementVertex) {x1, y1, 0, 0, tid};
        environment->elementVertices[environment->elementArraySize++] = (SUIElementVertex) {x2, y2, 1, 1, tid};
        environment->elementVertices[environment->elementArraySize++] = (SUIElementVertex) {x1, y2, 0, 1, tid};
        
        
        if (container->element->dirty == 1)
        {
            glActiveTexture(GL_TEXTURE0 + tid);
            glBindTexture(GL_TEXTURE_2D, container->element->texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, actualWidth, actualHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, container->element->buffer.buffer);
            glGenerateMipmap(GL_TEXTURE_2D);
            
            container->element->dirty = 0;
        }
        
        return;
    }
    
    __SUIFillBuffer(environment, container->first);
    __SUIFillBuffer(environment, container->second);
    
}

void SUIRender(SUIEnvironment* environment)
{
    environment->elementArraySize = 0;
    __SUIFillBuffer(environment, environment->root);
    
    glUseProgram(environment->elementShader);
    
    glBindVertexArray(environment->elementVAO);
    glBindBuffer(GL_ARRAY_BUFFER, environment->elementVBO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(SUIElementVertex) * environment->elementArraySize, environment->elementVertices, GL_DYNAMIC_DRAW);
    
    
    glDrawArrays(GL_TRIANGLES, 0, environment->elementArraySize);
    
    environment->lineArraySize = 0;
    __SUIDrawSeperatingLines(environment, environment->root);
    
    if (environment->currentContainerHeld != NULL)
    {
        if (environment->currentContainerHeld->split == VERTICAL)
        {
            float actualWidth = environment->phantomLine.width / environment->fullUISpace.width;
            
            float x1 = S2GL_W(environment->phantomLine.otherCood);
            float x2 = S2GL_W(environment->phantomLine.otherCood + actualWidth);
            
            float y1 = S2GL_H(environment->phantomLine.start);
            float y2 = S2GL_H(environment->phantomLine.end);
            
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y1, 1.0f, 1.0f, 1.0f, 1.0f};
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y1, 1.0f, 1.0f, 1.0f, 1.0f};
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y2, 1.0f, 1.0f, 1.0f, 1.0f};
            
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y1, 1.0f, 1.0f, 1.0f, 1.0f};
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y2, 1.0f, 1.0f, 1.0f, 1.0f};
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y2, 1.0f, 1.0f, 1.0f, 1.0f};
            
        }
        else if (environment->currentContainerHeld->split == HORIZONTAL)
        {
            
            float actualWidth = environment->phantomLine.width / environment->fullUISpace.height;
            
            float y1 = S2GL_H(environment->phantomLine.otherCood);
            float y2 = S2GL_H(environment->phantomLine.otherCood + actualWidth);
            
            float x1 = S2GL_W(environment->phantomLine.start);
            float x2 = S2GL_W(environment->phantomLine.end);
            
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y1, 1.0f, 1.0f, 1.0f, 1.0f};
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y1, 1.0f, 1.0f, 1.0f, 1.0f};
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y2, 1.0f, 1.0f, 1.0f, 1.0f};
            
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y1, 1.0f, 1.0f, 1.0f, 1.0f};
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y2, 1.0f, 1.0f, 1.0f, 1.0f};
            environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y2, 1.0f, 1.0f, 1.0f, 1.0f};
            
        }
        
    }
    
    if (currentContainerHover != NULL)
    {
        float middleX = S2GL_W(currentContainerHover->x + currentContainerHover->width / 2.0f);
        float middleY = S2GL_H(currentContainerHover->y + currentContainerHover->height / 2.0f);
        
        float halfWidth = 0.02f;
        
        float x1 = middleX - halfWidth;
        float x2 = middleX + halfWidth;
        float y1 = middleY - halfWidth;
        float y2 = middleY + halfWidth;
        
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y1, 0.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y1, 0.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y2, 0.0f, 0.0f, 0.0f, 1.0f};
        
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y1, 0.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x2, y2, 0.0f, 0.0f, 0.0f, 1.0f};
        environment->lineVertices[environment->lineArraySize++] = (SUILineVertex) {x1, y2, 0.0f, 0.0f, 0.0f, 1.0f};
        
    }
    
    glUseProgram(environment->lineShader);
    
    glBindVertexArray(environment->lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, environment->lineVBO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(SUILineVertex) * environment->lineArraySize, environment->lineVertices, GL_DYNAMIC_DRAW);
    
    glDrawArrays(GL_TRIANGLES, 0, environment->lineArraySize);
    
}


/*
// TODO(Suhaib): Remove this mostly we will do stuff like these with a callbACK
SUIContainer* SUIGetHoverWindow(SUIEnvironment* environment)
{
    return __SUIGetHoverWindow(environment, environment->root);
}
*/