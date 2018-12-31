#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <GL/glew.h>
using namespace glm;
double Pi=3.141592653589793;
//extern "C" {void mats(int);}
extern "C" void mats(int programID,double tx,double ty,double tz,double playerX,double playerY,double playerZ,double yaw,double pitch){
    //while(yaw>=360)yaw-=360;
    yaw*=Pi/180;pitch*=Pi/180;
    playerY+=1.76;
    double lookX=cos(yaw)*cos(pitch),lookZ=sin(yaw)*cos(pitch),lookY=sin(pitch);
    mat4 model=translate(vec3(tx,ty,tz)),view=lookAt(vec3(playerX,playerY,playerZ),vec3(playerX+lookX,playerY+lookY,playerZ+lookZ),vec3(0,1,0)),projection=perspective(51.9,1.6,0.1,233.3);
    mat4 mvp=projection*view*model;
    GLuint MatrixID = glGetUniformLocation(programID, "mvp");

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
}
