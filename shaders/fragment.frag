#version 400
in float hFactor;
in float isI;
in float isJ;
in float isVerifying;
in float vOpType;
out vec4 fc;

void main() {
    vec3 c = mix(vec3(0.0, 0.1, 0.3), vec3(0.0, 0.8, 1.0), hFactor);
    if(vOpType > 0.5 && vOpType < 1.5) { // Swap
        if(isI > 0.5 || isJ > 0.5) c = vec3(0.0, 1.0, 0.0);
    }
    else if(vOpType > 1.5) { // Compare
        if(isI > 0.5) c = vec3(1.0, 1.0, 0.0);
        if(isJ > 0.5) c = vec3(1.0, 0.0, 1.0);
    }
    if(isVerifying > 0.5) c = vec3(0.5, 1.0, 0.5);
    fc = vec4(c, 1.0);
}