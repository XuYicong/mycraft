#version 130
// It was expressed that some drivers required this next line to function properly
precision highp float;
 
in  vec4 ex_Color;
out vec4 color;
//uniform vec4 blockColor;
void main(void) {
    //gl_FragColor = vec4(ex_Color);
    //color=vec4(0.4,0.4,0.2,0.2);
    color=ex_Color;
    //color=blockColor;
}
