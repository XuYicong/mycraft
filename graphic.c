#include "objects.h"
GameState gameState=LOGIN;
int shaderProgram,vertID,fragID;
SDL_Window*win;
double cubes[23334],blocks[233345];
int cubect,blockct;
int flor=2333;
int d[3][6]={{-1,1,0,0,0,0},{0,0,0,0,-1,1},{0,0,-1,1,0,0}};
int *dx=d[0], *dy=d[1], *dz=d[2];
GLfloat cube[]={
    -.5,-.5,-.5,-.5,-.5,.5,
    -.5,.5,-.5,-.5,.5,.5,
    .5,-.5,-.5,.5,-.5,.5,
    .5,.5,-.5,.5,.5,.5};
int vertIdx[6][4]={
    {0,1,3,2},{4,5,7,6},
    {0,2,6,4},{1,3,7,5},
    {0,1,5,4},{2,3,7,6}};
int rerenderChunk(chunk*ck){
    if(!ck->loaded)return -1;
    for(int i=0;i<4;++i)if(!getChunk(ck->x+dx[i],ck->z+dz[i])->loaded)return -2;
    //Send chunk geometry to GPU
    ck->rendered=1;
    GLfloat *vert=NULL;
    int vertSize=0;
    int baseX=ck->x<<4, baseZ=ck->z<<4;
    //puts("rerender chunk");
    for(int i=0;i<16;++i){//For each section
        if(((1<<i)&ck->mask)==0)continue;
        glGenVertexArrays(1, &ck->section[i].vao);
        glBindVertexArray(ck->section[i].vao);
        glGenBuffers(1, &ck->section[i].vbo);
        glBindBuffer(GL_ARRAY_BUFFER, ck->section[i].vbo);
        int baseY=i<<4;
        for(int y=0;y<16;++y){
            int Y=baseY+y;
            for(int z=0;z<16;++z){
                int Z=baseZ+z;
                for(int x=0;x<16;++x){//For each block in the section
                    int X=baseX+x;//Could be more efficient
                    //puts("是air吗?");
                    int blockId=getBlock(X,Y,Z);
                    if(isAir(blockId))continue;
                    printf("\t(%d,%d,%d):%d:%d\n",X,Y,Z,blockId>>4,blockId&0xf);
                    //puts("不是");
                    int cood[]={X,Y,Z};
                    for(int m=0;m<6;++m){//For each surface of the block
                        if(!isAir(getBlock(X+dx[m],Y+dy[m],Z+dz[m])))continue;
                        int pos=vertSize;
                        vert=(GLfloat*)realloc(vert,(vertSize+=12)* sizeof(GLfloat));
                        if(vert==NULL){
                            gameState=EXIT;
                            exit(-1);
                        }
                        for(int i=0;i<4;++i){
                            for(int j=0;j<3;++j){
                                vert[pos++]=cood[j]+cube[vertIdx[m][i]*3+j];
                            }
                        }
                    }
                }
            }
        }
        ck->section[i].numVertices=vertSize/3;
        //printf("(%d,%d,%d):%d个面\n",baseX,baseY,baseZ,vertSize/12);
        glBufferData(GL_ARRAY_BUFFER, vertSize*sizeof(GLfloat), vert, GL_STATIC_DRAW);
        free(vert);vert=NULL;vertSize=0;
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
        glEnableVertexAttribArray(0);
    }
    return 0;
}
int renderChunk(chunk*ck){
    rerenderChunk(ck);
    for(int i=0;i<4;++i){
        rerenderChunk(getChunk(ck->x+dx[i],ck->z+dz[i]));
    }
    return 0;
}
void drawChunk(chunk*ck){
    //puts("draw chunk");
    for(int i=0;i<16;++i){
        if(ck->mask&(1<<i)){
            glBindVertexArray(ck->section[i].vao);
            glDrawArrays(GL_QUADS,0,ck->section[i].numVertices);
            //puts("我draw了呀");
        }
    }
}
void draw(){//Draw a frame
    if(player->y<1)return;
    //puts("renmeil");
    float offset=1;
    if(tick<5000||tick>19000) offset=0.2;
    else if(tick<6000) offset=0.2+(tick-5000.0)/1000*0.8;
    else if(tick>18000) offset=1-(tick-18000.0)/1000*0.8;
    glClearColor(offset*0.6,offset*0.6,offset*1,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    mats_vp(shaderProgram,player->x,player->y,player->z,playerYaw,playerPitch);
    int chunkX=player->x/16,chunkZ=player->z/16;
    for(int x=chunkX;x<=chunkX;++x){
        for(int z=chunkZ;z<=chunkZ;++z){//For nearby chunk, render each section
            chunk*this=getChunk(x,z);
            if(!this->loaded){
                if(this->rendered);//this->rendered=0;//TODO:Unload chunk
                continue;
            }
            if(!this->rendered)renderChunk(this);
            drawChunk(this);
        }
    }
    if(0){
    for(int i=0;i<cubect;i+=3){
        mats_mvp(shaderProgram,cubes[i],cubes[i+1],cubes[i+2],player->x,player->y,player->z,playerYaw,playerPitch);
        glDrawArrays(GL_TRIANGLES,0,36);
    }
    for(int i=0;i<blockct;i+=3){
        mats_mvp(shaderProgram,blocks[i],blocks[i+1],blocks[i+2],player->x,player->y,player->z,playerYaw,playerPitch);
        GLuint ID=glGetUniformLocation(shaderProgram,"blockColor");
        float cl[]={blocks[i],blocks[i+1],blocks[i+2],1};
        glUniform4fv(ID,1,cl);
        glDrawArrays(GL_TRIANGLES,0,36);
    }}
    SDL_GL_SwapWindow(win);//A frame done!
}
int shad(char *name,char*const ipt){
    FILE*f=fopen(name,"r");
    int I=-1;while(~(ipt[++I]=fgetc(f)));ipt[I]=0;
    fclose(f);
    return I;
}
void play(){
    win=SDL_CreateWindow("Mycraft",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1440,900,SDL_WINDOW_OPENGL);
    if(!win){printf("Create window error %s\n",SDL_GetError());return;}
    SDL_GLContext contx;
    if(NULL==(contx=SDL_GL_CreateContext(win))){printf("Create context error %s\n",SDL_GetError());return;}
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE)|SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,2))puts("GL_SetAttribute error");
    GLfloat cube[144]={-0.48f,-0.48f,-0.48f, // triangle 1 : begin
    -0.48f, 0.48f, 0.48f,
    0.48f,-0.48f, 0.48f};
    glewExperimental = GL_TRUE;glewInit();
    GLuint vbo,vao;glGenBuffers(1,&vbo);glGenVertexArrays(1,&vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBindVertexArray(vao);
    glBufferData(GL_ARRAY_BUFFER,sizeof(cube),cube,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
    glEnableVertexAttribArray(0);
    shaderProgram=glCreateProgram(),vertID=glCreateShader(GL_VERTEX_SHADER),fragID=glCreateShader(GL_FRAGMENT_SHADER);
    glAttachShader(shaderProgram,vertID);glAttachShader(shaderProgram,fragID);
    char*const ipt=malloc(23333);
    int I=shad("vert.glsl",ipt);
    glShaderSource(vertID,1,(const char*const*)&ipt,&I);
    glCompileShader(vertID);
    glGetShaderiv(vertID,GL_INFO_LOG_LENGTH,&I);
    glGetShaderInfoLog(vertID,I,NULL,ipt);//printf("Vert:%s",ipt);
    I=shad("frag.glsl",ipt);
    glShaderSource(fragID,1,(const char*const*)&ipt,&I);
    glCompileShader(fragID);
    glGetShaderiv(fragID,GL_INFO_LOG_LENGTH,&I);
    glGetShaderInfoLog(fragID,I,NULL,ipt);//printf("Frag:%s",ipt);
    free(ipt);
    glLinkProgram(shaderProgram);glUseProgram(shaderProgram);
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);
    //mats(shaderProgram,0,0);
    float blue=1;
    glClearColor(0.6,0.6,blue,0.8);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //glDrawArrays(GL_TRIANGLES,0,36);
    SDL_GL_SwapWindow(win);
    SDL_Event event;
    short mouseX=-1,mouseY=-1;
    long now=clock();
    while(gameState==PLAY){//Network thread exists
        if(SDL_PollEvent(&event)){
            if(event.type==SDL_QUIT)break;
            if(event.type==SDL_KEYDOWN){
                //printf("Key name:%s\n",SDL_GetKeyName(event.key.keysym.sym));
                if(event.key.keysym.sym==SDLK_ESCAPE)gameState=EXIT;
                if(event.key.keysym.sym==SDLK_t){
                    ct=0;
                    blue*=-1;
                    //glClearColor(0.5,0.2,blue,0.8);
                    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                    //SDL_GL_SwapWindow(win);
                }
                double dx=.2*cos(playerYaw*Pi/180),dz=.2*sin(playerYaw*Pi/180);
                if(event.key.keysym.sym==SDLK_w){
                    player->x+=dx;player->z+=dz;
                    //draw();
                    move();
                }
                if(event.key.keysym.sym==SDLK_a){
                    player->x+=dz;player->z-=dx;
                    //draw();
                    move();
                }
                if(event.key.keysym.sym==SDLK_s){
                    player->x-=dx;player->z-=dz;
                    //draw();
                    move();
                }
                if(event.key.keysym.sym==SDLK_d){
                    player->x-=dz;player->z+=dx;
                    //draw();
                    move();
                }
                if(event.key.keysym.sym==SDLK_SPACE){
                    player->y+=0.0625;
                    //draw();
                    move();
                }
                if(event.key.keysym.sym==SDLK_LSHIFT||event.key.keysym.sym==SDLK_RSHIFT){
                    player->y-=0.0625;
                    //draw();
                    move();
                }
            }
            if(event.type==SDL_MOUSEMOTION){
                double dx=event.motion.x-mouseX,dy=event.motion.y-mouseY;
                if(abs(dx)+abs(dy)<4)continue;
                if((~mouseX)&&abs(dx)+abs(dy)<100){
                //puts("mouse motion");
                    playerYaw+=(double)(dx)/-3;playerPitch+=(double)(dy)/3;
                    if(playerPitch*playerPitch>8000)playerPitch*=89.4/abs(playerPitch);
                    while(playerYaw<0)playerYaw+=360;
                    while(playerYaw>360)playerYaw-=360;
                    //draw();
                }
//                move();
                mouseX=event.motion.x;mouseY=event.motion.y;
            }
            continue;
        }
        if(clock()-now>333){draw();now=clock();}
    }
    /*for(int i=0;i<80;i++){
        for(int j=0;j<80;j++){
            printf("%d ",mapData[i][(int)player->y][j].id);
        }puts("");
    }*/
    glDeleteShader(vertID);
    glDeleteShader(fragID);
    glDisableVertexAttribArray(0);
    glDeleteBuffers(1,&vbo);
    glDeleteVertexArrays(1,&vao);
    SDL_GL_DeleteContext(contx);
    SDL_DestroyWindow(win);
}
int main(int argc,char **argv){
    if(SDL_Init(SDL_INIT_VIDEO)){puts("SDL Init error");return -1;}
    SDL_Thread*network=SDL_CreateThread((int (*)(void *))networkThread,"network thread",argc>1?argv[1]:NULL);
    SDL_DetachThread(network);

    while(gameState!=EXIT && gameState!=PLAY);
    if(gameState==PLAY)play();//Main function
    puts("\nbye");
    
    SDL_Quit();
   return 0;
}
