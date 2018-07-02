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
        /*if (colorIndex == 0)
            buffer[i] = 0xff00ff00;
        else if (colorIndex == 1)
            buffer[i] = 0xffff0000;
        else if (colorIndex == 2)
            buffer[i] = 0xff00ffff;
        else if (colorIndex == 3)
            buffer[i] = 0xff000000; */
        
        buffer[i] = 0xff000000;
    }
    
    //colorIndex = (colorIndex + 1) % 4;
    
}


SUIEnvironment* SUIInit()
{
    SUIEnvironment* environment = malloc(sizeof(SUIEnvironment));
    
    *environment = (SUIEnvironment) {0};
    
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
        free(container->element);
        container->element = NULL;
    }
    
    container->element = NULL;
    
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
        
        first->element = malloc(sizeof(SUIElement));
        
        first->element->buffer.buffer = NULL;
        
        first->element->SUIRenderCallback = SUIRenderCallbackStub;
        
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
        
        second->element = malloc(sizeof(SUIElement));
        
        second->element->buffer.buffer = NULL;
        
        second->element->SUIRenderCallback = SUIRenderCallbackStub;
        
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
        
        first->element = malloc(sizeof(SUIElement));
        
        first->element->buffer.buffer = NULL;
        
        first->element->SUIRenderCallback = SUIRenderCallbackStub;
        
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
        
        second->element = malloc(sizeof(SUIElement));
        
        second->element->buffer.buffer = NULL;
        
        second->element->SUIRenderCallback = SUIRenderCallbackStub;
        
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

void SUIDock(SUIContainer* container, SUIElement* element, SUIDockType docType, SUIContainer** first, SUIContainer** second)
{
    // TODO(Suhaib): Add check for problems like trying to dock to a non leaf
    
    if (docType == DOCK_COMPLETE)
    {
        container->element = element;
        
        *first = NULL;
        *second = NULL;
    }
    else if (docType == DOCK_LEFT)
    {
        __SUISubDivide(container, VERTICAL);
        
        *first = container->first;
        *second = container->second;
        
        container->first->element = element;
        
    }
    else if (docType == DOCK_RIGHT)
    {
        __SUISubDivide(container, VERTICAL);
        
        *first = container->first;
        *second = container->second;
        
        container->second->element = element;
    }
    else if (docType == DOCK_TOP)
    {
        __SUISubDivide(container, HORIZONTAL);
        
        *first = container->first;
        *second = container->second;
        
        container->first->element = element;
    }
    else if (docType == DOCK_BOTTOM)
    {
        __SUISubDivide(container, HORIZONTAL);
        
        *first = container->first;
        *second = container->second;
        
        container->second->element = element;
    }
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


void __SUIDrawSeperatingLines(SUIContainer* container)
{
    
    if (container->split == NONE) return;
    if (container == NULL) return;
    
    SUIDividingLine line = container->line;
    
    glLineWidth(line.width); 
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    if (container->split == VERTICAL)
    {
        glVertex3f(S2GL_W(line.otherCood), S2GL_H(line.start), 0.0f);
        glVertex3f(S2GL_W(line.otherCood), S2GL_H(line.end), 0.0f);
    }
    else if (container->split == HORIZONTAL)
    {
        glVertex3f(S2GL_W(line.start), S2GL_H(line.otherCood), 0.0f);
        glVertex3f(S2GL_W(line.end), S2GL_H(line.otherCood), 0.0f);
    }
    glEnd();
    
    __SUIDrawSeperatingLines(container->first);
    __SUIDrawSeperatingLines(container->second);
    
}

void __SUIResizeWindow(SUIContainer* container, SUIChildDirection direction)
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
                
                __SUIResizeWindow(container->first, TOP);
                __SUIResizeWindow(container->second, BOTTOM);
            }
            else if (container->split == VERTICAL)
            {
                container->line.start = container->y;
                container->line.end = container->y + container->height;
                
                container->line.otherCood = container->x + container->width * container->first->ratioOfParent;
                
                __SUIResizeWindow(container->first, LEFT);
                __SUIResizeWindow(container->second, RIGHT);
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
                
                __SUIResizeWindow(container->first, TOP);
                __SUIResizeWindow(container->second, BOTTOM);
            }
            else if (container->split == VERTICAL)
            {
                container->line.start = container->y;
                container->line.end = container->y + container->height;
                
                container->line.otherCood = container->x + container->width * container->first->ratioOfParent;
                
                __SUIResizeWindow(container->first, LEFT);
                __SUIResizeWindow(container->second, RIGHT);
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
                
                __SUIResizeWindow(container->first, TOP);
                __SUIResizeWindow(container->second, BOTTOM);
            }
            else if (container->split == VERTICAL)
            {
                container->line.start = container->y;
                container->line.end = container->y + container->height;
                
                container->line.otherCood = container->x + container->width * container->first->ratioOfParent;
                
                __SUIResizeWindow(container->first, LEFT);
                __SUIResizeWindow(container->second, RIGHT);
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
                
                __SUIResizeWindow(container->first, TOP);
                __SUIResizeWindow(container->second, BOTTOM);
            }
            else if (container->split == VERTICAL)
            {
                container->line.start = container->y;
                container->line.end = container->y + container->height;
                
                container->line.otherCood = container->x + container->width * container->first->ratioOfParent;
                
                __SUIResizeWindow(container->first, LEFT);
                __SUIResizeWindow(container->second, RIGHT);
            }
        }
        break;
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
            
            __SUIResizeWindow(first->first, LEFT);
            __SUIResizeWindow(first->second, RIGHT);
        }
        else if (first->split == HORIZONTAL)
        {
            first->line.end += toMove;
            __SUIResizeWindow(first->first, TOP);
            __SUIResizeWindow(first->second, BOTTOM);
        }
        
        if (second->split == VERTICAL)
        {
            second->line.otherCood += toMove * second->second->ratioOfParent;
            __SUIResizeWindow(second->first, LEFT);
            __SUIResizeWindow(second->second, RIGHT);
        }
        else if (second->split == HORIZONTAL)
        {
            second->line.start += toMove;
            __SUIResizeWindow(second->first, TOP);
            __SUIResizeWindow(second->second, BOTTOM);
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
            
            __SUIResizeWindow(first->first, LEFT);
            __SUIResizeWindow(first->second, RIGHT);
        }
        else if (first->split == HORIZONTAL)
        {
            first->line.otherCood += toMove * first->first->ratioOfParent;
            
            __SUIResizeWindow(first->first, TOP);
            __SUIResizeWindow(first->second, BOTTOM);
        }
        
        if (second->split == VERTICAL)
        {
            second->line.start += toMove;
            
            __SUIResizeWindow(second->first, LEFT);
            __SUIResizeWindow(second->second, RIGHT);
        }
        else if (second->split == HORIZONTAL)
        {
            second->line.otherCood += toMove * second->second->ratioOfParent;
            
            __SUIResizeWindow(second->first, TOP);
            __SUIResizeWindow(second->second, BOTTOM);
        }
        
    }
}

void SUISetDimension(SUIEnvironment* environment, int32 width, int32 height)
{
    environment->fullUISpace.width = width;
    environment->fullUISpace.height = height;
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
    
    if (gParent == NULL)
    {
        if (parent->first == container)
        {
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
            
            __SUIResizeWindow(environment->root->first, TOP);
            __SUIResizeWindow(environment->root->second, BOTTOM);
        }
        else if (environment->root->split == VERTICAL)
        {
            environment->root->line.start = environment->root->y;
            environment->root->line.end = environment->root->y + environment->root->height;
            environment->root->line.otherCood = environment->root->x + environment->root->width * environment->root->first->ratioOfParent;
            
            __SUIResizeWindow(environment->root->first, LEFT);
            __SUIResizeWindow(environment->root->second, RIGHT);
        }
        
        
        
        return;
    }
    
    if (gParent->first == parent)
    {
        
        
        if (parent->first == container)
        {
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
            
            __SUIResizeWindow(gParent->first->first, TOP);
            __SUIResizeWindow(gParent->first->second, BOTTOM);
        }
        else if (gParent->first->split == VERTICAL)
        {
            gParent->first->line.start = gParent->first->y;
            gParent->first->line.end = gParent->first->y + gParent->first->height;
            gParent->first->line.otherCood = gParent->first->x + gParent->first->width * gParent->first->first->ratioOfParent;
            
            __SUIResizeWindow(gParent->first->first, LEFT);
            __SUIResizeWindow(gParent->first->second, RIGHT);
        }
        
        
    }
    else if (gParent->second == parent)
    {
        
        if (parent->first == container)
        {
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
            
            __SUIResizeWindow(gParent->second->first, TOP);
            __SUIResizeWindow(gParent->second->second, BOTTOM);
        }
        else if (gParent->second->split == VERTICAL)
        {
            gParent->second->line.start = gParent->second->y;
            gParent->second->line.end = gParent->second->y + gParent->second->height;
            gParent->second->line.otherCood = gParent->second->x + gParent->second->width * gParent->second->first->ratioOfParent;
            
            __SUIResizeWindow(gParent->second->first, LEFT);
            __SUIResizeWindow(gParent->second->second, RIGHT);
        }
        
    }
}


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
    
    glfwSetCursor(glwindow, environment->cursor);
    
}


void __SUIFillBuffer(SUIEnvironment* environment, SUIContainer* container)
{
    int width = environment->fullUISpace.width;
    int height = environment->fullUISpace.height;
    
    if (container->split == NONE)
    {
        
        // TODO(Suhaib): Writing a terrible inefficient program make sure to change it
        
        int32 actualX = (int32) (container->x * width);
        int32 actualY = (int32) (container->y * height);
        
        int32 actualWidth = (int32) (container->width * width);
        int32 actualHeight = (int32) (container->height * height);
        
        container->element->buffer.width = actualWidth;
        container->element->buffer.height = actualHeight;
        
        if (container->element->buffer.buffer != NULL)
        {
            free(container->element->buffer.buffer);
            container->element->buffer.buffer = NULL;
            //free(container->element);
        }
        
        container->element->buffer.width = actualWidth;
        container->element->buffer.height = actualHeight;
        container->element->buffer.buffer = malloc(actualHeight * actualWidth * 4);
        
        container->element->SUIRenderCallback(container->element);
        
        
        // NOTE(Suhaib): All this because OpenGL renders from bottom left instead of top left
        int32 xStart = actualX;
        int32 yStart = height - actualY - 1;
        
        int32 xEnd = xStart + actualWidth - 1;
        int32 yEnd = yStart - actualHeight + 1;
        
        int32* outBuffer = environment->fullUISpace.buffer;
        int32* inBuffer = container->element->buffer.buffer;
        
        int32 inBufferIndex = 0; 
        
        for (int32 y = yStart; y >= yEnd; y--)
        {
            for (int32 x = xStart; x <= xEnd; x++)
            {
                outBuffer[x + y * width] = inBuffer[inBufferIndex++];
            }
        }
        return;
    }
    
    __SUIFillBuffer(environment, container->first);
    __SUIFillBuffer(environment, container->second);
    
}

void SUIRender(SUIEnvironment* environment)
{
    int width = environment->fullUISpace.width;
    int height = environment->fullUISpace.height;
    
    void* buffer = environment->fullUISpace.buffer;
    
    // TODO(Suhaib): Again writing terrible code here please fix this
    
    if (buffer != NULL)
    {
        free(buffer);
        environment->fullUISpace.buffer = NULL;
    }
    
    environment->fullUISpace.width = width;
    environment->fullUISpace.height = height;
    environment->fullUISpace.buffer = malloc(height * width * 4);
    
    __SUIFillBuffer(environment, environment->root);
    
    glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, environment->fullUISpace.buffer);
    
    //UIFillWithStuff();
    __SUIDrawSeperatingLines(environment->root);
    
    if (environment->currentContainerHeld != NULL)
    {
        
        glLineWidth(environment->phantomLine.width); 
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        if (environment->currentContainerHeld->split == VERTICAL)
        {
            glVertex3f(S2GL_W(environment->phantomLine.otherCood), S2GL_H(environment->phantomLine.start), 0.0f);
            glVertex3f(S2GL_W(environment->phantomLine.otherCood), S2GL_H(environment->phantomLine.end), 0.0f);
        }
        else if (environment->currentContainerHeld->split == HORIZONTAL)
        {
            glVertex3f(S2GL_W(environment->phantomLine.start), S2GL_H(environment->phantomLine.otherCood), 0.0f);
            glVertex3f(S2GL_W(environment->phantomLine.end), S2GL_H(environment->phantomLine.otherCood), 0.0f);
        }
        glEnd();
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


// TODO(Suhaib): Remove this mostly we will do stuff like these with a callbACK
SUIContainer* SUIGetHoverWindow(SUIEnvironment* environment)
{
    return __SUIGetHoverWindow(environment, environment->root);
}