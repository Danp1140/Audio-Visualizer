#version 410 core
in vec4 fragcol;
in float x;

out vec4 color;

//uniform vec3 colorIn;
void main() {
    color=fragcol;
//    color=vec4(fragcol.xyz, 0.5);
//    color=vec3(0, x, 1);
}
