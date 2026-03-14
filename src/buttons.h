#ifndef BUTTONS_H
#define BUTTONS_H

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#include <vector>

struct Button
{
    float x, y, w, h;
    std::string label;
    int actionID;// 1 for quicksort, 2 for bubble sort, etc.
};
struct Slider
{
    float x, y, w, h;
    float pos; // 0.0 - 1.0
    std::string label;
    int sliderID; // 1 for size, 2 for duration
};

std::vector<Button> guiButtons;
std::vector<Slider> guiSliders;
void initSliders(int winW, int winH)
{
    guiSliders.clear();
    // slides on the right and left centered vertically
    guiSliders.push_back({50, 200, 30, 300, 0.65f, "Array Size", 1}); // id 1: Array Size
    guiSliders.push_back({950, 200, 30, 300, 0.0f, "   Delay", 2});      // id 2: Delay
}
void initButtons(int winW, int winH)
{
    guiButtons.clear();
    // buttons centered at the bottom
    guiButtons.push_back({350, 700, 150, 40, "QuickSort", 1});
    guiButtons.push_back({524, 700, 150, 40, "BubbleSort", 2});
}
//init everything
void initUI(int winW, int winH)
{
    initButtons(winW, winH);
    initSliders(winW, winH);
}
void drawButtons()
{
    for (auto &b : guiButtons)
    {
        // background buttons
        glColor3f(0.02f, 0.02f, 0.1f);
        glBegin(GL_QUADS);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
        glEnd();
        // border
        glColor3f(0.0f, 0.8f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
        glEnd();
        // Text
        glRasterPos2f(b.x + 25, b.y + 25);
        for (char c : b.label)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, (int)c);
    }
}

void drawSliders()
{
    for (auto &s : guiSliders)
    {
        // Track , that vertical line
        glColor3f(0.1f, 0.1f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(s.x + s.w / 2 - 2, s.y);
        glVertex2f(s.x + s.w / 2 + 2, s.y);
        glVertex2f(s.x + s.w / 2 + 2, s.y + s.h);
        glVertex2f(s.x + s.w / 2 - 2, s.y + s.h);
        glEnd();

        // Handle, the square 
        float handleY = s.y + (1.0f - s.pos) * s.h;
        glColor3f(0.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(s.x, handleY - 12);
        glVertex2f(s.x + s.w, handleY - 12);
        glVertex2f(s.x + s.w, handleY + 12);
        glVertex2f(s.x, handleY + 12);
        glEnd();

        // Text
        glRasterPos2f(s.x - 5, s.y + s.h + 20);
        for (char c : s.label)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, (int)c);
    }
}

void drawGUI(int winW, int winH)
{
    glDisable(GL_DEPTH_TEST);
    glUseProgram(0);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1024, 768, 0); //basic window size , the coordinates will be scaled in main
    //hopefully won t have the bright idea to resize the initial window size from main and break everything :)
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    drawButtons();
    drawSliders();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}
// returns the actionID of the button clicked or -1 if no button was clicked
int getButtonClick(int mx, int my)
{
    for (auto &b : guiButtons)
    {
        if (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h)
        {
            return b.actionID;
        }
    }
    return -1;
}
#endif