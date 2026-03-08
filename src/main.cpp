#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/wglew.h>
#include <windows.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <random>
#include <algorithm>

const int ARRAY_SIZE = 1000;
int rawArray[ARRAY_SIZE];
float heightData[ARRAY_SIZE];
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


float unitCube[] = {
    -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 1.0f, -0.5f, 0.5f, 1.0f, -0.5f, -0.5f, 1.0f, -0.5f, -0.5f, 0.0f, -0.5f,
    -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 1.0f, 0.5f, -0.5f, 1.0f, 0.5f, -0.5f, 0.0f, 0.5f,
    -0.5f, 1.0f, 0.5f, -0.5f, 1.0f, -0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 1.0f, 0.5f,
    0.5f, 1.0f, 0.5f, 0.5f, 1.0f, -0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 1.0f, 0.5f,
    -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f,
    -0.5f, 1.0f, -0.5f, 0.5f, 1.0f, -0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 1.0f, 0.5f, -0.5f, 1.0f, 0.5f, -0.5f, 1.0f, -0.5f};

const char *vs_source =
    "#version 400\n"
    "layout (location = 0) in vec3 vp;"
    "layout (location = 1) in float vHeight;"
    "uniform mat4 view; uniform mat4 proj; uniform int activeI; uniform int activeJ; "
    "uniform int totalBars; uniform int verifyIdx; uniform int opType;"
    "out float hFactor; out float isI; out float isJ; out float isVerifying; out float vOpType;"
    "void main() {"
    "  float totalWidth = 6.0;"
    "  float bW = totalWidth / float(totalBars);"
    "  float x_off = -(totalWidth/2.0) + (gl_InstanceID * bW) + (bW/2.0);"
    "  vec3 pos = vp; pos.x *= (bW * 0.9); pos.z *= (bW * 0.9); pos.y *= (vHeight * 0.015);"
    "  hFactor = vHeight / 200.0;"
    "  isI = (gl_InstanceID == activeI) ? 1.0 : 0.0;"
    "  isJ = (gl_InstanceID == activeJ) ? 1.0 : 0.0;"
    "  isVerifying = (gl_InstanceID <= verifyIdx) ? 1.0 : 0.0;"
    "  vOpType = float(opType);"
    "  gl_Position = proj * view * vec4(pos.x + x_off, pos.y - 1.5, pos.z, 1.0);"
    "}";

const char *fs_source =
    "#version 400\n"
    "in float hFactor; in float isI; in float isJ; in float isVerifying; in float vOpType;"
    "out vec4 fc;"
    "void main() {"
    "  vec3 c = mix(vec3(0.0, 0.1, 0.3), vec3(0.0, 0.8, 1.0), hFactor);"
    "  "
    "  if(vOpType > 0.5 && vOpType < 1.5) {"
    "    if(isI > 0.5 || isJ > 0.5) c = vec3(0.0, 1.0, 0.0);"
    "  }"
    "  else if(vOpType > 1.5) {"
    "    if(isI > 0.5) c = vec3(1.0, 1.0, 0.0);"
    "    if(isJ > 0.5) c = vec3(1.0, 0.0, 1.0);"
    "  }"
    "  "
    "  if(isVerifying > 0.5) c = vec3(0.5, 1.0, 0.5);"
    "  "
    "  fc = vec4(c, 1.0);"
    "}";
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
    sprintf(buffer, " Elements: 1000 | Comparisons: %ld | Swaps: %ld | Time: %.2f s", comparisons, swaps, currentElapsed);

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
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);
    glUniform1i(totalBarsLoc, ARRAY_SIZE);
    setMatrices();
    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, ARRAY_SIZE);
    glUseProgram(0);
    drawHUD();
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

    int step = (ARRAY_SIZE / 100) + 1;
    for (int i = 0; i < ARRAY_SIZE; i += step)
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
    for (int k = 0; k < ARRAY_SIZE; k++)
    {
        int i = k;
        int j = rand() % ARRAY_SIZE;
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
    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.01f, 0.01f, 0.02f, 1.0f);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_source, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_source, NULL);
    glCompileShader(fs);

    shader_programme = glCreateProgram();
    glAttachShader(shader_programme, vs);
    glAttachShader(shader_programme, fs);
    glLinkProgram(shader_programme);
    // shader error checking and debugging
    int success;
    char infoLog[512];
    glGetProgramiv(shader_programme, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shader_programme, 512, NULL, infoLog);
        std::cout << "[ERROR] Shader Link Failed:\n"
                  << infoLog << std::endl;
    }
    else
    {
        std::cout << "[SUCCESS] Shaders Linked Perfectly!" << std::endl;
    }
    viewLoc = glGetUniformLocation(shader_programme, "view");
    projLoc = glGetUniformLocation(shader_programme, "proj");
    activeILoc = glGetUniformLocation(shader_programme, "activeI");
    activeJLoc = glGetUniformLocation(shader_programme, "activeJ");
    totalBarsLoc = glGetUniformLocation(shader_programme, "totalBars");
    verifyIdxLoc = glGetUniformLocation(shader_programme, "verifyIdx");
    opTypeLoc = glGetUniformLocation(shader_programme, "opType");
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unitCube), unitCube, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &heightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, heightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(heightData), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
}

int main(int argc, char **argv)
{
    std::vector<int> vals;
    for (int i = 0; i < ARRAY_SIZE; i++)
        vals.push_back((i * 180 / ARRAY_SIZE) + 10);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(vals.begin(), vals.end(), g);

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        rawArray[i] = vals[i];
        heightData[i] = (float)vals[i];
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("SPG Final - Orbit & Reshuffle");

    init();

    glutDisplayFunc(display);
    glutIdleFunc(idle);

    glutKeyboardFunc([](unsigned char k, int x, int y)
                     {
        if(k == ' ') { 
            visualReshuffle();
        }
        if(k == 's' || k == 'S') {
            comparisons = 0;
    swaps = 0;
    sortStartTime = glutGet(GLUT_ELAPSED_TIME);
    sortingActive = true;
            isBusy = true;
            HINSTANCE hLib = LoadLibraryA("algorithm.dll");
            if (hLib) {
                SortFunc sort = (SortFunc)GetProcAddress(hLib, "quick_sort_basic");
                if (sort) {
                    sort(rawArray, ARRAY_SIZE, updateVisuals, updateCompare);
                    for(int k=0; k<ARRAY_SIZE; k++) heightData[k] = (float)rawArray[k];
                    glBindBuffer(GL_ARRAY_BUFFER, heightVBO);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(heightData), heightData);
                    glUniform1i(activeILoc, -1); glUniform1i(activeJLoc, -1);
                    resetVisuals();
                    verifyAnimation();
                    display();
                    sortingActive = false;
                }
                
                FreeLibrary(hLib);
            }
            isBusy = false;
        } });

    glutMainLoop();
    return 0;
}