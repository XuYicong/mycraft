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
int unloadAllChunks(){
    for(int i=0;i<50;++i){
        for(int j=0;j<50;++j){
            chk[i][j].loaded=0;
        }
    }
    return 0;
}
char isAir(int id){
    return !(id/*>>4*/);//TODO: cave air
}
int getBlockInPallete(chunkSect*this,int id){
    if(this->pallete){
        if(id>this->palleSize)printf("error: block id %d exceeds pallete size %d!\n",id,this->palleSize);
        return this->palle[id];
    }
    return id;
}
int getBlockInSection(chunkSect*this,int x,int y,int z){
    short bits=this->bits;
    int idx=(((y<<4)+z)<<4)+x;//with increasing x coordinates, within rows of increasing z coordinates, within layers of increasing y coordinates. 
    if(idx>=4096)puts("Invalid relative coordinates in section");
    idx*=bits;//order of bits in the array
    //Order of bits in the long and order of the long in the array
    int pos=idx&0x3f, index=idx>>6;//Since a long is 64 bits aka 1<<6
    unsigned int id=this->data[index]>>pos;//Cut out less significant bits
    if((pos+bits)>64){//if right open order of this block exceeds 64
        id|=this->data[index+1]<<(64-pos);
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
    return getBlockInChunk(getChunk(chunkX,chunkZ),x-(chunkX*16),y,z-(chunkZ*16));
}
