#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <GL/glew.h>
using namespace glm;
extern "C" {void mats(int);}
extern "C" void mats(int programID){
    mat4 model=translate(vec3(0,0,0)),view=lookAt(vec3(2,-2,-1),vec3(-1,2,1),vec3(0,-1,0)),projection=perspective(55.5,1.6,0.1,233.3);
    mat4 mvp=projection*view*model;
    GLuint MatrixID = glGetUniformLocation(programID, "mvp");

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
}
