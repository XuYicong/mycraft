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
chunk*getChunk(int x,int z){
    return chk[(x%50+50)%50]+((z%50+50)%50);
}
char isAir(int id){
    return !(id>>4);
}
int getBlockInPallete(chunkSect*this,int id){
    return this->pallete?this->palle[id]:id;
}
int getBlockInSection(chunkSect*this,int x,int y,int z){
    short bits=this->bits;
    int idx=(((y<<4)+z)<<4)+x;//with increasing x coordinates, within rows of increasing z coordinates, within layers of increasing y coordinates. 
    idx*=bits;
    int pos=idx&0x3f, index=idx>>6;
    int id=this->palle[index]>>pos;
    if((pos+bits)&(1<<6)){
        id|=this->palle[index+1]<<(64-pos);
    }
    return getBlockInPallete(this,id&((1<<bits)-1));
}
int getBlockInChunk(chunk*this,int x,int y,int z){
    int sectionId=y>>4;
    if(this->mask&(1<<sectionId)){
        return getBlockInSection(this->section+sectionId,x,y&0xf,z);
    }else return 0;//Air
}
int getBlock(int x,int y,int z){
    int chunkX=floor(x/16.0), chunkZ=floor(z/16.0);
    return getBlockInChunk(getChunk(chunkX,chunkZ),x-(chunkX<<4),y,z-(chunkZ<<4));
}
