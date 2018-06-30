#include <stdio.h>
#include <malloc.h>

#define LINE_WIDTH 5

int currColorIndex = 0;

typedef struct SubWindow SubWindow;

static int uiWidth, uiHeight;
static float pixelWidth, pixelHeight;

static int xPos, yPos, held;
static int pXPos, pYPos, pHeld;

#define F2P(ratio, dim) ((ratio) * (dim))
#define P2F(pixel, dim) ((float) (pixel) / (float) (dim))


typedef struct UIElement UIElement;

typedef void (UIRenderCallbackFunction(UIElement*));


//#define S2GL(cood) ((cood) * 2.0f - 1)

float S2GL_W(float cood)
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

float S2GL_H(float cood)
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


typedef enum Alignment
{
    NONE,
    VERTICAL,
    HORIZONTAL
    
} Alignment;



typedef struct DividingLine
{
    float start, end;
    float otherCood;
    float width;
    
} DividingLine;

typedef enum ChildDirection
{
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
    
} ChildDirection;

typedef struct UIBuffer
{
    
    int width;
    int height;
    void* buffer;
    
} UIBuffer;

static UIBuffer fullscreen;


typedef struct UIElement
{
    UIBuffer buffer;
    UIRenderCallbackFunction* uiRenderCallback;
    
} UIElement;




void UIRenderCallbackStub(UIElement* element)
{
    static int colorIndex = 0;
    
    int width = element->buffer.width;
    int height = element->buffer.height;
    int* buffer = element->buffer.buffer;
    
    for (int i = 0; i < width * height; i++)
    {
        if (colorIndex == 0)
            buffer[i] = 0xff00ff00;
        else if (colorIndex == 1)
            buffer[i] = 0xffff0000;
        else if (colorIndex == 2)
            buffer[i] = 0xff00ffff;
        else if (colorIndex == 3)
            buffer[i] = 0xff000000; 
    }
    
    colorIndex = (colorIndex + 1) % 4;
    
}


struct SubWindow
{
    float x, y;
    float width;
    float height;
    float ratioOfParent;
    Alignment split;
    DividingLine line;
    UIElement* element;
    SubWindow* first;
    SubWindow* second;
    SubWindow* parent;
};

SubWindow* root;

SubWindow* UIInit()
{
    root = malloc(sizeof(SubWindow));
    
    root->x = 0.0f;
    root->y = 0.0f;
    
    root->width = 1.0f;
    root->height = 1.0f;
    
    root->split = NONE;
    
    root->line.start = 0.0f;
    root->line.end = 0.0f;
    root->line.width = 0.0f;
    
    root->element = malloc(sizeof(UIElement));
    
    root->element->buffer.buffer = NULL;
    
    root->element->uiRenderCallback = UIRenderCallbackStub;
    
    root->first = NULL;
    root->second = NULL;
    
    root->ratioOfParent = 1.0f;
    
    root->parent = NULL;
    
    return root;
}

void SubDivide(SubWindow* window, Alignment align)
{
    window->split = align;
    
    if (window->element != NULL)
    {
        free(window->element->buffer.buffer);
        window->element->buffer.buffer = NULL;
        free(window->element);
        window->element = NULL;
    }
    
    window->element = NULL;
    
    if (window->split == VERTICAL)
    {
        DividingLine line;
        line.start = window->y;
        line.end = window->y + window->height;
        
        line.otherCood = window->x + window->width / 2;
        
        line.width = LINE_WIDTH;
        
        SubWindow* first = malloc(sizeof(SubWindow));
        SubWindow* second = malloc(sizeof(SubWindow));
        
        window->line = line;
        
        first->x = window->x;
        first->y = window->y;
        
        first->width = window->width / 2;
        first->height = window->height;
        
        first->split = NONE;
        first->first = NULL;
        first->second = NULL;
        
        
        
        first->split = NONE;
        
        first->ratioOfParent = 0.5f;
        
        first->element = malloc(sizeof(UIElement));
        
        first->element->buffer.buffer = NULL;
        
        first->element->uiRenderCallback = UIRenderCallbackStub;
        
        first->parent = window;
        
        second->x = window->x + window->width / 2;
        second->y = window->y;
        
        second->width = window->width / 2;
        second->height = window->height;
        
        second->split = NONE;
        second->first = NULL;
        second->second = NULL;
        
        second->split = NONE;
        
        second->ratioOfParent = 0.5f;
        
        second->element = malloc(sizeof(UIElement));
        
        second->element->buffer.buffer = NULL;
        
        second->element->uiRenderCallback = UIRenderCallbackStub;
        
        second->parent = window;
        
        
        window->first = first;
        window->second = second;
        
    }
    else if (window->split == HORIZONTAL)
    {
        DividingLine line;
        line.start = window->x;
        line.end = window->x + window->width;
        
        line.otherCood = window->y + window->height / 2;
        
        line.width = LINE_WIDTH;
        
        SubWindow* first = malloc(sizeof(SubWindow));
        SubWindow* second = malloc(sizeof(SubWindow));
        
        window->line = line;
        
        first->x = window->x;
        first->y = window->y;
        
        first->width = window->width;
        first->height = window->height / 2;
        
        first->split = NONE;
        first->first = NULL;
        first->second = NULL;
        
        first->split = NONE;
        
        first->ratioOfParent = 0.5f;
        
        first->element = malloc(sizeof(UIElement));
        
        first->element->buffer.buffer = NULL;
        
        first->element->uiRenderCallback = UIRenderCallbackStub;
        
        first->parent = window;
        
        second->x = window->x;
        second->y = window->y + window->height / 2;
        
        second->width = window->width;
        second->height = window->height / 2;
        
        second->split = NONE;
        second->first = NULL;
        second->second = NULL;
        
        second->split = NONE;
        
        second->ratioOfParent = 0.5f;
        
        second->element = malloc(sizeof(UIElement));
        
        second->element->buffer.buffer = NULL;
        
        second->element->uiRenderCallback = UIRenderCallbackStub;
        
        second->parent = window;
        
        
        window->first = first;
        window->second = second;
    }
    
}

void FillWithStuff(SubWindow* window)
{
    if (window->split == NONE)
    {
        
        if (currColorIndex % 8 == 0)
        {
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        if (currColorIndex % 8 == 1)
        {
            glColor3f(0.0f, 1.0f, 0.0f);
        }
        if (currColorIndex % 8 == 2)
        {
            glColor3f(0.0f, 0.0f, 1.0f);
        }
        if (currColorIndex % 8 == 3)
        {
            glColor3f(1.0f, 1.0f, 0.0f);
        }
        if (currColorIndex % 8 == 4)
        {
            glColor3f(0.0f, 1.0f, 1.0f);
        }
        if (currColorIndex % 8 == 5)
        {
            glColor3f(1.0f, 0.0f, 1.0f);
        }
        if (currColorIndex % 8 == 6)
        {
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        if (currColorIndex % 8 == 7)
        {
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        
        currColorIndex = (currColorIndex + 1) % 8;
        
        glBegin(GL_QUADS);
        
        glVertex3f(S2GL_W(window->x), S2GL_H(window->y), 0.0f);
        glVertex3f(S2GL_W(window->x + window->width), S2GL_H(window->y), 0.0f);
        glVertex3f(S2GL_W(window->x + window->width), S2GL_H(window->y + window->height), 0.0f);
        glVertex3f(S2GL_W(window->x), S2GL_H(window->y + window->height), 0.0f);
        
        glEnd();
        return;
    }
    FillWithStuff(window->first);
    FillWithStuff(window->second);
}

void UISetMouseState(int xpos, int ypos, int pressed)
{
    pXPos = xPos;
    pYPos = yPos;
    pHeld = held;
    
    xPos = xpos;
    yPos = ypos;
    held = pressed;
}

void UIFillWithStuff()
{
    currColorIndex = 0;
    FillWithStuff(root);
}


void DrawSeperatingLines(SubWindow* window)
{
    
    if (window->split == NONE) return;
    if (window == NULL) return;
    
    DividingLine line = window->line;
    
    glLineWidth(line.width); 
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    if (window->split == VERTICAL)
    {
        glVertex3f(S2GL_W(line.otherCood), S2GL_H(line.start), 0.0f);
        glVertex3f(S2GL_W(line.otherCood), S2GL_H(line.end), 0.0f);
    }
    else if (window->split == HORIZONTAL)
    {
        glVertex3f(S2GL_W(line.start), S2GL_H(line.otherCood), 0.0f);
        glVertex3f(S2GL_W(line.end), S2GL_H(line.otherCood), 0.0f);
    }
    glEnd();
    
    DrawSeperatingLines(window->first);
    DrawSeperatingLines(window->second);
    
}


void UIDrawSeperatingLines()
{
    DrawSeperatingLines(root);
}

void ResizeWindow(SubWindow* window, ChildDirection direction)
{
    switch (direction)
    {
        case LEFT:
        {
            SubWindow* parent = window->parent;
            
            window->x = parent->x;
            window->y = parent->y;
            
            float widthChange = 0.0f;
            float heightChange = 0.0f;
            
            if (parent->height != window->height)
            {
                heightChange = parent->height - window->height;
                
                window->height += heightChange;
            }
            
            if (parent->width * window->ratioOfParent != window->width)
            {
                widthChange = parent->width * window->ratioOfParent - window->width;
                
                window->width += widthChange;
            }
            
            window->ratioOfParent = window->width / parent->width;
            
            if (window->split == HORIZONTAL)
            {
                window->line.start = window->x;
                window->line.end = window->x + window->width;
                window->line.otherCood = window->y + window->height * window->first->ratioOfParent;
                
                ResizeWindow(window->first, TOP);
                ResizeWindow(window->second, BOTTOM);
            }
            else if (window->split == VERTICAL)
            {
                window->line.start = window->y;
                window->line.end = window->y + window->height;
                
                window->line.otherCood = window->x + window->width * window->first->ratioOfParent;
                
                ResizeWindow(window->first, LEFT);
                ResizeWindow(window->second, RIGHT);
            }
        }
        break;
        case RIGHT:
        {
            SubWindow* parent = window->parent;
            
            window->x = parent->x + parent->width * (1 - window->ratioOfParent);
            window->y = parent->y;
            
            float widthChange = 0.0f;
            float heightChange = 0.0f;
            
            if (parent->height != window->height)
            {
                heightChange = parent->height - window->height;
                
                window->height += heightChange;
            }
            
            if (parent->width * window->ratioOfParent != window->width)
            {
                widthChange = parent->width * window->ratioOfParent - window->width;
                
                window->width += widthChange;
            }
            
            window->ratioOfParent = window->width / parent->width;
            
            if (window->split == HORIZONTAL)
            {
                window->line.start = window->x;
                window->line.end = window->x + window->width;
                window->line.otherCood = window->y + window->height * window->first->ratioOfParent;
                
                ResizeWindow(window->first, TOP);
                ResizeWindow(window->second, BOTTOM);
            }
            else if (window->split == VERTICAL)
            {
                window->line.start = window->y;
                window->line.end = window->y + window->height;
                
                window->line.otherCood = window->x + window->width * window->first->ratioOfParent;
                
                ResizeWindow(window->first, LEFT);
                ResizeWindow(window->second, RIGHT);
            }
        }
        break;
        case TOP:
        {
            SubWindow* parent = window->parent;
            
            window->x = parent->x;
            window->y = parent->y;
            
            float widthChange = 0.0f;
            float heightChange = 0.0f;
            
            if (parent->width != window->width)
            {
                widthChange= parent->width - window->width;
                
                window->width += widthChange;
            }
            
            if (parent->height * window->ratioOfParent != window->height)
            {
                heightChange = parent->height * window->ratioOfParent - window->height;
                
                window->height += heightChange;
            }
            
            window->ratioOfParent = window->height / parent->height;
            
            if (window->split == HORIZONTAL)
            {
                window->line.start = window->x;
                window->line.end = window->x + window->width;
                window->line.otherCood = window->y + window->height * window->first->ratioOfParent;
                
                ResizeWindow(window->first, TOP);
                ResizeWindow(window->second, BOTTOM);
            }
            else if (window->split == VERTICAL)
            {
                window->line.start = window->y;
                window->line.end = window->y + window->height;
                
                window->line.otherCood = window->x + window->width * window->first->ratioOfParent;
                
                ResizeWindow(window->first, LEFT);
                ResizeWindow(window->second, RIGHT);
            }
        }
        break;
        case BOTTOM:
        {
            SubWindow* parent = window->parent;
            
            window->x = parent->x;
            window->y = parent->y + parent->height * (1 - window->ratioOfParent);
            
            float widthChange = 0.0f;
            float heightChange = 0.0f;
            
            if (parent->width != window->width)
            {
                widthChange= parent->width - window->width;
                
                window->width += widthChange;
            }
            
            if (parent->height * window->ratioOfParent != window->height)
            {
                heightChange = parent->height * window->ratioOfParent - window->height;
                
                window->height += heightChange;
            }
            
            window->ratioOfParent = window->height / parent->height;
            
            if (window->split == HORIZONTAL)
            {
                window->line.start = window->x;
                window->line.end = window->x + window->width;
                window->line.otherCood = window->y + window->height * window->first->ratioOfParent;
                
                ResizeWindow(window->first, TOP);
                ResizeWindow(window->second, BOTTOM);
            }
            else if (window->split == VERTICAL)
            {
                window->line.start = window->y;
                window->line.end = window->y + window->height;
                
                window->line.otherCood = window->x + window->width * window->first->ratioOfParent;
                
                ResizeWindow(window->first, LEFT);
                ResizeWindow(window->second, RIGHT);
            }
        }
        break;
    }
}

void UIMoveSeperatingLine(SubWindow* window, int pixels)
{
    if (window->split == VERTICAL)
    {
        float toMove = P2F(pixels, uiWidth);
        
        SubWindow* first = window->first;
        SubWindow* second = window->second;
        
        
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
        
        window->line.otherCood += toMove;
        
        first->ratioOfParent = first->width / window->width;
        second->ratioOfParent = second->width / window->width;
        
        if (first->split == VERTICAL)
        {
            first->line.otherCood += toMove * first->first->ratioOfParent;
            
            ResizeWindow(first->first, LEFT);
            ResizeWindow(first->second, RIGHT);
        }
        else if (first->split == HORIZONTAL)
        {
            first->line.end += toMove;
            ResizeWindow(first->first, TOP);
            ResizeWindow(first->second, BOTTOM);
        }
        
        if (second->split == VERTICAL)
        {
            second->line.otherCood += toMove * second->second->ratioOfParent;
            ResizeWindow(second->first, LEFT);
            ResizeWindow(second->second, RIGHT);
        }
        else if (second->split == HORIZONTAL)
        {
            second->line.start += toMove;
            ResizeWindow(second->first, TOP);
            ResizeWindow(second->second, BOTTOM);
        }
        
        
    }
    else if (window->split == HORIZONTAL)
    {
        float toMove = P2F(pixels, uiHeight);
        
        SubWindow* first = window->first;
        SubWindow* second = window->second;
        
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
        
        window->line.otherCood += toMove;
        
        first->ratioOfParent = first->height / window->height;
        second->ratioOfParent = second->height / window->height;
        
        if (first->split == VERTICAL)
        {
            first->line.end += toMove;
            
            ResizeWindow(first->first, LEFT);
            ResizeWindow(first->second, RIGHT);
        }
        else if (first->split == HORIZONTAL)
        {
            first->line.otherCood += toMove * first->first->ratioOfParent;
            
            ResizeWindow(first->first, TOP);
            ResizeWindow(first->second, BOTTOM);
        }
        
        if (second->split == VERTICAL)
        {
            second->line.start += toMove;
            
            ResizeWindow(second->first, LEFT);
            ResizeWindow(second->second, RIGHT);
        }
        else if (second->split == HORIZONTAL)
        {
            second->line.otherCood += toMove * second->second->ratioOfParent;
            
            ResizeWindow(second->first, TOP);
            ResizeWindow(second->second, BOTTOM);
        }
        
    }
}

void UISetDimension(int width, int height)
{
    uiWidth = width;
    uiHeight = height;
    
    pixelWidth = 1.0f / (float) width;
    pixelHeight = 1.0f / (float) height;
}

SubWindow* GetSeperatingLineHover(SubWindow* window)
{
    if (window == NULL) return NULL;
    
    DividingLine line = window->line;
    
    if (window->split == VERTICAL)
    {
        // (start, otherCood - width) to (end, otherCood + width)
        
        int x1 = (int) (line.otherCood * uiWidth - line.width);
        int x2 = (int) (line.otherCood * uiWidth + line.width);
        int y1 = (int) (line.start * uiHeight);
        int y2 = (int) (line.end * uiHeight);
        
        
        if (xPos >= x1 && xPos <= x2 && yPos >= y1 && yPos <= y2)
        {
            return window;
        }
        
    }
    else if (window->split == HORIZONTAL)
    {
        // (start, otherCood - width) to (end, otherCood + width)
        
        int y1 = (int) (line.otherCood * uiHeight - line.width);
        int y2 = (int) (line.otherCood * uiHeight + line.width);
        int x1 = (int) (line.start * uiWidth);
        int x2 = (int) (line.end * uiWidth);
        
        
        if (xPos >= x1 && xPos <= x2 && yPos >= y1 && yPos <= y2)
        {
            return window;
        }
        
    }
    
    SubWindow* rWindow = GetSeperatingLineHover(window->first);
    
    if (rWindow == NULL)
    {
        rWindow = GetSeperatingLineHover(window->second);
    }
    
    return rWindow;
    
}

SubWindow* currentWindowHeld = NULL;

void UIUpdate()
{
    
    //printf("[%d, %d]\n", uiWidth, uiHeight);
    
    static int count = 0;
    //printf("xPos : %d, yPos : %d, Pressed : %d\n", xPos, yPos, held);
    
    SubWindow* window = GetSeperatingLineHover(root);
    
    
    if (!held)
    {
        currentWindowHeld = NULL;
    }
    
    if (currentWindowHeld)
    {
        if (currentWindowHeld->split == VERTICAL)
        {
            UIMoveSeperatingLine(currentWindowHeld, xPos - pXPos);
        }
        else if (currentWindowHeld->split == HORIZONTAL)
        {
            UIMoveSeperatingLine(currentWindowHeld, yPos - pYPos);
        }
    }
    else
    {
        if (window != NULL)
        {
            if (held && !pHeld) 
            {
                currentWindowHeld = window;
            }
        }
    }
    
    
}


void UIFillBuffer(SubWindow* window)
{
    if (window->split == NONE)
    {
        
        // TODO(Suhaib): Writing a terrible inefficient program make sure to change it
        
        int actualX = (int) (window->x * uiWidth);
        int actualY = (int) (window->y * uiHeight);
        
        int actualWidth = (int) (window->width * uiWidth);
        int actualHeight = (int) (window->height * uiHeight);
        
        window->element->buffer.width = actualWidth;
        window->element->buffer.height = actualHeight;
        
        if (window->element->buffer.buffer != NULL)
        {
            free(window->element->buffer.buffer);
            window->element->buffer.buffer = NULL;
            //free(window->element);
        }
        
        window->element->buffer.width = actualWidth;
        window->element->buffer.height = actualHeight;
        window->element->buffer.buffer = malloc(actualHeight * actualWidth * 4);
        
        window->element->uiRenderCallback(window->element);
        
        
        // NOTE(Suhaib): All this because OpenGL renders from bottom left instead of top left
        int xStart = actualX;
        int yStart = uiHeight - actualY - 1;
        
        int xEnd = xStart + actualWidth - 1;
        int yEnd = yStart - actualHeight + 1;
        
        int* outBuffer = fullscreen.buffer;
        int* inBuffer = window->element->buffer.buffer;
        
        int inBufferIndex = 0; 
        
        for (int y = yStart; y >= yEnd; y--)
        {
            for (int x = xStart; x <= xEnd; x++)
            {
                outBuffer[x + y * uiWidth] = inBuffer[inBufferIndex++];
            }
        }
        return;
    }
    
    UIFillBuffer(window->first);
    UIFillBuffer(window->second);
    
}

void UIRender()
{
    
    // TODO(Suhaib): Again writing terrible code here please fix this
    
    if (fullscreen.buffer != NULL)
    {
        free(fullscreen.buffer);
        fullscreen.buffer = NULL;
    }
    
    fullscreen.width = uiWidth;
    fullscreen.height = uiHeight;
    fullscreen.buffer = malloc(uiHeight * uiWidth * 4);
    
    
    UIFillBuffer(root);
    /*
    int* buffer = fullscreen.buffer;
    
    for (int i = 0; i < fullscreen.width * fullscreen.height; i++)
    {
    buffer[i] = 0xff000000;
    }
    
    for (int y = 0; y < fullscreen.height / 2; y++)
    {
    for (int x = 0; x < fullscreen.width / 2; x++)
    {
    buffer[x + y * fullscreen.width] = 0xff00ff00;
    }
    }
    */
    glDrawPixels(fullscreen.width, fullscreen.height, GL_RGBA, GL_UNSIGNED_BYTE, fullscreen.buffer);
    
    //UIFillWithStuff();
    UIDrawSeperatingLines();
}

SubWindow* GetHoverWindow(SubWindow* window)
{
    if (window->split == NONE)
    {
        int x1 = (int) (window->x * uiWidth);
        int x2 = (int) (window->x * uiWidth + window->width * uiWidth);
        
        int y1 = (int) (window->y * uiHeight);
        int y2 = (int) (window->y * uiHeight + window->height * uiHeight);
        
        if (xPos >= x1 && xPos <= x2 && yPos >= y1 && yPos <= y2)
        {
            return window;
        }
        else
        {
            return NULL;
        }
    }
    
    SubWindow* subWindow = GetHoverWindow(window->first);
    
    if (subWindow == NULL)
    {
        subWindow = GetHoverWindow(window->second);
    }
    
    return subWindow;
    
}

SubWindow* UIGetHoverWindow()
{
    return GetHoverWindow(root);
}