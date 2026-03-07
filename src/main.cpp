#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/wglew.h>
#include <windows.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <random>
#include <algorithm>

const int ARRAY_SIZE = 250; 
int rawArray[ARRAY_SIZE];
float heightData[ARRAY_SIZE];

typedef void (*VisualCallback)(int, int, int*);
typedef void (*SortFunc)(int*, int, VisualCallback, VisualCallback);

GLuint shader_programme, vao, heightVBO;
GLint viewLoc, projLoc, activeILoc, activeJLoc, totalBarsLoc, verifyIdxLoc;
float camAngle = 0.0f;
bool isBusy = false; 


float unitCube[] = {
    -0.5f, 0.0f, -0.5f,  0.5f, 0.0f, -0.5f,  0.5f, 1.0f, -0.5f,  0.5f, 1.0f, -0.5f, -0.5f, 1.0f, -0.5f, -0.5f, 0.0f, -0.5f,
    -0.5f, 0.0f,  0.5f,  0.5f, 0.0f,  0.5f,  0.5f, 1.0f,  0.5f,  0.5f, 1.0f,  0.5f, -0.5f, 1.0f,  0.5f, -0.5f, 0.0f,  0.5f,
    -0.5f, 1.0f,  0.5f, -0.5f, 1.0f, -0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f,  0.5f, -0.5f, 1.0f,  0.5f,
     0.5f, 1.0f,  0.5f,  0.5f, 1.0f, -0.5f,  0.5f, 0.0f, -0.5f,  0.5f, 0.0f, -0.5f,  0.5f, 0.0f,  0.5f,  0.5f, 1.0f,  0.5f,
    -0.5f, 0.0f, -0.5f,  0.5f, 0.0f, -0.5f,  0.5f, 0.0f,  0.5f,  0.5f, 0.0f,  0.5f, -0.5f, 0.0f,  0.5f, -0.5f, 0.0f, -0.5f,
    -0.5f, 1.0f, -0.5f,  0.5f, 1.0f, -0.5f,  0.5f, 1.0f,  0.5f,  0.5f, 1.0f,  0.5f, -0.5f, 1.0f,  0.5f, -0.5f, 1.0f, -0.5f
};
const char* vs_source = 
    "#version 400\n"
    "layout (location = 0) in vec3 vp;"
    "layout (location = 1) in float vHeight;"
    "uniform mat4 view; uniform mat4 proj; uniform int activeI; uniform int activeJ; uniform int totalBars; uniform int verifyIdx;"
    "out float hFactor; out float isSelected; out float isVerifying;"
    "void main() {"
    "  float totalWidth = 6.0;" 
    "  float bW = totalWidth / float(totalBars);" 
    "  float x_off = -(totalWidth/2.0) + (gl_InstanceID * bW) + (bW/2.0);"
    "  vec3 pos = vp;"
    "  pos.x *= (bW * 0.9); pos.z *= (bW * 0.9);"
    "  pos.y *= (vHeight * 0.015);"
    "  hFactor = vHeight / 200.0;"
    "  isSelected = (gl_InstanceID == activeI || gl_InstanceID == activeJ) ? 1.0 : 0.0;"
    "  isVerifying = (gl_InstanceID <= verifyIdx) ? 1.0 : 0.0;"
    "  gl_Position = proj * view * vec4(pos.x + x_off, pos.y - 1.5, pos.z, 1.0);"
    "}";

const char* fs_source = 
    "#version 400\n"
    "in float hFactor; in float isSelected; in float isVerifying;"
    "out vec4 fc;"
    "void main() {"
    "  vec3 c = mix(vec3(0.0, 0.1, 0.3), vec3(0.0, 0.8, 1.0), hFactor);"
    "  if(isVerifying > 0.5) c = vec3(0.0, 1.0, 0.2);"
    "  if(isSelected > 0.5) c = vec3(1.0, 1.0, 0.0);"
    "  fc = vec4(c, 1.0);"
    "}";

void setMatrices() {
    // Simple perspective projection and orbiting camera
    float ratio = 1920.0f / 1080.0f;
    float fov = 45.0f;
    float f = 1.0f / tan(fov * 0.5f * 3.14159f / 180.0f);
    float proj[16] = {0};
    proj[0] = f / ratio; proj[5] = f; proj[10] = -1.01f; proj[11] = -1.0f; proj[14] = -0.2f;

    float s = sin(camAngle), c = cos(camAngle);
    float view[16] = {0};
    view[0] = c; view[2] = -s; view[5] = 1.0f; view[8] = s; view[10] = c; view[14] = -7.5f; view[15] = 1.0f;

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
}

void display() {
    // Clear the screen and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);
    glUniform1i(totalBarsLoc, ARRAY_SIZE);
    setMatrices();
    glBindVertexArray(vao);
    // Draw the bars using instanced rendering for efficiency
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, ARRAY_SIZE);
    glutSwapBuffers();
}

void updateCamera() {
    // Calculate delta time for camera rotation
    static int lastTime = 0;
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - lastTime) * 0.001f;
    lastTime = currentTime;
    // Update camera angle for continuous orbiting
    camAngle += 0.5f * deltaTime; 
}

void idle() {
    // Update the camera position when not busy with sorting or reshuffling 
    updateCamera(); 
    if (!isBusy) {
        glutPostRedisplay();
    }
}

void verifyAnimation() {
    int step = (ARRAY_SIZE / 100) + 1;
    // Green highlight to indicate verfication
    for (int i = 0; i < ARRAY_SIZE; i += step) {
        updateCamera();
        glUseProgram(shader_programme);
        glUniform1i(verifyIdxLoc, i);
        Sleep(10);
        display();
        MSG msg; if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    }
   // Sleep(1000);
    glUniform1i(verifyIdxLoc, -1);
    display();
}

void updateVisuals(int i, int j, int* arr) {
    heightData[i] = (float)arr[i];
    heightData[j] = (float)arr[j];
    glBindBuffer(GL_ARRAY_BUFFER, heightVBO);
    glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(float), sizeof(float), &heightData[i]);
    glBufferSubData(GL_ARRAY_BUFFER, j * sizeof(float), sizeof(float), &heightData[j]);
    // Update only tree bars involved in the swap for efficiency
    static int frameCounter = 0;
    if (++frameCounter % 1 == 0) {
        glUseProgram(shader_programme);
        glUniform1i(activeILoc, i);
        glUniform1i(activeJLoc, j);
        updateCamera();
        display();
        MSG msg; if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    }
}

void visualReshuffle() {
    isBusy = true;
    for (int k = 0; k < ARRAY_SIZE; k++) {
        int i = k;
        int j = rand() % ARRAY_SIZE;
        std::swap(rawArray[i], rawArray[j]);
        updateVisuals(i, j, rawArray);
        Sleep(1);
    }
    glUniform1i(activeILoc, -1); glUniform1i(activeJLoc, -1);
    display();
    verifyAnimation();
    isBusy = false;
}

void init() {
    glewInit();
    if (wglSwapIntervalEXT) wglSwapIntervalEXT(0); 
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.01f, 0.01f, 0.02f, 1.0f);
    // Compile shaders
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
    // Get  locations
    viewLoc = glGetUniformLocation(shader_programme, "view");
    projLoc = glGetUniformLocation(shader_programme, "proj");
    activeILoc = glGetUniformLocation(shader_programme, "activeI");
    activeJLoc = glGetUniformLocation(shader_programme, "activeJ");
    totalBarsLoc = glGetUniformLocation(shader_programme, "totalBars");
    verifyIdxLoc = glGetUniformLocation(shader_programme, "verifyIdx");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint vbo;
    // Unit cube VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unitCube), unitCube, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    // Height VBO updated on swaps
    glGenBuffers(1, &heightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, heightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(heightData), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
}

int main(int argc, char** argv) {
    std::vector<int> vals;
    for(int i = 0; i < ARRAY_SIZE; i++) vals.push_back((i * 180 / ARRAY_SIZE) + 10);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(vals.begin(), vals.end(), g);

    for(int i = 0; i < ARRAY_SIZE; i++) {
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
    
    glutKeyboardFunc([](unsigned char k, int x, int y){
        if(k == ' ') { 
            visualReshuffle();
        }
        if(k == 's' || k == 'S') {
            isBusy = true;
            HINSTANCE hLib = LoadLibraryA("algorithm.dll");
            if (hLib) {
                // Get the sorting function from the DLL
                SortFunc sort = (SortFunc)GetProcAddress(hLib, "bubble_sort_basic");
                if (sort) {
                    // Call the sorting function with the visual callback
                    sort(rawArray, ARRAY_SIZE, updateVisuals, nullptr);
                    for(int k=0; k<ARRAY_SIZE; k++) heightData[k] = (float)rawArray[k];
                    glBindBuffer(GL_ARRAY_BUFFER, heightVBO);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(heightData), heightData);
                    glUniform1i(activeILoc, -1); glUniform1i(activeJLoc, -1);
                    display();
                    verifyAnimation();
                }
                // Free the DLL after use
                FreeLibrary(hLib);
            }
            isBusy = false;
        }
    });
    
    glutMainLoop();
    return 0;
}