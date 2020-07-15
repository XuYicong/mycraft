#include "objects.h"
int moveEntity(entity*tt){
                    float pos[3];
                    for(int i=0;i<3;++i){
                        pos[i]=((short)rd(2))/128.0/32.0;
                    }
                    //printf("Entity %d(%s) moves from (%f,%f,%f) ",tt->id,nttyp[tt->type],tt->x,tt->y,tt->z);
                    tt->x+=pos[0];tt->y+=pos[1];tt->z+=pos[2];
                    //printf("to (%f,%f,%f)\n",tt->x,tt->y,tt->z);
                    return 0;
}
int rotateEntity(entity*tt){
    tt->yaw=rcv[cur++];
    tt->pitch=rcv[cur++];//New angle, not a delta
    //printf("Pitch: %d/256\tYaw: %d/256\n",tt->pitch,tt->yaw);
    return 0;
}
int teleportEntity(entity*mob){
    double X,Y,Z;
    rdF((char*)&X,8);rdF((char*)&Y,8);rdF((char*)&Z,8);mob->x=X,mob->y=Y,mob->z=Z;
    printf("Position: (%lf,%lf,%lf)\n",X,Y,Z);
    rotateEntity(mob);
    return 0;
}
