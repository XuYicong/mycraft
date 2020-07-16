#include "objects.h"
GameState gameState=LOGIN;
int shaderProgram,vertID,fragID;
SDL_Window*win;
double cubes[23334],blocks[233345];
int cubect,blockct;
int dx[]={-1,0,0,1,0,0},dy[]={0,1,0,0,-1,0},dz[]={0,0,-1,0,0,1},flor=2333;
/*void dfs(int x,int y,int z){
    if(flor<0)return;
    int dataX=x-mapData[0][0][0].x,dataZ=z-mapData[0][0][0].z;
    while(dataX<0)dataX+=336;
    while(dataZ<0)dataZ+=336;
    dataX%=336;dataZ%=336;
    vis[dataX][y][dataZ]=1;
    for(int i=0;i<6;i++){
        int X=x+dx[i],Y=y+dy[i],Z=z+dz[i];
        if(abs(X+Y+Z-player->x-player->y-player->z)<13){
            if(mapData[(dataX+dx[i]+336)%336][Y][(dataZ+dz[i]+336)%336].id){
                blocks[blockct++]=X;
                blocks[blockct++]=Y;
                blocks[blockct++]=Z;
                vis[(dataX+dx[i]+336)%336][Y][(dataZ+dz[i]+336)%336]=1;
            }else if(!vis[(dataX+dx[i]+336)%336][Y][(dataZ+dz[i]+336)%336]){
                flor--;dfs(X,Y,Z);flor++;
            }
        }
    }
}*/
void draw(){
    if(player->y<1)return;
    if(0){
        for(int i=0;i<441;++i){//逻辑需改成：遍历所有chk，若存在且离我近则渲染
            for(int j=0;j<16;++j){
                if(chk[i].section[j].num<1)continue;
                int last=0,bitct=0;
                for(int longct=0;longct<chk[i].section[j].num;++longct){
                    while(64-bitct>=chk[i].section[j].bits){
                        if(last);//with increasing x coordinates, within rows of increasing z coordinates, within layers of increasing y coordinates. 
                    }
                    last=chk[i].section[j].data[longct]%(1<<(64-bitct));
                }
            }
        }
    }
    float offset=1;
    if(tick<5000||tick>19000) offset=0.2;
    else if(tick<6000) offset=0.2+(tick-5000.0)/1000*0.8;
    else if(tick>18000) offset=1-(tick-18000.0)/1000*0.8;
    glClearColor(offset*0.6,offset*0.6,offset*1,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if(0){
    for(int i=0;i<cubect;i+=3){
        mats(shaderProgram,cubes[i],cubes[i+1],cubes[i+2],player->x,player->y,player->z,playerYaw,playerPitch);
        glDrawArrays(GL_TRIANGLES,0,36);
    }
    for(int i=0;i<blockct;i+=3){
        mats(shaderProgram,blocks[i],blocks[i+1],blocks[i+2],player->x,player->y,player->z,playerYaw,playerPitch);
        GLuint ID=glGetUniformLocation(shaderProgram,"blockColor");
        float cl[]={blocks[i],blocks[i+1],blocks[i+2],1};
        glUniform4fv(ID,1,cl);
        glDrawArrays(GL_TRIANGLES,0,36);
    }}
    SDL_GL_SwapWindow(win);
}
int shad(char *name,char*const ipt){
    FILE*f=fopen(name,"r");
    int I=-1;while(~(ipt[++I]=fgetc(f)));ipt[I]=0;
    fclose(f);
    return I;
}
void play(){
    win=SDL_CreateWindow("Mycraft",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1440,900,SDL_WINDOW_OPENGL);
    if(!win){printf("Create window error %s\n",SDL_GetError());return -2;}
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
                if(event.key.keysym.sym==SDLK_ESCAPE)break;
                if(event.key.keysym.sym==SDLK_t){
                    ct=0;
                    blue*=-1;
                    //glClearColor(0.5,0.2,blue,0.8);
                    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                    //SDL_GL_SwapWindow(win);
                }
                double dx=.1*cos(playerYaw*Pi/180),dz=.1*sin(playerYaw*Pi/180);
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
    SDL_Thread*network=SDL_CreateThread(networkThread,"network thread",argc>1?argv[1]:NULL);
    SDL_DetachThread(network);

    while(gameState!=EXIT && gameState!=PLAY);
    if(gameState==PLAY)play();//Main function
    puts("\nbye");
    
    SDL_Quit();
   return 0;
}
