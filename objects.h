#include <stdio.h>
#include <time.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
typedef enum GlobalGameState{
    PING,
    LOGIN,
    PLAY,
    EXIT
} GameState;
typedef struct chunkSection{
    short num,pallete,bits,numVertices,palleSize;
    unsigned long long data[64*14+3];
    int palle[1<<8];
    GLuint vao,vbo;
} chunkSect;
typedef struct Chunk{
    char loaded,rendered;
    int x,z,mask;
    chunkSect section[16];
} chunk;
typedef struct Block{
    int x,y,z;
} block;
extern chunk chk[50][50];
extern GameState gameState;
extern unsigned char sd[203333],rcv[2033456];
extern char ct;//0 if user want to input a chat
extern const double Pi;
extern float playerYaw,playerPitch;
void mats_mvp(int,double px,double py,double pz,double x,double y,double z,double,double);
void mats_vp(int,double x,double y,double z,double,double);
unsigned long long rd(int);
void rdF(char*target,int numBytes);
void draw(void);//render a frame
void play(void);//game loop
void move();
int networkThread(char*data);//network loop
extern int cur,tick;
typedef struct Entity{//id < 0 means empty
    char onGround;
    short id,type;
    short vx,vy,vz;
    short pitch,yaw,headPitch,headYaw;
    double x,y,z;
    char data[256];
} entity;
extern entity*player;
extern int moveEntity(entity*);
extern int teleportEntity(entity*);
extern int rotateEntity(entity*);
extern chunk*getChunk(int x,int z);
extern int unloadAllChunks();
extern char isAir(int blockId);
extern int getBlock(int x,int y,int z);
extern char nttyp[123][123];
