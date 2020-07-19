#version 130
// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
in vec3 in_Position;
in vec4 in_Color;
 
// We output the ex_Color variable to the next shader in the chain
out vec4 ex_Color;
uniform mat4 mvp;
void main(void) {
    // Since we are using flat lines, our input only had two points: x and y.
    // Set the Z coordinate to 0 and W coordinate to 1
    gl_Position = mvp*vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
 
    //Let adjacent vertices have different color
    int int_Color[3];
    int_Color[0]=int(in_Position.x);
    int_Color[1]=int(in_Position.y);
    int_Color[2]=int(in_Position.z);

    int_Color[0]%=3;
    int_Color[1]%=3;
    int_Color[2]%=3;

    // Pass the color on to the fragment shader
    ex_Color = vec4(int_Color[0]/3.0+0.1,int_Color[1]/3.0+0.1,int_Color[2]/3.0+0.1,0.9);
}
