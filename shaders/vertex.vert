#version 400
layout (location = 0) in vec3 vp;
layout (location = 1) in float vHeight;

uniform mat4 view;
uniform mat4 proj;
uniform int activeI;
uniform int activeJ;
uniform int totalBars;
uniform int verifyIdx;
uniform int opType;

out float hFactor;
out float isI;
out float isJ;
out float isVerifying;
out float vOpType;

void main() {
    float totalWidth = 6.0;
    float bW = totalWidth / float(totalBars);
    float x_off = -(totalWidth/2.0) + (gl_InstanceID * bW) + (bW/2.0);
    vec3 pos = vp;
    pos.x *= (bW * 0.9);
    pos.z *= (bW * 0.9);
    pos.y *= (vHeight * 0.015);
    hFactor = vHeight / 200.0;
    isI = (gl_InstanceID == activeI) ? 1.0 : 0.0;
    isJ = (gl_InstanceID == activeJ) ? 1.0 : 0.0;
    isVerifying = (gl_InstanceID <= verifyIdx) ? 1.0 : 0.0;
    vOpType = float(opType);
    gl_Position = proj * view * vec4(pos.x + x_off, pos.y - 1.5, pos.z, 1.0);
}