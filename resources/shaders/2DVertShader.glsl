#version 410 core
layout(location=0) in vec2 vertpos;
layout(location=1) in vec4 vertcol;

out vec4 fragcol;
out float x;

void main() {
    vec2 vertposnew=vertpos-vec2(400, 300);
    vertposnew/=vec2(400, 300);
    gl_Position=vec4(vertposnew, 0.0, 1.0);
    x=(vertposnew.x+1)/2;
    fragcol=vertcol;
//    gl_Position=vec4(ver)
}
