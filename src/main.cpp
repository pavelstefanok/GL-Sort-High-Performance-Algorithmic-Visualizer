#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/wglew.h>
#include <windows.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <random>
#include <algorithm>
#include "buttons.h"
#include <fstream>
#include <string>

int currentSize = 500; // default size
int activeSliderID = -1; // 1 for size, 2 for duration
int rawArray[2000];
float heightData[2000];
double duration = 0; // nr of miliseconds for step

long comparisons = 0;
long swaps = 0;
double sortStartTime = 0;
double currentElapsed = 0;
bool sortingActive = false;
typedef void (*VisualCallback)(int, int, int *);
typedef void (*SortFunc)(int *, int, VisualCallback, VisualCallback);

GLuint shader_programme, vao, heightVBO;
GLint viewLoc, projLoc, activeILoc, activeJLoc, totalBarsLoc, verifyIdxLoc, opTypeLoc;
float camAngle = 0.0f;
bool isBusy = false;
void updateVisuals(int i, int j, int *arr);
void updateCompare(int i, int j, int *arr);
void resetVisuals();
void verifyAnimation();
void display();
void init();
float unitCube[] = {
    -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 1.0f, -0.5f, 0.5f, 1.0f, -0.5f, -0.5f, 1.0f, -0.5f, -0.5f, 0.0f, -0.5f,
    -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 1.0f, 0.5f, -0.5f, 1.0f, 0.5f, -0.5f, 0.0f, 0.5f,
    -0.5f, 1.0f, 0.5f, -0.5f, 1.0f, -0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 1.0f, 0.5f,
    0.5f, 1.0f, 0.5f, 0.5f, 1.0f, -0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 1.0f, 0.5f,
    -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f,
    -0.5f, 1.0f, -0.5f, 0.5f, 1.0f, -0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 1.0f, 0.5f, -0.5f, 1.0f, 0.5f, -0.5f, 1.0f, -0.5f};
// utility function to read shader files
std::string textFileRead(const char *fn)
{
    std::ifstream ifile(fn);
    std::string filetext;
    while (ifile.good())
    {
        std::string line;
        std::getline(ifile, line);
        filetext.append(line + "\n");
    }
    return filetext;
}
void setMatrices()
{
    float ratio = 1024.0f / 768.0f;
    float fov = 45.0f;
    float f = 1.0f / tan(fov * 0.5f * 3.14159f / 180.0f);
    float proj[16] = {0};
    proj[0] = f / ratio;
    proj[5] = f;
    proj[10] = -1.01f;
    proj[11] = -1.0f;
    proj[14] = -0.2f;

    float s = sin(camAngle), c = cos(camAngle);
    float view[16] = {0};
    view[0] = c;
    view[2] = -s;
    view[5] = 1.0f;
    view[8] = s;
    view[10] = c;
    view[14] = -7.5f;
    view[15] = 1.0f;

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
}

void drawHUD()
{
    glUseProgram(0);
    // Set up orthographic projection for 2D HUD overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1024, 0, 768);
    // Reset modelview for HUD
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    // Disable depth test for HUD
    glColor3f(1.0f, 1.0f, 1.0f);
    // Draw text with current stats
    char buffer[128];
    sprintf(buffer, " Elements: %d | Comparisons: %ld | Swaps: %ld | Time: %.2f s", currentSize, comparisons, swaps, currentElapsed);

    glRasterPos2i(20, 740);
    for (const char *c = buffer; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    // Restore previous projection and modelview
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    // Re-enable depth test for 3D rendering
    glEnable(GL_DEPTH_TEST);
}
void generateData(int size)
{
    currentSize = size;
    std::vector<int> vals;
    for (int i = 0; i < size; i++)
    {

        vals.push_back((i * 180 / size) + 10);
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(vals.begin(), vals.end(), g);

    for (int i = 0; i < size; i++)
    {
        rawArray[i] = vals[i];
        heightData[i] = (float)vals[i];
    }

    glBindBuffer(GL_ARRAY_BUFFER, heightVBO);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), heightData, GL_DYNAMIC_DRAW);
    comparisons = 0;
    swaps = 0;
    currentElapsed = 0;
    sortingActive = false;
    display();
}

void mouseFunc(int button, int state, int x, int y)
{
    // base width and in case the window gets resized we just 
    // calculate the coordinates based on the original 1024x768 ratio
    const float BASE_W = 1024.0f;
    const float BASE_H = 768.0f;
    
    // real window size
    int winW = glutGet(GLUT_WINDOW_WIDTH);
    int winH = glutGet(GLUT_WINDOW_HEIGHT);
    
    // calculate virtual coordinates based on the original ratio
    int virtualX = (int)(x * (BASE_W / winW));
    int virtualY = (int)(y * (BASE_H / winH));

    if (button == GLUT_LEFT_BUTTON) 
    {
        if (state == GLUT_DOWN) 
        {
            
            for (auto& s : guiSliders) {
                if (virtualX >= s.x && virtualX <= s.x + s.w && 
                    virtualY >= s.y && virtualY <= s.y + s.h) 
                {
                    activeSliderID = s.sliderID;
                    std::cout << "Slider: " << s.label << std::endl;
                    return; //no need to check buttons if we clicked on a slider
                        
                }
            }

            
            int action = getButtonClick(virtualX, virtualY);

            //for debugging output for clicks
            std::cout << "Click at (" << virtualX << ", " << virtualY << ") Action: " << action << std::endl;

            if (action == -1)
                return;

            if (isBusy)
            {
                std::cout << "busy" << std::endl;
                return;
            }

            if (action == 1 || action == 2)
            {
                isBusy = true;
                sortingActive = true;
                comparisons = 0;
                swaps = 0;
                sortStartTime = glutGet(GLUT_ELAPSED_TIME);
                // load the sorting function from the DLL depending on the button clicked
                HINSTANCE hLib = LoadLibraryA("algorithm.dll");
                if (hLib)
                {
                    const char *fName = (action == 1) ? "quick_sort_iterative" : "bubble_sort_basic";
                    SortFunc sort = (SortFunc)GetProcAddress(hLib, fName);
                    // call the sorting function with the raw array and visual callbacks
                    if (sort)
                    {
                        std::cout << "sorting: " << fName << std::endl;
                        sort(rawArray, currentSize, updateVisuals, updateCompare);
                        resetVisuals();
                        verifyAnimation();
                    }
                    else
                    {
                        //some debugging
                        std::cout << "wrong name function " << fName << std::endl;
                    }
                    FreeLibrary(hLib);
                }
                else
                {
                    std::cout << "can t find dll boss" << std::endl;
                }
                isBusy = false;
                sortingActive = false;
            }
            else if (action >= 10)
            {
                generateData(action);
            }
        }
        else if (state == GLUT_UP) 
        {
            //if we release mouse button we stop moving the slider
            activeSliderID = -1;
        }
    }
}
void motionFunc(int x, int y) {
    //if no slider is pressed, we don t care about mouse movement
    if (activeSliderID == -1) return; 

    float winW = (float)glutGet(GLUT_WINDOW_WIDTH);
    float winH = (float)glutGet(GLUT_WINDOW_HEIGHT);
    
    float virtualX = x * (1024.0f / winW);
    float virtualY = y * (768.0f / winH);

    for (auto& s : guiSliders) {
        if (s.sliderID == activeSliderID) {
            s.pos = 1.0f - ((virtualY - s.y) / s.h); 
            
            //for the slider to not go into space or the depths of hell (outside the track)
            //tested and actually the slider just crashes the program if it goes outside the track :)
            if (s.pos < 0) s.pos = 0; 
            if (s.pos > 1) s.pos = 1;

            if (s.sliderID == 1) { // Size
                int newSize = 10 + (int)(s.pos * s.pos * 990); // between 10 and 1000 with exponential for better control at lower values
                if (newSize != currentSize) {
                    generateData(newSize); 
                }
            } else if (s.sliderID == 2) {
                //delay exponential (again for better control)
                duration = s.pos * s.pos * 1000.0; 
            }
        }
    }
    
    //redraw with new slider position
    glutPostRedisplay();
}

void display()
{   // Clear the screen and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);
    glUniform1i(totalBarsLoc, currentSize);
    setMatrices();
    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, currentSize);
    glUseProgram(0);
    drawHUD();
    drawGUI(1024, 768);
    glutSwapBuffers();
}

void updateCamera()
{
    static int lastTime = 0;
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - lastTime) * 0.001f;
    lastTime = currentTime;
    camAngle += 0.5f * deltaTime;
}

void idle()
{
    updateCamera();
    if (!isBusy)
    {
        glutPostRedisplay();
    }
}
void resetVisuals()
{
    // the green line was getting stuck on the last compared/swapped bars
    // because of the drawhud function
    // so we reset them to -1 to avoid confusion
    glUseProgram(shader_programme);

    glUniform1i(activeILoc, -1);
    glUniform1i(activeJLoc, -1);
    glUniform1i(opTypeLoc, 0);
    glUniform1i(verifyIdxLoc, -1);

    display();
}

void verifyAnimation()
{

    int step = (currentSize / 100) + 1;
    for (int i = 0; i < currentSize; i += step)
    {
        updateCamera();
        glUseProgram(shader_programme);
        glUniform1i(verifyIdxLoc, i);
        Sleep(10);
        display();
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    // Sleep(1000);
    glUniform1i(verifyIdxLoc, -1);
    display();
    resetVisuals();
}

void updateCompare(int i, int j, int *arr)
{
    comparisons++;
    if (sortingActive)
    {
        currentElapsed = (glutGet(GLUT_ELAPSED_TIME) - sortStartTime) / 1000.0;
    }
    // printf("COMPARE:  %d and %d\n", i, j);
    glUseProgram(shader_programme);
    glUniform1i(activeILoc, i);
    glUniform1i(activeJLoc, j);
    glUniform1i(opTypeLoc, 2);

    double startTime = glutGet(GLUT_ELAPSED_TIME);
    if (duration)
        while (glutGet(GLUT_ELAPSED_TIME) - startTime < duration)
        {
            updateCamera();
            display();

            MSG msg;
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    else
    {
        updateCamera();
        display();

        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void updateVisuals(int i, int j, int *arr)
{
    swaps++;
    if (sortingActive)
    {
        currentElapsed = (glutGet(GLUT_ELAPSED_TIME) - sortStartTime) / 1000.0;
    }
    heightData[i] = (float)arr[i];
    heightData[j] = (float)arr[j];
    // printf("SWAP: %d and %d\n", i, j);
    glBindBuffer(GL_ARRAY_BUFFER, heightVBO);
    glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(float), sizeof(float), &heightData[i]);
    glBufferSubData(GL_ARRAY_BUFFER, j * sizeof(float), sizeof(float), &heightData[j]);
    // Highlight the swapped bars in green
    glUseProgram(shader_programme);
    glUniform1i(activeILoc, i);
    glUniform1i(activeJLoc, j);
    glUniform1i(opTypeLoc, 1); // Swap
    double startTime = glutGet(GLUT_ELAPSED_TIME);
    if (duration)
        while (glutGet(GLUT_ELAPSED_TIME) - startTime < duration)
        {
            updateCamera();
            display();
            MSG msg;
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    else
    {
        updateCamera();
        display();

        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void visualReshuffle()
{
    isBusy = true;
    for (int k = 0; k < currentSize; k++)
    {
        int i = k;
        int j = rand() % currentSize;
        std::swap(rawArray[i], rawArray[j]);
        updateVisuals(i, j, rawArray);
        Sleep(1);
    }
    glUniform1i(activeILoc, -1);
    glUniform1i(activeJLoc, -1);
    display();
    verifyAnimation();
    isBusy = false;
}

void init()
{
    glewInit();
    // v-sync off for performance
    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.01f, 0.01f, 0.02f, 1.0f);

    //compile shaders

    
    std::string vstext = textFileRead("shaders/vertex.vert");
    std::string fstext = textFileRead("shaders/fragment.frag");
    const char *vs_ptr = vstext.c_str();
    const char *fs_ptr = fstext.c_str();

    // Vertex Shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_ptr, NULL);
    glCompileShader(vs);

    //debugging
    GLint status = GL_FALSE;
    int infoLogLength;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> errMsg(infoLogLength + 1);
        glGetShaderInfoLog(vs, infoLogLength, NULL, &errMsg[0]);
        std::cerr << "[VERTEX ERROR]: " << &errMsg[0] << std::endl;
    }

    // Fragment Shader
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_ptr, NULL);
    glCompileShader(fs);

    //debugging
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> errMsg(infoLogLength + 1);
        glGetShaderInfoLog(fs, infoLogLength, NULL, &errMsg[0]);
        std::cerr << "[FRAGMENT ERROR]: " << &errMsg[0] << std::endl;
    }

    // Link Shaders
    shader_programme = glCreateProgram();
    glAttachShader(shader_programme, vs);
    glAttachShader(shader_programme, fs);
    glLinkProgram(shader_programme);

    //debugging
    GLint linkStatus;
    glGetProgramiv(shader_programme, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        glGetProgramiv(shader_programme, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> errMsg(infoLogLength + 1);
        glGetProgramInfoLog(shader_programme, infoLogLength, NULL, &errMsg[0]);
        std::cerr << "[LINK ERROR]: " << &errMsg[0] << std::endl;
    }

    // Get uniform locations for everything
    viewLoc = glGetUniformLocation(shader_programme, "view");
    projLoc = glGetUniformLocation(shader_programme, "proj");
    activeILoc = glGetUniformLocation(shader_programme, "activeI");
    activeJLoc = glGetUniformLocation(shader_programme, "activeJ");
    totalBarsLoc = glGetUniformLocation(shader_programme, "totalBars");
    verifyIdxLoc = glGetUniformLocation(shader_programme, "verifyIdx");
    opTypeLoc = glGetUniformLocation(shader_programme, "opType");

    //set up VAO and Vbo
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unitCube), unitCube, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // setup vbo for heights
    glGenBuffers(1, &heightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, heightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(heightData), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    // generate initial data
    initUI(1024, 768);
    visualReshuffle();
}

int main(int argc, char **argv)
{   
    // generate initial data for 1000 elements
    std::vector<int> vals;
    for (int i = 0; i < currentSize; i++)
        vals.push_back((i * 180 / currentSize) + 10);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(vals.begin(), vals.end(), g);

    for (int i = 0; i < currentSize; i++)
    {
        rawArray[i] = vals[i];
        heightData[i] = (float)vals[i];
    }
    // init glut and create window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("SPG Final - Orbit & Reshuffle");
    init();
    // getting mouse and keyboard callbacks from glut
    glutMouseFunc(mouseFunc);
    glutMotionFunc(motionFunc);

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    //if space is pressed reshuffle the array and visualize it
    glutKeyboardFunc([](unsigned char k, int x, int y)
                     {
        if(k == ' ') { 
            visualReshuffle();
        }
    });

    glutMainLoop();
    return 0;
}